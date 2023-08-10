//
// Created by cycastic on 8/3/2023.
//

#include "nexus_stack.h"
#include "../core/types/interned_string.h"


void empty_destructor(const StackItemMetadata* ignored, void* p_data) {  }

// TODO: Perform type comparison during Ref<ManagedObject> copy constructor

void StackStructItemMetadata::default_struct_constructor(const StackItemMetadata *p_metadata, void *p_mem_region) {
    auto struct_metadata = dynamic_cast<const StackStructItemMetadata*>(p_metadata);
    size_t offset = 0;
    for (const auto* it = struct_metadata->struct_items.first(); it; it = it->next()){
        auto metadata = it->data;
        metadata->vtable->constructor(metadata, (void*)((size_t)p_mem_region + offset));
        offset += metadata->data_size;
    }
}

void StackStructItemMetadata::default_struct_copy_constructor(const StackItemMetadata *p_metadata, void *p_mem_region, const void* p_copy_target_region) {
    auto struct_metadata = dynamic_cast<const StackStructItemMetadata*>(p_metadata);
    size_t offset = 0;
    for (const auto* it = struct_metadata->struct_items.first(); it; it = it->next()){
        auto metadata = it->data;
        metadata->vtable->copy_constructor(metadata, (void*)((size_t)p_mem_region + offset), (void*)((size_t)p_copy_target_region + offset));
        offset += metadata->data_size;
    }
}

void StackStructItemMetadata::default_assignment_operator(const StackItemMetadata *p_metadata, void *p_mem_region,
                                                          const void *p_copy_target_region) {
    auto struct_metadata = dynamic_cast<const StackStructItemMetadata*>(p_metadata);
    size_t offset = 0;
    for (const auto* it = struct_metadata->struct_items.first(); it; it = it->next()){
        auto metadata = it->data;
        metadata->vtable->op_assign(metadata, (void*)((size_t)p_mem_region + offset), (void*)((size_t)p_copy_target_region + offset));
        offset += metadata->data_size;
    }
}

void StackStructItemMetadata::default_struct_destructor(const StackItemMetadata *p_metadata, void *p_mem_region) {
    auto struct_metadata = dynamic_cast<const StackStructItemMetadata*>(p_metadata);
    size_t offset = 0;
    for (const auto* it = struct_metadata->struct_items.first(); it; it = it->next()){
        auto metadata = it->data;
        metadata->vtable->destructor(metadata, (void*)((size_t)p_mem_region + offset));
        offset += metadata->data_size;
    }
}

void StackStructItemMetadata::build_cache() {
    data_size = 0;
    for (const auto* it = struct_items.first(); it; it = it->next()){
        data_size += it->data->data_size;
    }
}

void NexusTypeInfoServer::add_builtin_types() {
#define ADD_TYPE(data_type, representation, constructor_cb, copy_constructor_cp, assignment_cb, destructor_cb){     \
    auto vtable = new NexusBaseVtable(constructor_cb, copy_constructor_cp,                                          \
                                      assignment_cb, destructor_cb);                                                \
    add_vtable(vtable, (data_type));                                                                                \
    add_metadata(new StackItemMetadata(                                                                             \
        (StackItemMetadata::ID)(data_type),                                                                         \
        (data_type),                                                                                                \
        sizeof(representation),                                                                                     \
        vtable                                                                                                      \
    ), (data_type));}
    {
        auto vtable = new NexusBaseVtable(StackStructItemMetadata::default_struct_constructor,
                                          StackStructItemMetadata::default_struct_copy_constructor,
                                          StackStructItemMetadata::default_assignment_operator,
                                          StackStructItemMetadata::default_struct_destructor);
        add_vtable(vtable, NexusStandardType::STACK_STRUCT);
    }
    // No STACK_STRUCT metadata, as it should not be situated on the stack, all struct type must have its custom metadata class
    ADD_TYPE(NexusStandardType::UNSIGNED_32_BIT_INTEGER, uint32_t, numeric_constructor<uint32_t>,
             generic_copy_constructor<uint32_t>, generic_copy_assignment<uint32_t>, empty_destructor)
    ADD_TYPE(NexusStandardType::SIGNED_32_BIT_INTEGER, int32_t, numeric_constructor<int32_t>,
             generic_copy_constructor<int32_t>, generic_copy_assignment<int32_t>, empty_destructor)
    ADD_TYPE(NexusStandardType::UNSIGNED_64_BIT_INTEGER, uint64_t, numeric_constructor<uint64_t>,
             generic_copy_constructor<uint64_t>, generic_copy_assignment<uint64_t>, empty_destructor)
    ADD_TYPE(NexusStandardType::UNSIGNED_64_BIT_INTEGER, int64_t, numeric_constructor<int64_t>,
             generic_copy_constructor<int64_t>, generic_copy_assignment<int64_t>, empty_destructor)
    ADD_TYPE(NexusStandardType::SINGLE_PRECISION_FLOATING_POINT, float, numeric_constructor<float>,
             generic_copy_constructor<float>, generic_copy_assignment<float>, empty_destructor)
    ADD_TYPE(NexusStandardType::DOUBLE_PRECISION_FLOATING_POINT, double, numeric_constructor<double>,
             generic_copy_constructor<double>, generic_copy_assignment<double>, empty_destructor)
    ADD_TYPE(NexusStandardType::STRING_LITERAL, InternedString, generic_constructor<InternedString>,
             generic_copy_constructor<InternedString>, generic_copy_assignment<InternedString>, generic_destructor<InternedString>)
    ADD_TYPE(NexusStandardType::STRING, VString, generic_constructor<VString>,
             generic_copy_constructor<VString>, generic_copy_assignment<VString>, generic_destructor<VString>)
    ADD_TYPE(NexusStandardType::REFERENCE_COUNTED_OBJECT, Ref<ManagedObject>, generic_constructor<Ref<ManagedObject>>,
             generic_copy_constructor<Ref<ManagedObject>>, generic_copy_assignment<Ref<ManagedObject>>, generic_destructor<Ref<ManagedObject>>)
    ADD_TYPE(NexusStandardType::METHOD, size_t, numeric_constructor<size_t>,
             generic_copy_constructor<size_t>, generic_copy_assignment<size_t>, empty_destructor)
    // No NONE

    // Skip an ID? Whatever...
    metadata_id_allocator.set(NexusStandardType::MAX_TYPE);
    vtable_id_allocator.set(NexusStandardType::MAX_TYPE);
#undef ADD_TYPE
}

NexusTypeInfoServer::NexusTypeInfoServer(bool p_is_multi_threaded) {
    lock = p_is_multi_threaded ? new RWLock() : new InertRWLock();
    add_builtin_types();
}

NexusTypeInfoServer::~NexusTypeInfoServer() {
    lock->write_lock();
    while (!metadata_record.empty()){
        delete metadata_record.first()->data;
        metadata_record.erase(metadata_record.first());
    }
    while (!vtable_record.empty()){
        delete vtable_record.first()->data;
        vtable_record.erase(vtable_record.first());
    }
    lock->write_unlock();
    delete lock;
}

StackItemMetadata::ID NexusTypeInfoServer::add_metadata(StackItemMetadata *p_metadata, const StackItemMetadata::ID &p_id) {
    p_metadata->type_id = p_id;
    metadata_record.add_last(p_metadata);
    metadata_map[p_metadata->type_id] = p_metadata;
    return p_id;
}

NexusBaseVtable::ID NexusTypeInfoServer::add_vtable(NexusBaseVtable *p_vtable, const NexusBaseVtable::ID &p_id) {
    vtable_record.add_last(p_vtable);
    vtable_map[p_id] = p_vtable;
    return p_id;
}

NexusStack::Frame::Frame(NexusStack *p_stack)
    : parent(p_stack), current_object_count(0)
{
    memory_offset = (void*)(parent->allocated + (size_t)parent->stack_begin);
    object_offset = parent->current_object_count;
}

NexusStack::Frame::Frame(Frame&& p_other) noexcept {
    parent = p_other.parent;
    object_offset = p_other.object_offset;
    memory_offset = p_other.memory_offset;
    current_object_count = p_other.current_object_count;

    p_other.parent = nullptr;
    p_other.object_offset = 0;
    p_other.memory_offset = nullptr;
    p_other.current_object_count = 0;
}

NexusStack::Frame::~Frame() {
    clear();
}

void NexusStack::Frame::register_object(const StackItemMetadata *p_metadata) {
    auto last_allocation = memory_offset;
    memory_offset = (void*)((size_t)memory_offset + p_metadata->data_size);
    parent->current_object_count++;
    parent->allocated += p_metadata->data_size;
    parent->object_info.push_back(ObjectInfo{
        .type = p_metadata,
        .data = last_allocation,
        .index = current_object_count++
    });
}

void NexusStack::Frame::push(const NexusStack::ObjectInfo &p_info) {
    if (!p_info.is_valid()) throw NexusStackException("p_info is invalid");
    push(p_info.type, p_info.data);
}

void NexusStack::Frame::push_empty(const StackItemMetadata *p_metadata) {
    if (!p_metadata) throw NexusStackException("p_metadata is null");
    p_metadata->vtable->constructor(p_metadata, memory_offset);
    register_object(p_metadata);
}

void NexusStack::Frame::push(const StackItemMetadata* p_metadata, const void* p_copy_from) {
    if (!p_metadata) throw NexusStackException("p_metadata is null");
    if (parent->allocated + p_metadata->data_size > parent->max_stack_size) throw NexusStackException("Stack overflow");
    p_metadata->vtable->copy_constructor(p_metadata, memory_offset, p_copy_from);
    register_object(p_metadata);
}

void NexusStack::Frame::push(const uint32_t &p_u32) {
    push_primitive(NexusStandardType::UNSIGNED_32_BIT_INTEGER, p_u32);
}

void NexusStack::Frame::push(const int32_t &p_i32) {
    push_primitive(NexusStandardType::SIGNED_32_BIT_INTEGER, p_i32);
}

void NexusStack::Frame::push(const uint64_t &p_u64) {
    push_primitive(NexusStandardType::UNSIGNED_64_BIT_INTEGER, p_u64);
}

void NexusStack::Frame::push(const int64_t &p_i64) {
    push_primitive(NexusStandardType::SIGNED_64_BIT_INTEGER, p_i64);
}

void NexusStack::Frame::push(const float &p_f32) {
    push_primitive(NexusStandardType::SINGLE_PRECISION_FLOATING_POINT, p_f32);
}

void NexusStack::Frame::push(const double &p_f64) {
    push_primitive(NexusStandardType::DOUBLE_PRECISION_FLOATING_POINT, p_f64);
}

void NexusStack::Frame::push(const wchar_t *p_str) {
    push_primitive(NexusStandardType::STRING_LITERAL, InternedString(p_str));
}

void NexusStack::Frame::push(const InternedString& p_str) {
    push_primitive(NexusStandardType::STRING_LITERAL, p_str);
}

void NexusStack::Frame::push(const VString &p_string) {
    push_primitive(NexusStandardType::STRING, p_string);
}

void NexusStack::Frame::push(const Ref<ManagedObject> &p_ref_counted) {
    push_primitive(NexusStandardType::REFERENCE_COUNTED_OBJECT, p_ref_counted);
}

void NexusStack::Frame::set(const int64_t &p_idx, const NexusStack::ObjectInfo &p_info) const {
    auto object = get_at(p_idx);
    if (!object.is_valid()) throw NexusStackException("Invalid stack frame index");
    set_object(object, p_info.type, p_info.data);
}

void NexusStack::Frame::set(const int64_t &p_idx, const StackItemMetadata *p_metadata, const void *p_copy_from) const {
    auto object = get_at(p_idx);
    if (!object.is_valid()) throw NexusStackException("Invalid stack frame index");
    set_object(object, p_metadata, p_copy_from);
}

void NexusStack::Frame::set(const int64_t &p_idx, const uint32_t &p_u32) const {
    set_primitive(p_idx, NexusStandardType::UNSIGNED_32_BIT_INTEGER, p_u32);
}

void NexusStack::Frame::set(const int64_t &p_idx, const int32_t &p_i32) const {
    set_primitive(p_idx, NexusStandardType::SIGNED_32_BIT_INTEGER, p_i32);
}

void NexusStack::Frame::set(const int64_t &p_idx, const uint64_t &p_u64) const {
    set_primitive(p_idx, NexusStandardType::UNSIGNED_64_BIT_INTEGER, p_u64);
}

void NexusStack::Frame::set(const int64_t &p_idx, const int64_t &p_i64) const {
    set_primitive(p_idx, NexusStandardType::SIGNED_64_BIT_INTEGER, p_i64);
}

void NexusStack::Frame::set(const int64_t &p_idx, const float &p_f32) const {
    set_primitive(p_idx, NexusStandardType::SINGLE_PRECISION_FLOATING_POINT, p_f32);
}

void NexusStack::Frame::set(const int64_t &p_idx, const double &p_f64) const {
    set_primitive(p_idx, NexusStandardType::DOUBLE_PRECISION_FLOATING_POINT, p_f64);
}

void NexusStack::Frame::set(const int64_t &p_idx, const wchar_t *p_str) const {
    throw NexusStackException("Can not set an InternedString's value");
//    set_primitive(p_idx, NexusStandardType::STRING_LITERAL, InternedString(p_str));
}

void NexusStack::Frame::set(const int64_t &p_idx, const InternedString &p_str) const {
    throw NexusStackException("Can not set an InternedString's value");
//    set_primitive(p_idx, NexusStandardType::UNSIGNED_32_BIT_INTEGER, p_str);
}

void NexusStack::Frame::set(const int64_t &p_idx, const VString &p_string) const {
    set_primitive(p_idx, NexusStandardType::STRING_LITERAL, p_string);
}

void NexusStack::Frame::set(const int64_t &p_idx, const Ref<ManagedObject> &p_ref_counted) const {
    set_primitive(p_idx, NexusStandardType::REFERENCE_COUNTED_OBJECT, p_ref_counted);
}

void NexusStack::Frame::pop() {
    if (empty()) throw NexusStackException("Stack frame is empty");
    const auto& last_object_info = parent->object_info.last();
    const auto* metadata = last_object_info.type;
    memory_offset = last_object_info.data;
    parent->current_object_count--;
    current_object_count--;
    parent->allocated -= metadata->data_size;
    metadata->vtable->destructor(metadata, memory_offset);

    // If current frame is not the latest frame, this will lead to undefined behavior
    parent->object_info.pop_back();
}

void NexusStack::Frame::push_object(const StackItemMetadata *p_metadata, const void* p_data) {
    p_metadata->vtable->copy_constructor(p_metadata, memory_offset, p_data);
    register_object(p_metadata);
}

void NexusStack::Frame::set_object(const NexusStack::ObjectInfo &p_info, const StackItemMetadata *p_metadata,
                                   const void* p_data) {
    // Simple pointers comparison, comment out for extra performance *wink* *wink*
    if (p_info.type != p_metadata) throw NexusStackException("Can not set object of incompatible type");
    p_info.set_primitive(p_data);
}

void NexusStack::Frame::copy_to_top(const int64_t &p_idx) {
    auto object = get_at(p_idx);
    if (!object.is_valid()) throw NexusStackException("Invalid stack frame index");
    push_object(object.type, object.data);
}

NexusStack::~NexusStack() {
    stack_frames.clear();
    free(stack_begin);
}

NexusStack::NexusStack(const NexusTypeInfoServer *p_type_info_server, const size_t &p_stack_size, const size_t& p_initial_frame_capacity)
: stack_begin(malloc(p_stack_size)), max_stack_size(p_stack_size),
allocated(0), current_object_count(0), stack_frames(p_initial_frame_capacity),
type_info_server(p_type_info_server), object_info(p_initial_frame_capacity) {}

Box<NexusStack::Frame, ThreadUnsafeObject> &NexusStack::push_stack_frame() {
    stack_frames.emplace(Box<Frame, ThreadUnsafeObject>::make_box(this));
    return get_last_frame();
}

bool NexusStack::pop_stack_frame() {
    if (empty()) return false;
    stack_frames.safe_pop();
    return true;
}

Box<NexusStack::Frame, ThreadUnsafeObject> &NexusStack::get_last_frame() {
    return stack_frames.modify_last();
}

const Box<NexusStack::Frame, ThreadUnsafeObject> &NexusStack::get_last_frame() const {
    return stack_frames.peek_last();
}

const Box<NexusStack::Frame, ThreadUnsafeObject> &NexusStack::get_frame_at(const int64_t &p_idx) const {
    return stack_frames.peek_at(p_idx);
}
