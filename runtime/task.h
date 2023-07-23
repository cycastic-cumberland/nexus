//
// Created by cycastic on 7/22/2023.
//

#ifndef NEXUS_TASK_H
#define NEXUS_TASK_H

#include "../core/types/object.h"

class TaskScheduler;
class MemoryStack;
class Task;
class NexusMethodPointer;
struct AmbiguousValue;

struct NexusExecutionState {
    MemoryStack *thread_stack;
    Ref<NexusMethodPointer> method_pointer;
    Ref<Task> return_task;

    NexusExecutionState(const Ref<NexusMethodPointer>& p_method_pointer, MemoryStack* p_mem_stack);
    ~NexusExecutionState();
};

class TaskScheduler;

typedef AmbiguousValue (*probe_return_value)(NexusExecutionState*);

class Task : public Object {
public:
    enum AsyncCallbackReturn {
        EXITED_SAFELY,
        EXCEPTION_THROWN,
        AWAIT,
    };
    struct TaskHandlerData {
        AsyncCallbackReturn (*callback)(TaskHandlerData*);
        NexusExecutionState* execution_state;
        TaskHandlerData* awaiting;
    };
private:
    AmbiguousValue* return_value;
    SafeFlag finished{false};
    uint64_t task_id;

    friend class TaskScheduler;
public:
    static void set_return_value(Ref<Task> p_task, const AmbiguousValue& p_value);
    _FORCE_INLINE_ uint64_t get_id() const { return task_id; }
    const AmbiguousValue* get_return_value() const;
    _FORCE_INLINE_ bool is_finished() const { return finished.is_set(); }
    _FORCE_INLINE_ void wait() const {
        while (!finished.is_set()) {  }
    }
    static uint32_t hash(const Ref<Task>& p_task);
    static uint32_t hash(const Task* p_task);
    static bool compare(const Ref<Task>& p_lhs, const Ref<Task>& p_rhs);
    static bool compare(const Task* p_lhs, const Task* p_rhs);

    explicit Task(const uint64_t& p_id);
    ~Task() override;
};

#endif //NEXUS_TASK_H
