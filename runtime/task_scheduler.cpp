//
// Created by cycastic on 7/24/2023.
//

#include "task_scheduler.h"
#include "../language/bytecode.h"
#include "system.h"
#include "nexus_output.h"
#include "task.h"

TaskScheduler* TaskScheduler::singleton = nullptr;

#define TASK_SCHEDULER get_singleton()
#define TS_R R_GUARD(TASK_SCHEDULER->lock);
#define TS_W W_GUARD(TASK_SCHEDULER->lock);
#define TS_RL TASK_SCHEDULER->lock.read_lock();
#define TS_RU TASK_SCHEDULER->lock.read_unlock();
#define TS_WL TASK_SCHEDULER->lock.write_lock();
#define TS_WU TASK_SCHEDULER->lock.write_unlock();


Ref<ThreadPool::TaskTicket> TaskScheduler::queue_task_internal(const Ref<Task> &p_task, ThreadPool::Priority p_priority) {
    auto id = p_task->get_id();
    // No one run this program on a 16-bit system anyway, amirite?
    auto as_void_pointer = (void*)(size_t)id;
    if (TASK_SCHEDULER->in_transit_tasks.exists(id)){
        print_error(VString("Task with id ") + itos(id) + " has already been queued");
        return Ref<ThreadPool::TaskTicket>::null();
    }
    // Keeping the task alive while a thread is picking it up
    TASK_SCHEDULER->in_transit_tasks[id] = p_task;
    auto ticket = TASK_SCHEDULER->thread_pool->queue_task(ThreadTask(task_handler, as_void_pointer), p_priority);
    return ticket;
}

Ref<ThreadPool::TaskTicket> TaskScheduler::queue_task(const Ref<Task> &p_task, ThreadPool::Priority p_priority) {
    TS_W;
    return queue_task_internal(p_task, p_priority);
}

TaskScheduler::TaskScheduler() {
    singleton = this;
    thread_pool = new ThreadPool();
    thread_pool->batch_allocate_threads(NexusRuntimeGlobalSettings::get_settings()->task_scheduler_starting_thread_count);
}

void TaskScheduler::task_handler(void *p_async_request) {
    TS_WL
    auto as_task_id = (uint32_t)(size_t)p_async_request;
    auto current_task = TASK_SCHEDULER->in_transit_tasks[as_task_id];
    TASK_SCHEDULER->in_transit_tasks.erase(as_task_id);
    TS_WU
    current_task->handle_resume();
    auto result = current_task->execute();
    Task::AsyncCallbackReturn async_return = Task::EXITED_SAFELY;
    Ref<Task> branched_into = Ref<Task>::null();
    result.unpack(async_return, branched_into);
    switch (async_return) {
        case Task::EXITED_SAFELY:{
            TS_W
            if (!TASK_SCHEDULER->frozen_tasks.exists(current_task)){
                // This is a genesis task, do nothing
            } else {
                // This is spawned from another task, resuming it
                TASK_SCHEDULER->frozen_tasks.erase(current_task);
                queue_task_internal(TASK_SCHEDULER->frozen_tasks[current_task]);
            }
            current_task->set_finished();
            break;
        }
        case Task::AWAIT: {
            // Set this request's child task as the branched task's task object
            current_task->set_child_task(branched_into);
            TS_W
            TASK_SCHEDULER->frozen_tasks[branched_into] = current_task;
            queue_task_internal(branched_into);
            break;
        }
        case Task::EXCEPTION_THROWN:
            throw TaskSchedulerException("Not yet supported...");
    }
}

TaskScheduler::~TaskScheduler() {
    is_terminating.set();
    thread_pool->join_all();
    delete thread_pool;
}

#undef TASK_SCHEDULER
#undef TS_R
#undef TS_W
#undef TS_RL
#undef TS_RU
#undef TS_WL
#undef TS_WU
