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


void TaskScheduler::queue_task_internal(const Ref<Task> &p_task) {
    TASK_SCHEDULER->tasks_queue.enqueue(p_task);
}
void TaskScheduler::queue_task(const Ref<Task> &p_task) {
    TS_W
    queue_task_internal(p_task);
}

void TaskScheduler::initialize_task_scheduler() {
    TS_W
    if (TASK_SCHEDULER->poll_thread_id != 0) return;
    TASK_SCHEDULER->poll_thread_id = ThreadsManager::create_new_thread(WorkerThread::ENCLOSED);
    ThreadsManager::add_work(TASK_SCHEDULER->poll_thread_id, task_scheduler_daemon, nullptr);
}

void TaskScheduler::task_scheduler_daemon(void *p_ignored) {
    while (!TASK_SCHEDULER->is_terminating.is_set()){
        for (auto completed = 0; completed < NexusRuntimeGlobalSettings::get_settings()->task_scheduler_max_request_per_cycle; completed++){
            TS_WL
            if (TASK_SCHEDULER->tasks_queue.empty()) {
                TS_WU
                break;
            }
            Ref<Task> current_request = TASK_SCHEDULER->tasks_queue.dequeue();
            TS_WU
            // Temporarily increase ref count, so it could survive until a task takes it in
            current_request.ptr()->ref();
            auto result = ThreadsManager::populate_free_thread(task_handler, current_request.ptr());
            // If failed to allocate any thread, yield for the time and return at a later time
            if (!result) {
                // If failed to populate a thread, decrement
                current_request.ptr()->unref();
                break;
            }
        }
        System::get_singleton()->yield();
    }
}

TaskScheduler::TaskScheduler() {
    singleton = this;
    initialize_task_scheduler();
}

void TaskScheduler::task_handler(void *p_async_request) {
    auto raw_request = (Task*)p_async_request;
    // Decrement previous ref
    raw_request->unref();
    // If somehow the object has been disconnected from all other source except this one,
    // Reinitialize the ref count
    Ref<Task> current_task = Ref<Task>::null();
    if (raw_request->get_reference_count() == 0) current_task = Ref<Task>::from_uninitialized_object(raw_request);
    else current_task = Ref<Task>::from_initialized_object(raw_request);
    // If there's no task_object, then it should do nothing
    current_task->handle_resume();
    auto result = current_task->execute();
    Task::AsyncCallbackReturn async_return = Task::EXITED_SAFELY;
    Ref<Task> branched_into = Ref<Task>::null();
    result.unpack(async_return, branched_into);
    switch (async_return) {
        case Task::EXITED_SAFELY:{
            TS_WL
            if (!TASK_SCHEDULER->frozen_tasks.exists(current_task)){
                // This is a genesis task, do nothing
            } else {
                // This is spawned from another task, resuming it
                TASK_SCHEDULER->frozen_tasks.erase(current_task);
                queue_task_internal(TASK_SCHEDULER->frozen_tasks[current_task]);
            }
            TS_WU
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
    ThreadsManager::terminate_thread(poll_thread_id);
}

#undef TASK_SCHEDULER
#undef TS_R
#undef TS_W
#undef TS_RL
#undef TS_RU
#undef TS_WL
#undef TS_WU
