//
// Created by cycastic on 7/25/2023.
//
#include "threads_pool.h"
#include "../core/types/vstring.h"
#include "nexus_output.h"

Thread::ID Thread::main_thread_id = thread_id_hash(std::this_thread::get_id());
static thread_local Thread::ID caller_id = 0;
static thread_local bool caller_id_cached = false;

uint64_t Thread::thread_id_hash(const std::thread::id &p_native_id) {
    static std::hash<std::thread::id> hasher{};
    return hasher(p_native_id);
}

void Thread::callback(Thread *p_self, thread_callback p_cb, void *p_userdata) {
    caller_id = thread_id_hash(p_self->thread.get_id());
    caller_id_cached = true;
    p_cb(p_userdata);
}

void Thread::start(const ThreadTask& p_task) {
    W_GUARD(lock);
    // Current ID is not this thread's ID
    if (id != thread_id_hash(std::thread::id())){
        thread.detach();
        std::thread empty_thread;
        thread.swap(empty_thread);
    }
    std::thread new_thread(&Thread::callback, this, p_task.callback, p_task.userdata);
    thread.swap(new_thread);
    id = thread_id_hash(thread.get_id());
}

bool Thread::is_started() const {
    R_GUARD(lock);
    return id != thread_id_hash(std::thread::id());
}

bool Thread::is_alive() const {
    R_GUARD(lock);
    return !thread.joinable();
}

bool Thread::is_finished() const {
    R_GUARD(lock);
    return !is_alive();
}

void Thread::join() {
    W_GUARD(lock);
    if (id != thread_id_hash(std::thread::id())) {
        if (id == Thread::get_caller_id()){
            print_error(VString("Thread ") + itos(id) + " can't join itself.");
            return;
        }
        thread.join();
        std::thread empty_thread;
        thread.swap(empty_thread);
        id = thread_id_hash(std::thread::id());
    }
}

Thread::ID Thread::get_caller_id() {
    if (likely(caller_id_cached)) {
        return caller_id;
    } else {
        caller_id = thread_id_hash(std::this_thread::get_id());
        caller_id_cached = true;
        return caller_id;
    }
}


Thread::~Thread(){
    if (id != thread_id_hash(std::thread::id())){
        print_warning("A Thread object has been destroyed without join() having been called on it."
                      " Please do so to ensure correct cleanup of the thread.");
        thread.detach();
    }
}

Thread::ID ThreadPool::allocate_new_thread() {
    W_GUARD(thread_lock);
    auto new_thread_object = new ThreadObject();
    new_thread_object->self = this;
    new_thread_object->thread.start(ThreadTask(callback, new_thread_object));
    auto id = new_thread_object->thread.get_id();
    all_threads[id] = new_thread_object;
    return id;
}

Vector<Thread::ID> ThreadPool::batch_allocate_threads(const uint8_t &thread_count) {
    auto ids = Vector<Thread::ID>(thread_count);
    for (uint8_t i = 0; i < thread_count; i++){
        ids.push_back(allocate_new_thread());
    }
    return ids;
}

void ThreadPool::join_thread(const Thread::ID &p_id) {
    W_GUARD(thread_lock);
    join_thread_internal(p_id);
}

void ThreadPool::join_thread_internal(const Thread::ID &p_id) {
    if (!all_threads.exists(p_id)) return;
    auto thread = all_threads[p_id];
    all_threads.erase(p_id);
    thread->is_terminated.set();
    thread->thread.join();
    delete thread;
}

void ThreadPool::join_all() {
    W_GUARD(thread_lock);
    Vector<ThreadObject*> objects(all_threads.size());
    auto it = all_threads.const_iterator();
     while (it.move_next()){
         auto obj = it.get_pair().value;
         obj->is_terminated.set();
         objects.push_back(obj);
     }
     all_threads.clear();
     for (const auto& obj : objects){
         obj->thread.join();
         delete obj;
     }
}

Ref<ThreadPool::TaskTicket> ThreadPool::queue_task(const ThreadTask &p_task, ThreadPool::Priority p_priority) {
    GUARD(task_lock);
    InternalInternalTicket ticket(p_task);
    tasks_queue.push(ticket, p_priority);
    return ticket.exposed_ticket;
}

void ThreadPool::callback(void *p_self) {
    auto thread_object = (ThreadObject*)p_self;
    while (!thread_object->is_terminated.is_set()){
        ThreadPool::InternalInternalTicket task_ticket = thread_object->self->get_task_internal();
        if (task_ticket.is_valid()) {
            task_ticket.exposed_ticket->started.set();
            task_ticket.task.invoke();
            task_ticket.exposed_ticket->finished.set();
        }
        System::get_singleton()->yield();
    }
}

ThreadPool::InternalInternalTicket ThreadPool::get_task_internal() {
    GUARD(task_lock);
    if (tasks_queue.empty()) return {};
    return tasks_queue.pop();
}

ThreadPool::~ThreadPool() {
    join_all();
}

void ThreadPool::TaskTicket::wait() const {
    while (!is_finished()) System::get_singleton()->yield();
}
