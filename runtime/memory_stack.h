//
// Created by cycastic on 7/22/2023.
//

#ifndef NEXUS_MEMORY_STACK_H
#define NEXUS_MEMORY_STACK_H

#include "runtime_global_settings.h"
#include "../language/bytecode.h"
#include "../core/types/vector.h"
#include "../core/types/stack.h"

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
        if (sizeof(T) + stack_objects_size > NexusRuntimeGlobalSettings::get_settings().stack_size)
            throw MemoryStackException("Stack overflow");
        stack_objects_count++;
        stack_objects_data_type.push_back(p_type);
        stack_objects_offset.push_back(stack_objects_size);
        auto current_offset = stack_objects_size;
        stack_objects_size += sizeof(T);
        new (&stack_location[current_offset]) T(p_data);
    }

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
    _NO_DISCARD_ _FORCE_INLINE_ uint32_t count() const { return stack_objects_count; }
    _NO_DISCARD_ _FORCE_INLINE_ bool empty() const { return count() == 0; }
    _NO_DISCARD_ _FORCE_INLINE_ StackItem get_top() const {
        return StackItem {
                .type = stack_objects_data_type.last(),
                .data = &stack_location[stack_objects_offset.last()]
        };
    }
    _FORCE_INLINE_ void pop(){
        if (empty()) throw MemoryStackException("No object to pop");
        // Destroy object
        auto obj_type = stack_objects_data_type.last();
        auto obj_offset = stack_objects_offset.last();
        stack_objects_data_type.pop_back();
        stack_objects_offset.pop_back();
        stack_objects_count--;
        switch (obj_type) {
            case NexusSerializedBytecode::UNSIGNED_32_BIT_INTEGER:
            case NexusSerializedBytecode::SIGNED_32_BIT_INTEGER:
                stack_objects_size -= sizeof(uint32_t);
                break;
            case NexusSerializedBytecode::UNSIGNED_64_BIT_INTEGER:
            case NexusSerializedBytecode::SIGNED_64_BIT_INTEGER:
                stack_objects_size -= sizeof(uint64_t);
                break;
            case NexusSerializedBytecode::SINGLE_PRECISION_FLOATING_POINT:
                stack_objects_size -= sizeof(float);
                break;
            case NexusSerializedBytecode::DOUBLE_PRECISION_FLOATING_POINT:
                stack_objects_size -= sizeof(double);
                break;
            case NexusSerializedBytecode::STRING: {
                object_destroy<VString>(obj_offset);
                break;
            }
            case NexusSerializedBytecode::REFERENCE_COUNTED_OBJECT: {
                object_destroy<Ref<Object>>(obj_offset);
                break;
            }
            case NexusSerializedBytecode::VOID:
            case NexusSerializedBytecode::STRING_LITERAL:
            case NexusSerializedBytecode::METHOD:
            case NexusSerializedBytecode::NONE:
                throw MemoryStackException(CharString("Destructor not found for type: ") + itos(obj_type).utf8());
        }
    }
private:
    _FORCE_INLINE_ void recline(const uint32_t& p_to_size){
        if (p_to_size <= stack_objects_size) return;
        while (stack_objects_size > p_to_size){
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
    MemoryStack(){
        stack_location = (uint8_t *)malloc(NexusRuntimeGlobalSettings::get_settings().stack_size);
        stack_objects_data_type = Vector<NexusSerializedBytecode::DataType>();
        stack_objects_offset = Vector<size_t>();
        stack_frames = Stack<StackFrame>();
        stack_objects_count = 0;
        stack_objects_size = 0;
        allocate_stack_frame();
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
    _FORCE_INLINE_ void duplicate_top(){
        auto top = get_top();
        switch (top.type) {
            case NexusSerializedBytecode::UNSIGNED_32_BIT_INTEGER:
                push(*(uint32_t*)top.data);
                break;
            case NexusSerializedBytecode::SIGNED_32_BIT_INTEGER:
                push(*(int32_t*)top.data);
                break;
            case NexusSerializedBytecode::UNSIGNED_64_BIT_INTEGER:
                push(*(uint64_t*)top.data);
                break;
            case NexusSerializedBytecode::SIGNED_64_BIT_INTEGER:
                push(*(int64_t*)top.data);
                break;
            case NexusSerializedBytecode::SINGLE_PRECISION_FLOATING_POINT:
                push(*(float*)top.data);
                break;
            case NexusSerializedBytecode::DOUBLE_PRECISION_FLOATING_POINT:
                push(*(double*)top.data);
                break;
            case NexusSerializedBytecode::STRING:
                push(*(VString*)top.data);
                break;
            case NexusSerializedBytecode::REFERENCE_COUNTED_OBJECT:
                push(*(Ref<Object>*)top.data);
                break;
            case NexusSerializedBytecode::VOID:
            case NexusSerializedBytecode::STRING_LITERAL:
            case NexusSerializedBytecode::METHOD:
            case NexusSerializedBytecode::NONE:
                throw MemoryStackException(CharString("Cannot push object of unsupported type: ") + itos(top.type).utf8());
        }
    }
    _FORCE_INLINE_ void set_object(const uint32_t& p_idx, const void* p_item){
        if (count() <= p_idx) throw MemoryStackException(CharString("Invalid index: ") + itos(p_idx).utf8());
        auto obj_type = stack_objects_data_type[p_idx];
        auto obj_loc = stack_objects_offset[p_idx];
        switch (obj_type) {
            case NexusSerializedBytecode::UNSIGNED_32_BIT_INTEGER:
                set_object<uint32_t>(obj_loc, p_item);
                break;
            case NexusSerializedBytecode::SIGNED_32_BIT_INTEGER:
                set_object<int32_t>(obj_loc, p_item);
                break;
            case NexusSerializedBytecode::UNSIGNED_64_BIT_INTEGER:
                set_object<uint64_t>(obj_loc, p_item);
                break;
            case NexusSerializedBytecode::SIGNED_64_BIT_INTEGER:
                set_object<int64_t>(obj_loc, p_item);
                break;
            case NexusSerializedBytecode::SINGLE_PRECISION_FLOATING_POINT:
                set_object<float>(obj_loc, p_item);
                break;
            case NexusSerializedBytecode::DOUBLE_PRECISION_FLOATING_POINT:
                set_object<double>(obj_loc, p_item);
                break;
            case NexusSerializedBytecode::STRING:
                set_object<VString>(obj_loc, p_item);
                break;
            case NexusSerializedBytecode::REFERENCE_COUNTED_OBJECT:
            // TODO: Actually run type check
                set_object<Ref<Object>>(obj_loc, p_item);
                break;
            case NexusSerializedBytecode::VOID:
            case NexusSerializedBytecode::METHOD:
            case NexusSerializedBytecode::STRING_LITERAL:
            case NexusSerializedBytecode::NONE:
                throw MemoryStackException("This should not happen???");
        }
    }
    ~MemoryStack() {
        clear();
        free(stack_location);
    }
};

#endif //NEXUS_MEMORY_STACK_H
