//
// Created by cycastic on 7/23/2023.
//

#include "../language/bytecode.h"
#include "memory_stack.h"
#include "task.h"
#include "ambigous_value.h"

NexusExecutionState::NexusExecutionState(const Ref<NexusMethodPointer>& p_method_pointer, MemoryStack* p_mem_stack) : thread_stack(p_mem_stack), method_pointer(p_method_pointer), return_task() {}

NexusExecutionState::~NexusExecutionState() { delete thread_stack; }

Task::Task(const uint64_t &p_id) : Object(), task_id(p_id), return_value() {}

uint32_t Task::hash(const Ref<Task> &p_task) { return StandardHasher::hash(p_task->get_id()); }

uint32_t Task::hash(const Task *p_task) { return StandardHasher::hash(p_task->get_id()); }

bool Task::compare(const Task *p_lhs, const Task *p_rhs) {
    return p_lhs->get_id() == p_lhs->get_id();
}

bool Task::compare(const Ref<Task> &p_lhs, const Ref<Task> &p_rhs) {
    return compare(p_lhs.ptr(), p_rhs.ptr());
}

void Task::set_return_value(Ref<Task> p_task, const AmbiguousValue &p_value) {
    p_task->return_value = new AmbiguousValue(p_value);
    p_task->finished.set();
}

const AmbiguousValue *Task::get_return_value() const {
    return return_value;
}

Task::~Task() {
    delete return_value;
}
