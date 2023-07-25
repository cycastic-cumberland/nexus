//
// Created by cycastic on 7/23/2023.
//

#ifndef NEXUS_STACK_STRUCT_H
#define NEXUS_STACK_STRUCT_H

#include "vector.h"
#include "reference.h"

struct NexusBytecodeArgument;

struct StackStructMetadata {
private:
    size_t total_struct_size;
public:
    Vector<Ref<NexusBytecodeArgument>> struct_metadata;
    explicit StackStructMetadata(const Vector<Ref<NexusBytecodeArgument>>& p_args);
    StackStructMetadata(const StackStructMetadata& p_other);

    _NO_DISCARD_ _FORCE_INLINE_ size_t get_struct_size() const { return total_struct_size;}
};

struct StackStruct {
    StackStructMetadata metadata;

    explicit StackStruct(const Vector<Ref<NexusBytecodeArgument>>& p_args);
    explicit StackStruct(const StackStructMetadata& p_metadata);
    StackStruct(const StackStruct& p_other);
    StackStruct& operator=(const StackStruct& p_other);

    _NO_DISCARD_ void * get_data() const;
    void cleanup();
    ~StackStruct();
};

#endif //NEXUS_STACK_STRUCT_H
