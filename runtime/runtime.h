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
    static constexpr bool endian_mode = false;
    enum BytecodeLoadMode : unsigned int {
        LOAD_HEADER,
        LOAD_ALL,
    };
    struct NexusBytecodeInstance : public Object {
        BytecodeLoadMode load_mode;
        FilePointer file_pointer;
        Ref<NexusBytecode> bytecode;
        HashMap<VString, uint64_t> bodies_location{};
        uint64_t header_end{};
        NexusBytecodeInstance(FilePointer& p_fp, BytecodeLoadMode p_load_mode){
            load_mode = p_load_mode;
            bytecode = Ref<NexusBytecode>::init();
            file_pointer = p_fp;
            switch (load_mode) {
                case LOAD_HEADER:
                case LOAD_ALL:
                    bytecode->load_header(file_pointer);
                    header_end = (uint64_t)file_pointer->get_pos();
                    if (load_mode == LOAD_HEADER) break;
                    bytecode->load_methods_body(file_pointer);
            }
            // Get body offset to jump around file faster
            // Method must not be external
            for (const auto& method_metadata : bytecode->get_methods_metadata()){
                if (!(method_metadata->attributes | NexusMethodMetadata::MA_EXTERNAL) && method_metadata->method_body_offset != 0)
                    bodies_location[method_metadata->method_name] = method_metadata->method_body_offset;
            }
        }
        NexusBytecodeInstance(const VString& p_bc_path, BytecodeLoadMode p_load_mode){
            load_mode = p_load_mode;
            bytecode = Ref<NexusBytecode>::init();
            file_pointer = FileAccessServer::open(p_bc_path, FileAccess::ACCESS_READ, endian_mode);
            if (!file_pointer->is_open()) throw RuntimeException(CharString("Failed to open bytecode: ") + p_bc_path.utf8());
            switch (load_mode) {
                case LOAD_HEADER:
                case LOAD_ALL:
                    bytecode->load_header(file_pointer);
                    header_end = (uint64_t)file_pointer->get_pos();
                    if (load_mode == LOAD_HEADER) break;
                    bytecode->load_methods_body(file_pointer);
            }
            // Get body offset to jump around file faster
            // Method must not be external
            for (const auto& method_metadata : bytecode->get_methods_metadata()){
                if (!(method_metadata->attributes | NexusMethodMetadata::MA_EXTERNAL) && method_metadata->method_body_offset != 0)
                    bodies_location[method_metadata->method_name] = method_metadata->method_body_offset;
            }
        }
    };
private:
    HashMap<VString, Ref<NexusBytecodeInstance>> bytecode_instances{};
    HashMap<VString, Ref<NexusBytecodeInstance>> bytecode_method_bodies{};
    RWLock rwlock{};
    SafeNumeric<uint32_t> anonymous_instances_count{};

    void cache_method_bodies(const Ref<NexusBytecodeInstance>& instance);
public:
    void load_bytecode(const VString& p_bytecode_path, BytecodeLoadMode p_load_mode);
    void load_bytecode(FilePointer& p_file_pointer, BytecodeLoadMode p_load_mode);

    NexusRuntime();
    ~NexusRuntime() override;
};
#endif //NEXUS_RUNTIME_H
