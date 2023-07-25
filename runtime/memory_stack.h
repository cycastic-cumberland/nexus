//
// Created by cycastic on 7/22/2023.
//

#ifndef NEXUS_MEMORY_STACK_H
#define NEXUS_MEMORY_STACK_H

#include "ambigous_value.h"
#include "../language/bytecode.h"
#include "../core/types/vector.h"
#include "../core/types/stack.h"

struct StackStructMetadata;

class MemoryStackException : public Exception {
public:
    explicit MemoryStackException(const char* p_msg) : Exception(p_msg) {}
    explicit MemoryStackException(const CharString& p_msg) : Exception(p_msg) {}
};

class MemoryStack {
public:
    struct StackItem {
        NexusSerializedBytecode::DataType type;
        void* data;

        _NO_DISCARD_ AmbiguousValue to_ambiguous() const { return {const_cast<StackItem*>(this)->data, type}; }
    };
    struct StackFrame {
        uint32_t objects_count_offset;
    };
private:
    uint8_t* stack_location;
    uint32_t stack_objects_count;
    size_t stack_objects_size;
    Vector<NexusSerializedBytecode::DataType> stack_objects_data_type;
    Vector<size_t> stack_objects_offset;
    Stack<StackFrame> stack_frames;

    template<class T>
    _FORCE_INLINE_ void push(const T& p_data, const NexusSerializedBytecode::DataType& p_type){
        if (sizeof(T) + stack_objects_size > NexusRuntimeGlobalSettings::get_settings()->stack_size)
            throw MemoryStackException("Stack overflow");
        stack_objects_count++;
        stack_objects_data_type.push_back(p_type);
        stack_objects_offset.push_back(stack_objects_size);
        auto current_offset = stack_objects_size;
        stack_objects_size += sizeof(T);
        new (&stack_location[current_offset]) T(p_data);
    }
    void push_ambiguous(const AmbiguousValue& p_ambiguous);

    template<class T>
    _FORCE_INLINE_ void object_destroy(const size_t& p_obj_offset){
        ((T*)&stack_location[p_obj_offset])->~T();
        stack_objects_size = p_obj_offset;
    }

    template<class T>
    _FORCE_INLINE_ void set_object(const size_t& p_loc, const void* p_data){
        *(T*)(&stack_location[p_loc]) = *(T*)p_data;
    }
public:
    _NO_DISCARD_ _FORCE_INLINE_ size_t data_size() const { return stack_objects_size; }
    _NO_DISCARD_ _FORCE_INLINE_ uint32_t objects_count() const { return stack_objects_count; }
    _NO_DISCARD_ _FORCE_INLINE_ bool empty() const { return objects_count() == 0; }
    _NO_DISCARD_ _FORCE_INLINE_ StackItem free_probe(const size_t& p_absolute_pos) const {
        return StackItem {
                .type = stack_objects_data_type[p_absolute_pos],
                .data = &stack_location[stack_objects_offset[p_absolute_pos]]
        };
    }
    void pop();
private:
    _FORCE_INLINE_ void recline(const uint32_t& p_to_size){
        if (p_to_size > stack_objects_count) return;
        while (stack_objects_count > p_to_size){
            pop();
        }
    }
    _FORCE_INLINE_ void clear(){
        while (!empty()) pop();
    }
public:
    _FORCE_INLINE_ const StackFrame& allocate_stack_frame(){
        if (!stack_frames.empty()){
            if (stack_frames.peek_last().objects_count_offset == stack_objects_count)
                throw MemoryStackException("Stack frame already allocated at this offset");
        }
        stack_frames.push({
            .objects_count_offset = stack_objects_count
        });
        return stack_frames.peek_last();
    }
    _FORCE_INLINE_ void deallocate_stack_frame(){
        auto frame = stack_frames.pop();
        recline(frame.objects_count_offset);
    }
    _NO_DISCARD_ _FORCE_INLINE_ uint32_t allocated_objects_in_this_frame() const {
        if (stack_frames.empty()) return 0;
        return stack_objects_count - stack_frames.peek_last().objects_count_offset;
    }
    _NO_DISCARD_ _FORCE_INLINE_ uint32_t get_stack_frames_count() const {
        return stack_frames.size();
    }
    _NO_DISCARD_ _FORCE_INLINE_ bool empty_stack() const {
        return allocated_objects_in_this_frame() == 0;
    }
    MemoryStack(){
        stack_location = (uint8_t *)malloc(NexusRuntimeGlobalSettings::get_settings()->stack_size);
        stack_objects_data_type = Vector<NexusSerializedBytecode::DataType>();
        stack_objects_offset = Vector<size_t>();
        stack_frames = Stack<StackFrame>();
        stack_objects_count = 0;
        stack_objects_size = 0;
        allocate_stack_frame();
    }
    _FORCE_INLINE_ void push(const AmbiguousValue& p_ambiguous){
        push_ambiguous(p_ambiguous);
    }
    _FORCE_INLINE_ void push(const uint32_t& p_data){
        push(p_data, NexusSerializedBytecode::UNSIGNED_32_BIT_INTEGER);
    }
    _FORCE_INLINE_ void push(const int32_t& p_data){
        push(p_data, NexusSerializedBytecode::SIGNED_32_BIT_INTEGER);
    }
    _FORCE_INLINE_ void push(const uint64_t& p_data){
        push(p_data, NexusSerializedBytecode::UNSIGNED_64_BIT_INTEGER);
    }
    _FORCE_INLINE_ void push(const int64_t& p_data){
        push(p_data, NexusSerializedBytecode::SIGNED_64_BIT_INTEGER);
    }
    _FORCE_INLINE_ void push(const float& p_data){
        push(p_data, NexusSerializedBytecode::SINGLE_PRECISION_FLOATING_POINT);
    }
    _FORCE_INLINE_ void push(const double& p_data){
        push(p_data, NexusSerializedBytecode::DOUBLE_PRECISION_FLOATING_POINT);
    }
    _FORCE_INLINE_ void push(const VString& p_data){
        push(p_data, NexusSerializedBytecode::STRING);
    }
    _FORCE_INLINE_ void push(const Ref<Object>& p_data){
        push(p_data, NexusSerializedBytecode::REFERENCE_COUNTED_OBJECT);
    }
    void push(const StackStructMetadata& p_struct_metadata);
    _NO_DISCARD_ _FORCE_INLINE_ StackItem get_top() const {
        const auto& last_frame = stack_frames.peek_last();
        if (last_frame.objects_count_offset == stack_objects_count)
            throw MemoryStackException("No value at top");
        return free_probe(stack_objects_count - 1);
    }
    _NO_DISCARD_ _FORCE_INLINE_ StackItem get_at(const int32_t& p_relative_pos) const {
        const auto& last_frame = stack_frames.peek_last();
        auto absolute = p_relative_pos >= 0 ?
                        int32_t(last_frame.objects_count_offset) + p_relative_pos :
                        int32_t(stack_objects_count) + p_relative_pos;
        if (last_frame.objects_count_offset > absolute || absolute > stack_objects_count)
            throw MemoryStackException("Invalid stack index");
        return free_probe(absolute);
    }
    _FORCE_INLINE_ void duplicate_top(){
        auto top = get_top().to_ambiguous();
        push(top);
    }
    _FORCE_INLINE_ void copy_to_top(const int32_t& p_relative_pos){
        auto item = get_at(p_relative_pos).to_ambiguous();
        push(item);
    }
    void set_object(const int32_t& p_relative_idx, const AmbiguousValue& p_ambiguous);
    void set_object(const int32_t& p_relative_idx, const void* p_item);
    ~MemoryStack() {
        clear();
        free(stack_location);
    }
};

#endif //NEXUS_MEMORY_STACK_H
