//
// Created by cycastic on 7/23/2023.
//

#include "../language/bytecode.h"
#include "nexus_stack.h"
#include "task.h"
#include "task_scheduler.h"

NexusExecutionState::~NexusExecutionState() { delete thread_stack; }

NexusExecutionState::NexusExecutionState(const Ref<NexusMethodPointer> &p_method_pointer) : method_pointer(p_method_pointer) {
    thread_stack = new NexusStack(p_method_pointer->get_type_info_server(),
                                  NexusRuntimeGlobalSettings::get_settings()->stack_size,
                                  NexusRuntimeGlobalSettings::get_settings()->stack_metadata_initial_capacity);
}

uint32_t Task::hash(const Task *p_task) { return p_task->get_id(); }

uint32_t Task::hash(const Ref<Task> &p_task) { return hash(p_task.ptr()); }

bool Task::compare(const Task *p_lhs, const Task *p_rhs) {
    return p_lhs->get_id() == p_lhs->get_id();
}

bool Task::compare(const Ref<Task> &p_lhs, const Ref<Task> &p_rhs) {
    return compare(p_lhs.ptr(), p_rhs.ptr());
}

Task::Task(TupleT2<Task::AsyncCallbackReturn, Ref<Task>> (*p_callback)(NexusExecutionState*),
           void (*p_resume_callback)(Ref<Task>, Ref<Task>),
           const Ref<NexusMethodPointer>& p_mp,
           const uint8_t& p_priority)
           : Task(TaskScheduler::get_singleton()->task_id_allocator.increment(), p_callback, p_resume_callback, p_mp, p_priority) {}

Task::Task(const uint32_t &p_id,
           TupleT2<Task::AsyncCallbackReturn, Ref<Task>> (*p_callback)(NexusExecutionState *),
           void (*p_resume_callback)(Ref<Task>, Ref<Task>),
           const Ref<NexusMethodPointer> &p_mp, const uint8_t& p_priority)
           : task_id(p_id), callback(p_callback), resume_callback(p_resume_callback), state(p_mp), priority(p_priority) {}

TupleT2<Task::AsyncCallbackReturn, Ref<Task>> Task::execute() const {
    return callback(const_cast<NexusExecutionState*>(&state));
}

void Task::handle_resume() {
    Ref<Task> as_ref = Ref<Task>::from_initialized_object(this);
    resume_callback(as_ref, get_child_task());
    set_child_task(Ref<Task>::null());
}

NexusExecutionState *Task::get_state() {
    return &state;
}

const NexusExecutionState *Task::get_state() const {
    return &state;
}

Task::~Task() = default;