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
                total_struct_size += sizeof(Ref<ManagedObject>);
                break;
            case NexusSerializedBytecode::STACK_STRUCT:
            case NexusSerializedBytecode::STRING_LITERAL:
            case NexusSerializedBytecode::METHOD:
                throw Exception();
            case NexusSerializedBytecode::NONE:
                break;
        }
    }
}
StackStructMetadata::StackStructMetadata(const StackStructMetadata& p_other) 
: struct_metadata(p_other.struct_metadata), total_struct_size(p_other.total_struct_size) {

}

StackStruct::StackStruct(const Vector<Ref<NexusBytecodeArgument>> &p_args) : metadata(p_args) {

}

StackStruct::StackStruct(const StackStruct &p_other) : metadata(p_other.metadata) {
    size_t offset = 0;
#define PLACEMENT_NEW(type) new ((type*)((size_t)get_data() + offset)) type(*(type*)((size_t)p_other.get_data() + offset)); offset += sizeof(type)
    for (const auto& item : metadata.struct_metadata){
        switch (item->type){
            case NexusSerializedBytecode::UNSIGNED_32_BIT_INTEGER:
                PLACEMENT_NEW(uint32_t);
                break;
            case NexusSerializedBytecode::SIGNED_32_BIT_INTEGER:
                PLACEMENT_NEW(int32_t);
                break;
            case NexusSerializedBytecode::UNSIGNED_64_BIT_INTEGER:
                PLACEMENT_NEW(uint64_t);
                break;
            case NexusSerializedBytecode::SIGNED_64_BIT_INTEGER:
                PLACEMENT_NEW(int64_t);
                break;
            case NexusSerializedBytecode::SINGLE_PRECISION_FLOATING_POINT:
                PLACEMENT_NEW(float);
                break;
            case NexusSerializedBytecode::DOUBLE_PRECISION_FLOATING_POINT:
                PLACEMENT_NEW(double);
                break;
            case NexusSerializedBytecode::STRING:
                PLACEMENT_NEW(VString);
                break;
            case NexusSerializedBytecode::REFERENCE_COUNTED_OBJECT:
                PLACEMENT_NEW(Ref<ManagedObject>);
                break;
            case NexusSerializedBytecode::STRING_LITERAL:
            case NexusSerializedBytecode::STACK_STRUCT:
            case NexusSerializedBytecode::METHOD:
                throw Exception();
            case NexusSerializedBytecode::NONE:
                break;
        }
    }
#undef PLACEMENT_NEW
}

// WARNING: USE FOR COPY VALUES ONLY!!!
StackStruct &StackStruct::operator=(const StackStruct &p_other) {
#define COPY_VALUE(type) *(type*)((size_t)get_data() + offset) = *(type*)((size_t)p_other.get_data() + offset); offset += sizeof(type)
    size_t offset = 0;
    for (const auto& item : metadata.struct_metadata){
        switch (item->type){
            case NexusSerializedBytecode::UNSIGNED_32_BIT_INTEGER:
            COPY_VALUE(uint32_t);
                break;
            case NexusSerializedBytecode::SIGNED_32_BIT_INTEGER:
            COPY_VALUE(int32_t);
                break;
            case NexusSerializedBytecode::UNSIGNED_64_BIT_INTEGER:
            COPY_VALUE(uint64_t);
                break;
            case NexusSerializedBytecode::SIGNED_64_BIT_INTEGER:
            COPY_VALUE(int64_t);
                break;
            case NexusSerializedBytecode::SINGLE_PRECISION_FLOATING_POINT:
            COPY_VALUE(float);
                break;
            case NexusSerializedBytecode::DOUBLE_PRECISION_FLOATING_POINT:
            COPY_VALUE(double);
                break;
            case NexusSerializedBytecode::STRING:
            COPY_VALUE(VString);
                break;
            case NexusSerializedBytecode::REFERENCE_COUNTED_OBJECT:
            COPY_VALUE(Ref<ManagedObject>);
                break;
            case NexusSerializedBytecode::STRING_LITERAL:
            case NexusSerializedBytecode::STACK_STRUCT:
            case NexusSerializedBytecode::METHOD:
                throw Exception();
            case NexusSerializedBytecode::NONE:
                break;
        }
    }
#undef COPY_VALUE
    return *this;
}

void *StackStruct::get_data() const {
    return (void*)(this + sizeof(StackStructMetadata));
}

void StackStruct::cleanup() {
#define RAISE_OFFSET(type) offset += sizeof(type)
#define FREE_OBJECT(type) ((type*)((size_t)get_data() + offset))->~type(); RAISE_OFFSET(type)
    size_t offset = 0;
    for (const auto& item : metadata.struct_metadata){
        switch (item->type){
            case NexusSerializedBytecode::UNSIGNED_32_BIT_INTEGER:
                RAISE_OFFSET(uint32_t);
                break;
            case NexusSerializedBytecode::SIGNED_32_BIT_INTEGER:
                RAISE_OFFSET(int32_t);
                break;
            case NexusSerializedBytecode::UNSIGNED_64_BIT_INTEGER:
                RAISE_OFFSET(uint64_t);
                break;
            case NexusSerializedBytecode::SIGNED_64_BIT_INTEGER:
                RAISE_OFFSET(int64_t);
                break;
            case NexusSerializedBytecode::SINGLE_PRECISION_FLOATING_POINT:
                RAISE_OFFSET(float);
                break;
            case NexusSerializedBytecode::DOUBLE_PRECISION_FLOATING_POINT:
                RAISE_OFFSET(double);
                break;
            case NexusSerializedBytecode::STRING:
                FREE_OBJECT(VString);
                break;
            case NexusSerializedBytecode::REFERENCE_COUNTED_OBJECT:
                FREE_OBJECT(Ref<ManagedObject>);
                break;
            case NexusSerializedBytecode::STRING_LITERAL:
            case NexusSerializedBytecode::STACK_STRUCT:
            case NexusSerializedBytecode::METHOD:
                throw Exception();
            case NexusSerializedBytecode::NONE:
                break;
        }
    }
#undef  FREE_OBJECT
#undef  RAISE_OFFSET
}

StackStruct::~StackStruct() {
    cleanup();
}

StackStruct::StackStruct(const StackStructMetadata &p_metadata) : metadata(p_metadata) {

}
