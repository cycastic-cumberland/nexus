//
// Created by cycastic on 7/22/2023.
//

#include "task_scheduler.h"
#include "../language/bytecode.h"
#include "threads_manager.h"
#include "memory_stack.h"

TaskScheduler* TaskScheduler::singleton = new TaskScheduler();

void TaskScheduler::allocate_thread() {
    ThreadsManager::create_new_thread(WorkerThread::SHARED);
}

void TaskScheduler::call(const Ref<NexusMethodPointer>& p_method_pointer) {
    auto data = new Task::TaskHandlerData{
        .callback = standard_bytecode_evaluator,
        .execution_state = new NexusExecutionState(p_method_pointer, new MemoryStack()),
        .awaiting = nullptr,
    };
    call_task_handler(data);
}

void TaskScheduler::task_handler(void *p_userdata) {
    standard_bytecode_task_handler((Task::TaskHandlerData*)p_userdata);
}

Task::AsyncCallbackReturn TaskScheduler::standard_bytecode_evaluator(Task::TaskHandlerData *state) {
    return Task::EXITED_SAFELY;
}

void TaskScheduler::standard_bytecode_task_handler(Task::TaskHandlerData *p_data) {
    auto evaluation_result = p_data->callback(p_data);
    switch (evaluation_result) {
        case Task::EXITED_SAFELY: {
            auto curr_task = p_data->execution_state->return_task;
            if (curr_task.is_valid()){
                auto return_result = standard_stack_return_probe(p_data->execution_state);
                if (return_result.get_type() != NexusSerializedBytecode::NONE)
                    Task::set_return_value(curr_task, return_result);
                GUARD(get_singleton()->lock);
                auto original_data = get_singleton()->state_to_return[curr_task];
                get_singleton()->state_to_return.erase(curr_task);
                call_task_handler(original_data);
            }
            delete p_data->execution_state;
            break;
        }
        case Task::AWAIT: {
            get_singleton()->lock.lock();
            get_singleton()->state_to_return[p_data->execution_state->return_task] = p_data;
            get_singleton()->lock.unlock();
            call_task_handler(p_data->awaiting);
            break;
        }
        case Task::EXCEPTION_THROWN:
            delete p_data->execution_state;
            throw TaskSchedulerException("Exception not yet supported");
    }
    delete p_data;
}

Ref<Task> TaskScheduler::allocate_task() {
    return Ref<Task>::make_ref(get_singleton()->task_id_allocator.increment());
}

void TaskScheduler::call_task_handler(Task::TaskHandlerData *data) {
    auto thread = ThreadsManager::create_new_thread(WorkerThread::ENCLOSED);
    thread->add_work(task_handler, (void*)data);
}

AmbiguousValue TaskScheduler::standard_stack_return_probe(NexusExecutionState *p_state) {
    // TODO: Check if function return anything, if true, get topmost value, if false, return void
    return {};
}

TaskScheduler::~TaskScheduler() = default;
