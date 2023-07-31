//
// Created by cycastic on 7/22/2023.
//

#ifndef NEXUS_TASK_H
#define NEXUS_TASK_H

#include "../core/types/object.h"
#include "../core/types/tuple.h"

class TaskScheduler;
class MemoryStack;
class Task;
class NexusMethodPointer;
struct AmbiguousValue;

struct NexusExecutionState {
public:
    MemoryStack *thread_stack;
    Ref<NexusMethodPointer> method_pointer;

    explicit NexusExecutionState(const Ref<NexusMethodPointer>& p_method_pointer);
    ~NexusExecutionState();
};

class TaskScheduler;

class Task : public Object {
public:
    enum AsyncCallbackReturn {
        EXITED_SAFELY,
        EXCEPTION_THROWN,
        AWAIT,
    };
private:
    SafeFlag finished{false};
    uint32_t task_id;
    uint8_t priority;

    Ref<Task> child_task;
    TupleT2<Task::AsyncCallbackReturn, Ref<Task>> (*callback)(NexusExecutionState*);
    // resume_callback(original_task, child_task)
    void (*resume_callback)(Ref<Task>, Ref<Task>);
    NexusExecutionState state;

    friend class TaskScheduler;
    _FORCE_INLINE_ void set_finished() { finished.set(); }
    explicit Task(const uint32_t& p_id,
                  TupleT2<Task::AsyncCallbackReturn, Ref<Task>> (*p_callback)(NexusExecutionState*),
                  void (*p_resume_callback)(Ref<Task>, Ref<Task>),
                  const Ref<NexusMethodPointer>& p_mp,
                  const uint8_t& p_priority);
public:
    _FORCE_INLINE_ uint32_t get_id() const { return task_id; }
    _FORCE_INLINE_ uint8_t get_priority() const { return priority; }
    _FORCE_INLINE_ bool is_finished() const { return finished.is_set(); }
    _FORCE_INLINE_ void wait() const {
        finished.wait();
    }
    static uint32_t hash(const Ref<Task>& p_task);
    static uint32_t hash(const Task* p_task);
    static bool compare(const Ref<Task>& p_lhs, const Ref<Task>& p_rhs);
    static bool compare(const Task* p_lhs, const Task* p_rhs);

    _ALWAYS_INLINE_ void set_child_task(const Ref<Task>& p_child_task) { child_task = p_child_task; }
    _ALWAYS_INLINE_ Ref<Task> get_child_task() const { return child_task; }

    NexusExecutionState* get_state();
    const NexusExecutionState* get_state() const;
    TupleT2<Task::AsyncCallbackReturn, Ref<Task>> execute() const;
    void handle_resume();

    explicit Task(TupleT2<Task::AsyncCallbackReturn, Ref<Task>> (*p_callback)(NexusExecutionState*),
                  void (*p_resume_callback)(Ref<Task>, Ref<Task>),
                  const Ref<NexusMethodPointer>& p_mp,
                  const uint8_t& p_priority = 2);
    ~Task() override;
};

#endif //NEXUS_TASK_H
