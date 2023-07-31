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


std::future<void> TaskScheduler::queue_task_internal(const Ref<Task> &p_task) {
    auto ticket = TASK_SCHEDULER->thread_pool->queue_task((ThreadPool::Priority)p_task->get_priority(),
                                                          [p_task]() -> void { TaskScheduler::task_handler(p_task); });
    return ticket;
}

std::future<void> TaskScheduler::queue_task(const Ref<Task> &p_task) {
//    TS_W;
    return queue_task_internal(p_task);
}

TaskScheduler::TaskScheduler() {
    singleton = this;
    thread_pool = new ThreadPool();
    thread_pool->batch_allocate_workers(NexusRuntimeGlobalSettings::get_settings()->task_scheduler_starting_thread_count);
}

void TaskScheduler::task_handler(const Ref<Task>& p_current_task) {
    auto current_task = p_current_task;
    current_task->handle_resume();
    auto result = current_task->execute();
    Task::AsyncCallbackReturn async_return = Task::EXITED_SAFELY;
    Ref<Task> branched_into = Ref<Task>::null();
    result.unpack(async_return, branched_into);
    switch (async_return) {
        case Task::EXITED_SAFELY:{
            TS_W
            if (!TASK_SCHEDULER->frozen_tasks.has(current_task)){
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
    thread_pool->terminate_all_workers();
    delete thread_pool;
}

#undef TASK_SCHEDULER
#undef TS_R
#undef TS_W
#undef TS_RL
#undef TS_RU
#undef TS_WL
#undef TS_WU
