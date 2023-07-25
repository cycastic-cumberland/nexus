//
// Created by cycastic on 7/22/2023.
//

#include "threads_manager.h"
#include "../core/types/linked_list.h"

ThreadsManager* ThreadsManager::singleton = nullptr;

void ThreadsManager::thread_state_changed(Ref<WorkerThread> p_thread, uint32_t p_thread_id, uint8_t p_state) {
    switch ((WorkerThread::WorkerState)p_state) {
        case WorkerThread::SETUP_COMPLETED: {
            W_GUARD(get_singleton()->lock);
            auto native_id = std::this_thread::get_id();
            get_singleton()->native_id_to_managed[native_id] = p_thread_id;
            get_singleton()->managed_id_to_native[p_thread_id] = native_id;
            break;
        }
//        case WorkerThread::WORKING: {
//            W_GUARD(get_singleton()->lock);
//            get_singleton()->free_threads.erase(p_thread_id);
//            break;
//        }
        case WorkerThread::FREE: {
            W_GUARD(get_singleton()->lock);
            get_singleton()->free_threads[p_thread_id] = p_thread;
            break;
        }
        case WorkerThread::TERMINATED: {
            // This would only be called if the thingy is terminated by manager
            W_GUARD(get_singleton()->lock);
            get_singleton()->deallocate_thread_internal(p_thread);
            break;
        }
        // Handled by manager
        case WorkerThread::WORKING:
            break;
    }
}

uint32_t ThreadsManager::create_new_thread(const uint32_t &p_attributes) {
    auto thread = Ref<WorkerThread>::make_ref(get_singleton()->thread_id_allocator.increment(),
                                              p_attributes,
                                              thread_state_changed);
    {
        W_GUARD(get_singleton()->lock);
        get_singleton()->all_threads[thread->get_thread_id()] = thread;
    }
    return thread->get_thread_id();
}

ThreadsManager::~ThreadsManager() {
    W_GUARD(lock);
    Vector<Ref<WorkerThread>> thread_ids{};
    auto it = all_threads.const_iterator();
    while (it.move_next()){
        thread_ids.push_back(it.get_pair().value);
        it.get_pair().value->set_terminated_by_manager();
    }
    for (const auto& id : thread_ids){
        deallocate_thread_internal(id);
    }
}

Ref<WorkerThread> ThreadsManager::get_thread_by_id_internal(const uint32_t &p_id) {
//    R_GUARD(lock);
    if (!all_threads.exists(p_id)) return Ref<WorkerThread>::null();
    return all_threads[p_id];
}

bool ThreadsManager::deallocate_thread_internal(const Ref<WorkerThread> &p_thread) {
    auto managed_id = p_thread->get_thread_id();
    if (!all_threads.exists(managed_id)) return false;
    auto native_id = managed_id_to_native[managed_id];
    managed_id_to_native.erase(managed_id);
    native_id_to_managed.erase(native_id);
    free_threads.erase(managed_id);
    all_threads.erase(managed_id);
    return true;
}

void ThreadsManager::terminate_thread(const uint32_t &p_thread_id) {
    auto thread = get_singleton()->get_thread_by_id_internal(p_thread_id);
    W_GUARD(get_singleton()->lock);
    thread->set_terminated_by_manager();
    get_singleton()->deallocate_thread_internal(thread);
}

uint32_t ThreadsManager::get_current_managed_thread_id() {
    R_GUARD(get_singleton()->lock);
    auto native_id = std::this_thread::get_id();
    if (!get_singleton()->native_id_to_managed.exists(native_id)) return 0;
    return get_singleton()->native_id_to_managed[native_id];
}

Ref<WorkerThread> ThreadsManager::get_first_free_thread() {
    auto it = free_threads.const_iterator();
    while (it.move_next()){
        return it.get_pair().value;
    }
    return Ref<WorkerThread>::null();
}

bool ThreadsManager::add_work(const uint32_t &p_thread_id, worker_callback p_callback, void *p_args) {

    W_GUARD(get_singleton()->lock);
    auto thread = get_singleton()->get_thread_by_id_internal(p_thread_id);
    if (thread.is_null()) return false;
    thread->add_work(p_callback, p_args);
    get_singleton()->free_threads.erase(thread->get_thread_id());
    return true;
}

bool ThreadsManager::populate_free_thread(worker_callback p_callback, void *p_args) {
    W_GUARD(get_singleton()->lock);
    auto thread = get_singleton()->get_first_free_thread();
    if (thread.is_null()) return false;
    thread->add_work(p_callback, p_args);
    get_singleton()->free_threads.erase(thread->get_thread_id());
    return true;
}

void WorkerThread::thread_loop(WorkerThread *thread) {
    const auto worker_state_change = thread->worker_state_change;
    const auto is_daemon = thread->is_daemon();
    const auto is_enclosed = thread->is_enclosed();
    const auto as_ref = Ref<WorkerThread>::from_initialized_object(thread);
    if (!is_daemon && worker_state_change) worker_state_change(as_ref, thread->thread_id, SETUP_COMPLETED);
    auto last_state_free = false;
    auto curr_state_free = true;
    while (!thread->is_terminating()){
        worker_callback callback;
        void* args;
        {
            R_GUARD(thread->rwlock);
            callback = thread->current_callback;
            args = thread->callback_arguments;
        }
        if (callback){
            curr_state_free = false;
            {
                W_GUARD(thread->rwlock);
                thread->current_callback = nullptr;
                thread->callback_arguments = nullptr;
                thread->thread_attributes |= IS_BUSY;
            }
            if (!is_enclosed && !is_daemon && worker_state_change) worker_state_change(as_ref, thread->thread_id, WORKING);
            callback(args);
            {
                W_GUARD(thread->rwlock);
                thread->thread_attributes &= busy_mask;
                // If enclosed => immediately terminate
                if (is_enclosed) break;
            }
        } else curr_state_free = true;
        if (!is_daemon && worker_state_change && (last_state_free != curr_state_free)) {
//            System::get_singleton()->yield();
            worker_state_change(as_ref, thread->thread_id, FREE);
        }
        last_state_free = curr_state_free;
    }
    thread->terminated = true;
    if (!is_daemon && !thread->is_terminated_by_manager() && worker_state_change) worker_state_change(as_ref, thread->thread_id, TERMINATED);
}
