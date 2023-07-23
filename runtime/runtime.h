//
// Created by cycastic on 7/22/2023.
//

#ifndef NEXUS_RUNTIME_H
#define NEXUS_RUNTIME_H
#include "../language/bytecode.h"
#include "system.h"

class RuntimeException : Exception {
public:
    explicit RuntimeException(const char* p_msg) : Exception(p_msg) {}
    explicit RuntimeException(const CharString& p_msg) : Exception(p_msg) {}
};

class NexusRuntime : public Object {
public:
private:
    HashMap<VString, Ref<NexusBytecodeInstance>> bytecode_instances{};
    HashMap<VString, Ref<NexusBytecodeInstance>> bytecode_method_bodies{};
    RWLock rwlock{};
    SafeNumeric<uint32_t> anonymous_instances_count{};

    void cache_method_bodies(const Ref<NexusBytecodeInstance>& instance);
public:
    void load_bytecode(const VString& p_bytecode_path, NexusBytecodeInstance::BytecodeLoadMode p_load_mode);
    void load_bytecode(FilePointer& p_file_pointer, NexusBytecodeInstance::BytecodeLoadMode p_load_mode);

    NexusRuntime();
    ~NexusRuntime() override;
};
#endif //NEXUS_RUNTIME_H
