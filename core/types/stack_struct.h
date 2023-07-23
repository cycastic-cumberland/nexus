//
// Created by cycastic on 7/23/2023.
//

#ifndef NEXUS_STACK_STRUCT_H
#define NEXUS_STACK_STRUCT_H

#include "vector.h"
#include "reference.h"

struct NexusBytecodeArgument;

struct StackStructMetadata {
    Vector<Ref<NexusBytecodeArgument>> struct_metadata;
    size_t total_struct_size;
    explicit StackStructMetadata(const Vector<Ref<NexusBytecodeArgument>>& p_args);
    StackStructMetadata(const StackStructMetadata& p_other);
    StackStructMetadata& operator=(const StackStructMetadata& p_other);
};

// A struct that live on the memory stack
struct StackStruct {
    StackStructMetadata metadata;

    _NO_DISCARD_ _FORCE_INLINE_ const StackStructMetadata& get_metadata() const { return metadata; }
    _NO_DISCARD_ _FORCE_INLINE_ size_t get_struct_size() const { return metadata.total_struct_size; }
    _NO_DISCARD_ _FORCE_INLINE_ const uint8_t* get_data() const { return (const uint8_t*)(this + sizeof(StackStructMetadata)); }
    _NO_DISCARD_ _FORCE_INLINE_ uint8_t* get_data() { return (uint8_t*)(this + sizeof(StackStructMetadata)); }
};

#endif //NEXUS_STACK_STRUCT_H
