//
// Created by cycastic on 7/22/2023.
//

#ifndef NEXUS_TASK_SCHEDULER_H
#define NEXUS_TASK_SCHEDULER_H

#include "task.h"
#include "../core/exception.h"
#include "../core/types/hashmap.h"

struct AmbiguousValue;


class TaskSchedulerException : public Exception {
public:
    explicit TaskSchedulerException(const char* p_msg) : Exception(p_msg) {}
};

class TaskScheduler {
private:
    HashMap<Ref<Task>, Task::TaskHandlerData*, 64, Task, Task> state_to_return{};
    SafeNumeric<uint64_t> task_id_allocator{0};
    SharedMutex lock{};

    static TaskScheduler* singleton;

    static void task_handler(void* p_userdata);
    static void standard_bytecode_task_handler(Task::TaskHandlerData* p_data);
    static Task::AsyncCallbackReturn standard_bytecode_evaluator(Task::TaskHandlerData* state);
    static AmbiguousValue standard_stack_return_probe(NexusExecutionState* p_state);

    static void call_task_handler(Task::TaskHandlerData* data);
    static _FORCE_INLINE_ TaskScheduler* get_singleton() { return singleton; }
public:
    ~TaskScheduler();

    static void allocate_thread();
    static void call(const Ref<NexusMethodPointer>& p_method_pointer);
    static Ref<Task> allocate_task();

    static void destroy() { singleton->~TaskScheduler(); singleton = nullptr; }
};

#endif //NEXUS_TASK_SCHEDULER_H
