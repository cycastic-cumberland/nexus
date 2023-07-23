//
// Created by cycastic on 7/22/2023.
//

#include "threads_manager.h"
#include "../core/types/linked_list.h"
#include "system.h"

ThreadsManager* ThreadsManager::singleton = new ThreadsManager();

void ThreadsManager::thread_state_changed(WorkerThread *p_thread, uint32_t p_thread_id, uint8_t p_state) {
    get_singleton()->rwlock.write_lock();
    switch ((WorkerThread::WorkerState)p_state) {
        case WorkerThread::SETUP_COMPLETED: {
            auto native_id = std::this_thread::get_id();
            get_singleton()->native_id_to_managed[native_id] = p_thread_id;
            get_singleton()->managed_id_to_native[p_thread_id] = native_id;
            get_singleton()->rwlock.write_unlock();
            break;
        }
        case WorkerThread::WORKING:
            get_singleton()->free_threads.erase(p_thread_id);
            get_singleton()->rwlock.write_unlock();
            break;
        case WorkerThread::FREE:
            get_singleton()->free_threads[p_thread_id] = p_thread;
            get_singleton()->rwlock.write_unlock();
            break;
        case WorkerThread::TERMINATED:
            get_singleton()->all_threads.erase(p_thread_id);
            get_singleton()->terminate_thread(p_thread);
            get_singleton()->rwlock.write_unlock();
            break;
    }
}

WorkerThread *ThreadsManager::create_new_thread(const uint32_t &p_attributes) {
    W_GUARD(get_singleton()->rwlock);
    auto id = get_singleton()->thread_count.increment();
    auto thread = new WorkerThread(id, p_attributes, ThreadsManager::thread_state_changed);
    get_singleton()->all_threads[id] = thread;
    if (!thread->is_enclosed())
        ThreadsManager::get_singleton()->free_threads[id] = thread;
    return thread;
}

void ThreadsManager::terminate_thread(WorkerThread *p_thread) {
    auto managed_thread_id = p_thread->get_thread_id();
    free_threads.erase(managed_thread_id);
    auto native_thread_id = managed_id_to_native[managed_thread_id];
    managed_id_to_native.erase(managed_thread_id);
    native_id_to_managed.erase(native_thread_id);
    delete p_thread;
}

WorkerThread *ThreadsManager::get_current_managed_thread() {
    auto native = std::this_thread::get_id();
    get_singleton()->rwlock.read_lock();
    if (!get_singleton()->native_id_to_managed.exists(native)) return nullptr;
    auto managed_id = get_singleton()->native_id_to_managed[native];
    get_singleton()->rwlock.read_unlock();
    return get_thread_by_id(managed_id);
}

uint32_t ThreadsManager::get_current_managed_thread_id(){
    auto re = get_current_managed_thread();
    if (!re) return 0;
    else return re->get_thread_id();
}

ThreadsManager::~ThreadsManager() {
    LinkedList<uint32_t> ids{};
    rwlock.write_lock();
    {
        auto it = all_threads.const_iterator();
        while (it.move_next()) {
            ids.add_last(it.get_pair().key);
        }
    }
    for (const auto* it = ids.first(); it; it = it->next()){
        terminate_thread(it->data);
    }
    rwlock.write_unlock();
}

void ThreadsManager::terminate_thread(const uint32_t &p_thread_id) {
    W_GUARD(get_singleton()->rwlock);
    if (!get_singleton()->all_threads.exists(p_thread_id)) {
        return;
    }
    auto thread = get_singleton()->get_thread_by_id_internal(p_thread_id);
    get_singleton()->terminate_thread(thread);
}

WorkerThread *ThreadsManager::get_thread_by_id(const uint32_t &p_id) {
    R_GUARD(get_singleton()->rwlock);
    return get_singleton()->get_thread_by_id_internal(p_id);
}

WorkerThread *ThreadsManager::get_thread_by_id_internal(const uint32_t &p_id){
    if (!all_threads.exists(p_id)) return nullptr;
    return all_threads[p_id];
}

WorkerThread *ThreadsManager::get_first_free_thread() {
    auto it = free_threads.const_iterator();
    if (it.move_next()) return it.get_pair().value;
    else return nullptr;
}

bool ThreadsManager::populate_free_thread(worker_callback p_callback, void *p_args) {
    W_GUARD(get_singleton()->rwlock);
    auto thread = get_singleton()->get_first_free_thread();
    if (!thread) return false;
    thread->add_work(p_callback, p_args);
    return true;
}

void WorkerThread::thread_loop(WorkerThread *thread) {
    const auto worker_state_change = thread->worker_state_change;
    if (worker_state_change) worker_state_change(thread, thread->thread_id, SETUP_COMPLETED);
    auto last_state_free = true;
    auto curr_state_free = true;
    while (!thread->is_terminating()){
        thread->rwlock.read_lock();
        auto callback = thread->current_callback;
        auto args = thread->callback_arguments;
        thread->rwlock.read_unlock();
        if (callback){
            curr_state_free = false;
            {
                W_GUARD(thread->rwlock);
                thread->current_callback = nullptr;
                thread->callback_arguments = nullptr;
                thread->thread_attributes |= IS_BUSY;
            }
            if (worker_state_change) worker_state_change(thread, thread->thread_id, WORKING);
            callback(args);
            {
                W_GUARD(thread->rwlock);
                thread->thread_attributes &= busy_mask;
                // If enclosed => immediately terminate
                if (thread->thread_attributes | ENCLOSED) break;
            }
        } else curr_state_free = true;
        // If not enclosed and free state changed, yield this thread to the system before notifying
        if (worker_state_change && (last_state_free != curr_state_free)) {
            System::get_singleton()->yield();
            worker_state_change(thread, thread->thread_id, FREE);
        }
        last_state_free = curr_state_free;
    }
    thread->terminated = true;
    if (worker_state_change) worker_state_change(thread, thread->thread_id, TERMINATED);
}
