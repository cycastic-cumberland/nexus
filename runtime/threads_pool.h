//
// Created by cycastic on 7/25/2023.
//

#ifndef NEXUS_THREADS_POOL_H
#define NEXUS_THREADS_POOL_H

#include <thread>
#include "../core/typedefs.h"
#include "../core/lock.h"
#include "../core/types/hashmap.h"
#include "../core/types/priority_queue.h"
#include "../core/types/object.h"

typedef void (*thread_callback)(void*);

struct ThreadTask {
    thread_callback callback;
    void* userdata;

    explicit ThreadTask(thread_callback p_cb, void* p_userdata = nullptr) : callback(p_cb), userdata(p_userdata) {}
    // 0 is highest level of priority
    ThreadTask() : ThreadTask(nullptr) {}
    ThreadTask(const ThreadTask& p_other) : callback(p_other.callback), userdata(p_other.userdata) {}
    _NO_DISCARD_ _ALWAYS_INLINE_ bool is_valid() const { return callback != nullptr; }
    void invoke() const { callback(userdata); }
};

class Thread {
public:
    typedef uint64_t ID;
private:
    static uint64_t thread_id_hash(const std::thread::id& p_native_id);
    static void callback(Thread *p_self, thread_callback p_cb, void* p_userdata);

    static ID main_thread_id;

    mutable RWLock lock{};
    std::thread thread{};
    ID id = thread_id_hash(std::thread::id());
public:
    Thread() = default;
    ~Thread();

    void start(const ThreadTask& p_task);
    _NO_DISCARD_ _FORCE_INLINE_ ID get_id() const { return id; }
    _NO_DISCARD_ bool is_started() const;
    _NO_DISCARD_ bool is_alive() const;
    _NO_DISCARD_ bool is_finished() const;
    void join();

    static ID get_caller_id();
};

class ThreadPool {
public:
    enum Priority : unsigned char {
        SYSTEM = 0,
        HIGH = 1,
        MEDIUM = 2,
        LOW = 3,
    };
    struct TaskTicket : public Object {
    private:
        SafeFlag finished{};
        SafeFlag started{};

        friend class ThreadPool;

    public:
        TaskTicket() = default;

        _NO_DISCARD_ _ALWAYS_INLINE_ bool is_finished() const { return finished.is_set(); }
        _NO_DISCARD_ _ALWAYS_INLINE_ bool task_started() const { return started.is_set(); }
        void wait() const;
    };
private:
    struct InternalInternalTicket {
        Ref<TaskTicket> exposed_ticket;
        ThreadTask task;

        explicit InternalInternalTicket(const ThreadTask& p_task)
        : exposed_ticket(Ref<TaskTicket>::make_ref()),
          task(p_task) {}
        InternalInternalTicket() : InternalInternalTicket(ThreadTask()) {}
        InternalInternalTicket(const InternalInternalTicket& p_other)
        : exposed_ticket(p_other.exposed_ticket),
          task(p_other.task) {}
        _NO_DISCARD_ _FORCE_INLINE_ bool is_valid() const { return task.is_valid(); }
    };
    struct ThreadObject {
        Thread thread{};
        SafeFlag is_terminated{false};
        ThreadPool* self{};

    };
private:
    mutable RWLock thread_lock{};
    mutable BinaryMutex task_lock{};
    HashMap<Thread::ID, ThreadObject*> all_threads{};
    PriorityQueue<InternalInternalTicket> tasks_queue{};

    static void callback(void* p_self);
    InternalInternalTicket get_task_internal();
    void join_thread_internal(const Thread::ID& p_id);
public:
    Thread::ID allocate_new_thread();
    Vector<Thread::ID> batch_allocate_threads(const uint8_t& thread_count);
    void join_thread(const Thread::ID& p_id);
    void join_all();

    Ref<TaskTicket> queue_task(const ThreadTask& p_task, Priority p_priority = MEDIUM);

    ~ThreadPool();
};

#endif //NEXUS_THREADS_POOL_H
