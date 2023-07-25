//
// Created by cycastic on 7/24/2023.
//

#ifndef NEXUS_TASK_SCHEDULER_H
#define NEXUS_TASK_SCHEDULER_H

#include "../core/exception.h"
#include "../core/types/reference.h"
#include "../core/types/tuple.h"
#include "../core/types/hashmap.h"
#include "../core/types/linked_list.h"
#include "../core/lock.h"
#include "../core/types/queue.h"
#include "runtime_global_settings.h"
#include "threads_pool.h"

class NexusMethodPointer;
class NexusBytecodeInstance;

class TaskSchedulerException : public Exception {
public:
    explicit TaskSchedulerException(const char* p_msg = nullptr) : Exception(p_msg) {}
};

class TaskScheduler {
private:
    static TaskScheduler* singleton;
    RWLock lock{};
    SafeNumeric<uint32_t> task_id_allocator{0};
    SafeFlag is_terminating{false};
    HashMap<Ref<Task>, Ref<Task>, 32, Task, Task> frozen_tasks{};
    HashMap<uint32_t, Ref<Task>> in_transit_tasks{};
    ThreadPool* thread_pool;

    static _ALWAYS_INLINE_ TaskScheduler* get_singleton() { return singleton; }
    static _ALWAYS_INLINE_ uint32_t next_task_id() {
        return get_singleton()->task_id_allocator.increment();
    }
    static void task_handler(void* p_async_request);

    friend class Task;
    static Ref<ThreadPool::TaskTicket> queue_task_internal(const Ref<Task>& p_task, ThreadPool::Priority p_priority = ThreadPool::MEDIUM);
public:
    static Ref<ThreadPool::TaskTicket> queue_task(const Ref<Task>& p_task, ThreadPool::Priority p_priority = ThreadPool::MEDIUM);

    TaskScheduler();
    ~TaskScheduler();
};

#endif //NEXUS_TASK_SCHEDULER_H
