//
// Created by cycastic on 7/22/2023.
//

#include "runtime.h"
#include "nexus_stack.h"
#include "../core/types/linked_list.h"

NexusRuntime::NexusRuntime() : ManagedObject(), type_info_server(new NexusTypeInfoServer(true)) {
    anonymous_instances_count.set(0);
}

void NexusRuntime::load_bytecode(const VString &p_bytecode_path, NexusBytecodeInstance::BytecodeLoadMode p_load_mode) {
    W_GUARD(rwlock);
    Ref<NexusBytecodeInstance> instance = Ref<NexusBytecodeInstance>::make_ref(type_info_server, p_bytecode_path, p_load_mode);
    bytecode_instances[p_bytecode_path] = instance;
    cache_method_bodies(instance);
}

void NexusRuntime::load_bytecode(FilePointer &p_file_pointer, NexusBytecodeInstance::BytecodeLoadMode p_load_mode) {
    W_GUARD(rwlock);
    Ref<NexusBytecodeInstance> instance = Ref<NexusBytecodeInstance>::make_ref(type_info_server, p_file_pointer, p_load_mode);
    bytecode_instances[VString("@AnonymousInstance:") + uitos(anonymous_instances_count.increment())] = instance;
    cache_method_bodies(instance);
}

void NexusRuntime::cache_method_bodies(const Ref<NexusBytecodeInstance>& instance) {
    auto it = instance->bodies_location.const_iterator();
    LinkedList<VString> loaded_methods{};
    try {
        while (it.move_next()){
            const auto& method_name = it.get_pair().key;
            if (bytecode_method_bodies.has(method_name)) throw RuntimeException(CharString("Method already has: ") + method_name.utf8());
            bytecode_method_bodies[method_name] = instance;
        }
    } catch (const std::exception& ex){
        // Rollback changes before rethrowing
        for (const auto* method_name = loaded_methods.first(); method_name; method_name = method_name->next()){
            bytecode_method_bodies.erase(method_name->data);
        }
        throw ex;
    }
}

NexusRuntime::~NexusRuntime() = default;
