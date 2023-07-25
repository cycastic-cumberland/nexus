//
// Created by cycastic on 7/22/2023.
//

#ifndef NEXUS_THREADS_MANAGER_H
#define NEXUS_THREADS_MANAGER_H

#include <thread>
#include "../core/lock.h"
#include "../core/typedefs.h"
#include "../core/types/object.h"
#include "../core/types/safe_refcount.h"

class WorkerThread;
class ThreadsManager;

typedef void (*worker_callback)(void*);
typedef void (*worker_state_change_cb)(Ref<WorkerThread>, uint32_t, uint8_t);

// Ideal operation
// Start: create thread and stack1, not busy
// Insert process: mark busy, start executing
// Process leave: return stack1 data and exit info (thread left safely/exception thrown/await called)
// If shared: do not object_destroy thread on leave
class WorkerThread : public Object {
public:
    static constexpr uint32_t busy_mask = ~(1 << 1);
    enum ThreadAttribute : unsigned int {
        ENCLOSED = 0,       // Standard thread
        SHARED = 1,         // Async thread,
        IS_BUSY = 2,        // Currently executing
        IS_DAEMON,          // Is daemon thread
    };
    enum WorkerState : unsigned int {
        SETUP_COMPLETED,
        WORKING,
        FREE,
        TERMINATED,
    };
private:
    uint32_t thread_id;
    uint32_t thread_attributes;
    std::thread* managed_thread;
    worker_callback current_callback{};
    worker_state_change_cb worker_state_change;
    void* callback_arguments{};
    bool terminating = false;
    bool terminated = false;
    mutable RWLock rwlock;
    mutable bool terminated_by_manager = false;
    _FORCE_INLINE_ bool is_terminating() {
        R_GUARD(rwlock);
        return terminating;
    }
    static void thread_loop(WorkerThread* thread);
public:
    _FORCE_INLINE_ bool is_enclosed() const {
        R_GUARD(rwlock);
        return thread_attributes == 0;
    }
    _FORCE_INLINE_ bool is_busy() const {
        R_GUARD(rwlock);
        return thread_attributes & IS_BUSY;
    }
    _FORCE_INLINE_ bool is_daemon() const {
        R_GUARD(rwlock);
        return thread_attributes & IS_DAEMON;
    }
    _FORCE_INLINE_ bool is_terminated() const {
//        R_GUARD(lock);
        if (is_daemon()) return true;
        return managed_thread->joinable();
    }
    _FORCE_INLINE_ bool is_terminated_by_manager() const {
        R_GUARD(rwlock);
        return terminated_by_manager;
    }
    _FORCE_INLINE_ uint32_t get_thread_id() const { return thread_id; }
    _FORCE_INLINE_ void add_work(worker_callback callback, void* args = nullptr) {
        W_GUARD(rwlock);
        current_callback = callback;
        callback_arguments = args;
    }
    _FORCE_INLINE_ void join() const {
        if (is_daemon()) return;
        managed_thread->join();
    }
    _FORCE_INLINE_ void terminate(){
        W_GUARD(rwlock);
        terminating = true;
    }
    friend class ThreadsManager;
    friend class Ref<WorkerThread>;
private:
    WorkerThread(const uint32_t& p_id, const uint32_t& p_attributes, worker_state_change_cb p_free_worker = nullptr) : Object(), thread_id(p_id){
        thread_attributes = p_attributes;
        // Always set IS_BUSY to false
        thread_attributes &= busy_mask;
        worker_state_change = p_free_worker;
        managed_thread = new std::thread(&WorkerThread::thread_loop, this);
        if (is_daemon())
            managed_thread->detach();
    }
    explicit WorkerThread(const uint32_t& p_id, worker_state_change_cb p_free_worker = nullptr) : WorkerThread(p_id, ENCLOSED, p_free_worker) {}
    _FORCE_INLINE_ void set_terminated_by_manager() const {
        W_GUARD(rwlock);
        terminated_by_manager = true;
    }
public:
    ~WorkerThread() override {
        terminate();
        join();
        // Would theoretically not block, as the thread is detached
        delete managed_thread;
    }
};

struct NativeThreadIdHasher {
    static _FORCE_INLINE_ uint32_t hash(const std::thread::id& p_id) {
        return StandardHasher::hash(std::hash<std::thread::id>{}(p_id));
    }
};

class ThreadsManager {
private:
    HashMap<uint32_t, Ref<WorkerThread>> all_threads{};
    HashMap<uint32_t, Ref<WorkerThread>> free_threads{};
    HashMap<std::thread::id, uint32_t, 32, NativeThreadIdHasher> native_id_to_managed;
    HashMap<uint32_t, std::thread::id, 32> managed_id_to_native;
    SafeNumeric<uint32_t> thread_id_allocator{0};
    RWLock lock{};

    static ThreadsManager* singleton;
    static void thread_state_changed(Ref<WorkerThread> p_thread, uint32_t p_thread_id, uint8_t p_state);
    Ref<WorkerThread> get_first_free_thread();
    bool deallocate_thread_internal(const Ref<WorkerThread>& p_thread);
//    void terminate_thread(Ref<WorkerThread> p_thread);
    Ref<WorkerThread> get_thread_by_id_internal(const uint32_t &p_id);
    static _ALWAYS_INLINE_ ThreadsManager* get_singleton() { return singleton; }
public:
    ThreadsManager() { singleton = this; }
    ~ThreadsManager();

    static uint32_t create_new_thread(const uint32_t& p_attributes);
    static void terminate_thread(const uint32_t& p_thread_id);
    static bool populate_free_thread(worker_callback p_callback, void* p_args);
    static bool add_work(const uint32_t& p_thread_id, worker_callback p_callback, void* p_args);
    static uint32_t get_current_managed_thread_id();
//    static _FORCE_INLINE_ void destroy() { singleton->~ThreadsManager(); singleton = nullptr; }
};

#endif //NEXUS_THREADS_MANAGER_H
