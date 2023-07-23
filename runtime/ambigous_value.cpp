#include "ambigous_value.h"
#include "../core/types/stack_struct.h"

void AmbiguousValue::set_from(const void* p_data, NexusSerializedBytecode::DataType p_data_type){
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
        case NexusSerializedBytecode::STACK_STRUCT:
            AUTO_CAST(StackStructMetadata);
            break;
        case NexusSerializedBytecode::STRING_LITERAL:
        case NexusSerializedBytecode::METHOD:
        case NexusSerializedBytecode::NONE:
            throw Exception("This should not happen...");
    }
#undef AUTO_CAST
    data_type = p_data_type;
}
