#include "ambigous_value.h"
#include "../core/types/stack_struct.h"

void AmbiguousValue::clear_and_set_from(const void* p_data, NexusSerializedBytecode::DataType p_data_type){
    if (data) free_data();
#define AUTO_CAST(type) data = new type(*(type*)p_data)
    switch (p_data_type){
        case NexusSerializedBytecode::UNSIGNED_32_BIT_INTEGER:
            AUTO_CAST(uint32_t);
            break;
        case NexusSerializedBytecode::SIGNED_32_BIT_INTEGER:
            AUTO_CAST(int32_t);
            break;
        case NexusSerializedBytecode::UNSIGNED_64_BIT_INTEGER:
            AUTO_CAST(uint64_t);
            break;
        case NexusSerializedBytecode::SIGNED_64_BIT_INTEGER:
            AUTO_CAST(int64_t);
            break;
        case NexusSerializedBytecode::SINGLE_PRECISION_FLOATING_POINT:
            AUTO_CAST(float);
            break;
        case NexusSerializedBytecode::DOUBLE_PRECISION_FLOATING_POINT:
            AUTO_CAST(double);
            break;
        case NexusSerializedBytecode::STRING:
            AUTO_CAST(VString);
            break;
        case NexusSerializedBytecode::REFERENCE_COUNTED_OBJECT:
            AUTO_CAST(Ref<Object>);
            break;
        case NexusSerializedBytecode::STACK_STRUCT: {
            auto other_as_metadata = (StackStruct*)p_data;
            data = malloc(sizeof(StackStruct) + other_as_metadata->metadata.get_struct_size());
            new(data) StackStruct(*other_as_metadata);
            break;
        }
        case NexusSerializedBytecode::STRING_LITERAL:
        case NexusSerializedBytecode::METHOD:
        case NexusSerializedBytecode::NONE:
            throw Exception("This should not happen...");
    }
#undef AUTO_CAST
    data_type = p_data_type;
}

void AmbiguousValue::free_data() {
    switch (data_type){
        case NexusSerializedBytecode::NONE:
            return;
        case NexusSerializedBytecode::UNSIGNED_32_BIT_INTEGER:
            delete (uint32_t*)data;
            break;
        case NexusSerializedBytecode::SIGNED_32_BIT_INTEGER:
            delete (int32_t*)data;
            break;
        case NexusSerializedBytecode::UNSIGNED_64_BIT_INTEGER:
            delete (uint64_t*)data;
            break;
        case NexusSerializedBytecode::SIGNED_64_BIT_INTEGER:
            delete (int64_t*)data;
            break;
        case NexusSerializedBytecode::SINGLE_PRECISION_FLOATING_POINT:
            delete (float*)data;
            break;
        case NexusSerializedBytecode::DOUBLE_PRECISION_FLOATING_POINT:
            delete (double*)data;
            break;
        case NexusSerializedBytecode::STRING:
            delete (VString*)data;
            break;
        case NexusSerializedBytecode::REFERENCE_COUNTED_OBJECT:
            delete (Ref<Object>*)data;
            break;
        case NexusSerializedBytecode::STACK_STRUCT:
//            ((StackStructMetadata*)data)->cleanup();
            ((StackStruct*)data)->~StackStruct();
            free(data);
            break;
        case NexusSerializedBytecode::STRING_LITERAL:
        case NexusSerializedBytecode::METHOD:
            throw Exception("This should not happen...");
    }
    data = nullptr;
    data_type = NexusSerializedBytecode::NONE;
}

StackStruct *&AmbiguousValue::get_struct() { return get_internal<StackStruct>(NexusSerializedBytecode::STACK_STRUCT); }
