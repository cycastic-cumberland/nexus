#include "memory_stack.h"
#include "../core/types/stack_struct.h"

void MemoryStack::push_ambiguous(const AmbiguousValue& p_ambiguous){
    // Max size per value is 64 bit anyway...
    if (sizeof(uint64_t) + stack_objects_size > NexusRuntimeGlobalSettings::get_settings()->stack_size)
        throw MemoryStackException("Stack overflow");
    stack_objects_count++;
    stack_objects_data_type.push_back(p_ambiguous.get_type());
    stack_objects_offset.push_back(stack_objects_size);
    auto current_offset = stack_objects_size;
#define PUSH_AMBIGUOUS_ITEM(type) {                                                         \
stack_objects_size += sizeof(type);                                                     \
new (&stack_location[current_offset]) type(*(type*)(p_ambiguous.get_raw_pointer()));    \
break;                                                                                  \
}
    switch (p_ambiguous.get_type()) {
        case NexusSerializedBytecode::UNSIGNED_32_BIT_INTEGER:
            PUSH_AMBIGUOUS_ITEM(uint32_t)
        case NexusSerializedBytecode::SIGNED_32_BIT_INTEGER:
            PUSH_AMBIGUOUS_ITEM(int32_t)
        case NexusSerializedBytecode::UNSIGNED_64_BIT_INTEGER:
            PUSH_AMBIGUOUS_ITEM(uint64_t)
        case NexusSerializedBytecode::SIGNED_64_BIT_INTEGER:
            PUSH_AMBIGUOUS_ITEM(int64_t)
        case NexusSerializedBytecode::SINGLE_PRECISION_FLOATING_POINT:
            PUSH_AMBIGUOUS_ITEM(float)
        case NexusSerializedBytecode::DOUBLE_PRECISION_FLOATING_POINT:
            PUSH_AMBIGUOUS_ITEM(double)
        case NexusSerializedBytecode::STRING:
            PUSH_AMBIGUOUS_ITEM(VString)
        case NexusSerializedBytecode::REFERENCE_COUNTED_OBJECT:
            PUSH_AMBIGUOUS_ITEM(Ref<Object>)
        case NexusSerializedBytecode::STACK_STRUCT: {
            // Aside from allocating the struct metadata, also make space for the actual data
            auto as_struct = (StackStruct*)p_ambiguous.get_raw_pointer();
            stack_objects_size += sizeof(StackStruct) + as_struct->metadata.get_struct_size();
            new (&stack_location[current_offset]) StackStruct(*as_struct);
        }
        case NexusSerializedBytecode::METHOD:
        case NexusSerializedBytecode::STRING_LITERAL:
        case NexusSerializedBytecode::NONE:
            throw MemoryStackException("This should not happen...");
    }
#undef PUSH_AMBIGUOUS_ITEM
}

void MemoryStack::pop(){
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
        case NexusSerializedBytecode::STRING: 
            object_destroy<VString>(obj_offset);
            break;
        case NexusSerializedBytecode::REFERENCE_COUNTED_OBJECT: 
            object_destroy<Ref<Object>>(obj_offset);
            break;
        case NexusSerializedBytecode::STACK_STRUCT: {
            auto as_struct = ((StackStruct*)&stack_location[obj_offset]);
//            as_struct->cleanup();
            as_struct->~StackStruct();
            stack_objects_size = obj_offset;
            break;
        }
        case NexusSerializedBytecode::STRING_LITERAL:
        case NexusSerializedBytecode::METHOD:
        case NexusSerializedBytecode::NONE:
            throw MemoryStackException(CharString("Destructor not found for type: ") + uitos(obj_type).utf8());
    }
}

void MemoryStack::set_object(const int32_t& p_relative_idx, const AmbiguousValue& p_ambiguous){
    auto item = get_at(p_relative_idx);
    auto obj_type = item.type;
    auto obj_loc = (size_t)item.data;
    if (obj_type != p_ambiguous.get_type()) throw MemoryStackException("Mismatch data type");
#define OBJECT_SET_HELPER(type, getter) set_object<type>(obj_loc, (const void*)(const_cast<AmbiguousValue&>(p_ambiguous).getter()))
    switch (obj_type) {
        case NexusSerializedBytecode::UNSIGNED_32_BIT_INTEGER:
            OBJECT_SET_HELPER(uint32_t, get_u32);
            break;
        case NexusSerializedBytecode::SIGNED_32_BIT_INTEGER:
            OBJECT_SET_HELPER(int32_t, get_i32);
            break;
        case NexusSerializedBytecode::UNSIGNED_64_BIT_INTEGER:
            OBJECT_SET_HELPER(uint64_t, get_u64);
            break;
        case NexusSerializedBytecode::SIGNED_64_BIT_INTEGER:
            OBJECT_SET_HELPER(int64_t, get_i64);
            break;
        case NexusSerializedBytecode::SINGLE_PRECISION_FLOATING_POINT:
            OBJECT_SET_HELPER(float, get_fp32);
            break;
        case NexusSerializedBytecode::DOUBLE_PRECISION_FLOATING_POINT:
            OBJECT_SET_HELPER(double, get_fp64);
            break;
        case NexusSerializedBytecode::STRING:
            OBJECT_SET_HELPER(VString, get_string);
            break;
        case NexusSerializedBytecode::REFERENCE_COUNTED_OBJECT:
        // TODO: Actually run type check
            OBJECT_SET_HELPER(Ref<Object>, get_object);
            break;
        case NexusSerializedBytecode::STACK_STRUCT:
            OBJECT_SET_HELPER(StackStruct, get_struct);
            break;
        case NexusSerializedBytecode::METHOD:
        case NexusSerializedBytecode::STRING_LITERAL:
        case NexusSerializedBytecode::NONE:
            throw MemoryStackException("This should not happen???");
    }
#undef OBJECT_SET_HELPER
}

void MemoryStack::set_object(const int32_t& p_relative_idx, const void* p_item){
    auto item = get_at(p_relative_idx);
    auto obj_type = item.type;
    auto obj_loc = (size_t)item.data;
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
        case NexusSerializedBytecode::STACK_STRUCT:
            set_object<StackStructMetadata>(obj_loc, p_item);
            break;
        case NexusSerializedBytecode::METHOD:
        case NexusSerializedBytecode::STRING_LITERAL:
        case NexusSerializedBytecode::NONE:
            throw MemoryStackException("This should not happen???");
    }
}

void MemoryStack::push(const StackStructMetadata &p_struct_metadata) {
    auto object_size = sizeof(StackStruct) + p_struct_metadata.get_struct_size();
    if (object_size + stack_objects_size > NexusRuntimeGlobalSettings::get_settings()->stack_size)
        throw MemoryStackException("Stack overflow");
    stack_objects_count++;
    stack_objects_data_type.push_back(NexusSerializedBytecode::STACK_STRUCT);
    stack_objects_offset.push_back(stack_objects_size);
    auto current_offset = stack_objects_size;
    stack_objects_size += object_size;
    new (&stack_location[current_offset]) StackStruct(p_struct_metadata);
}
