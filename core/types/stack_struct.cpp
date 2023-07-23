//
// Created by cycastic on 7/23/2023.
//

#include "stack_struct.h"
#include "vstring.h"
#include "../../language/bytecode.h"

StackStructMetadata::StackStructMetadata(const Vector<Ref<NexusBytecodeArgument>>& p_args) : struct_metadata(p_args) {
    total_struct_size = 0;
    for (const auto& arg : p_args){
        switch (arg->type) {
            case NexusSerializedBytecode::STACK_STRUCT:
                // TODO: Actually handle this
                total_struct_size += sizeof(StackStructMetadata);
                break;
            case NexusSerializedBytecode::UNSIGNED_32_BIT_INTEGER:
            case NexusSerializedBytecode::SIGNED_32_BIT_INTEGER:
                total_struct_size += sizeof(uint32_t);
                break;
            case NexusSerializedBytecode::UNSIGNED_64_BIT_INTEGER:
            case NexusSerializedBytecode::SIGNED_64_BIT_INTEGER:
                total_struct_size += sizeof(uint64_t);
                break;
            case NexusSerializedBytecode::SINGLE_PRECISION_FLOATING_POINT:
                total_struct_size += sizeof(float);
                break;
            case NexusSerializedBytecode::DOUBLE_PRECISION_FLOATING_POINT:
                total_struct_size += sizeof(double);
                break;
            case NexusSerializedBytecode::STRING:
                total_struct_size += sizeof(VString);
                break;
            case NexusSerializedBytecode::REFERENCE_COUNTED_OBJECT:
                total_struct_size += sizeof(Ref<Object>);
                break;
            case NexusSerializedBytecode::STRING_LITERAL:
            case NexusSerializedBytecode::METHOD:
                total_struct_size += sizeof(size_t);
                break;
            case NexusSerializedBytecode::NONE:
                break;
        }
    }
}
StackStructMetadata::StackStructMetadata(const StackStructMetadata& p_other) 
: struct_metadata(p_other.struct_metadata), total_struct_size(p_other.total_struct_size) {

}

StackStructMetadata& StackStructMetadata::operator=(const StackStructMetadata& p_other){
    struct_metadata = p_other.struct_metadata;
    total_struct_size = p_other.total_struct_size;
    return *this;
}