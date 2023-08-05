//
// Created by cycastic on 7/20/2023.
//

#ifndef NEXUS_Bytecode_H
#define NEXUS_Bytecode_H

#include <cstdint>
#include "../core/types/vector.h"
#include "../core/types/char_string.h"
#include "../core/exception.h"
#include "../core/io/file_access_server.h"
#include "../runtime/runtime_global_settings.h"

struct StackStructMetadata;
class TypeInfoServer;

class BytecodeException : public Exception {
public:
//    explicit BytecodeParseException(const CharString& message) : Exception(message) {}
    explicit BytecodeException(const char* message) : Exception(message) {}
    explicit BytecodeException(const CharString& message) : Exception(message) {}
};

class BytecodeParseException : public BytecodeException {
public:
//    explicit BytecodeParseException(const CharString& message) : Exception(message) {}
    explicit BytecodeParseException(const char* message) : BytecodeException(message) {}
};

struct NexusSerializedBytecode : public ManagedObject {
public:
    enum DataType : unsigned char {
        STACK_STRUCT,
        UNSIGNED_32_BIT_INTEGER,
        SIGNED_32_BIT_INTEGER,
        UNSIGNED_64_BIT_INTEGER,
        SIGNED_64_BIT_INTEGER,
        SINGLE_PRECISION_FLOATING_POINT,
        DOUBLE_PRECISION_FLOATING_POINT,
        STRING_LITERAL,
        STRING,
        REFERENCE_COUNTED_OBJECT,
        METHOD,
        NONE,
        MAX_TYPE,
    };
    enum OpCode : unsigned int {
        // Unused/deleted instruction
        OPCODE_UNUSED,
        // Load constant onto the stack1
        // Argument: a constant (of suitable type)
        // Example (in CIL):
        //     IL_0005: ldc.i4       300 // 0x0000012c
        OPCODE_LOAD_CONSTANT_I32,
        OPCODE_LOAD_CONSTANT_I64,
        OPCODE_LOAD_CONSTANT_U32,
        OPCODE_LOAD_CONSTANT_U64,
        OPCODE_LOAD_CONSTANT_FP32,
        OPCODE_LOAD_CONSTANT_FP64,
        // Copies the current_callback topmost value on the evaluation stack1, and then pushes the copy onto the evaluation stack1.
        OP_DUPLICATE,
        // Call a method
        // Argument: a string constant indicating method name
        // Stack: -1: host object
        OP_CALL,
        // Call a virtual method
        // Argument: a string constant indicating method name
        // Stack: -1: host object
        OP_CALL_VIRTUAL,
        // Load an argument onto the stack1
        // Argument: unsigned 32-bit integer, indicating argument position
        OP_LOAD_ARG,
        // Replace the value of an argument.
        // Argument: unsigned 32-bit integer, indicating argument position
        // Stack: -1: value to be replaced with
        OP_STORE_ARG,
        // Load an object from the stack1, and push it on top
        // Argument: unsigned 32-bit integer, indicating object position on the stack1
        OP_LOAD_STACK,
        // Store a value to a stack1 object
        // Argument: unsigned 32-bit integer, indicating object position on the stack1.
        // Stack: -1: value to be replaced with
        OP_STORE_STACK,
        // Load an object field onto the stack1. If said field is not static, load an object pointer first
        // Argument: 0: field number (not data_size offset)
        // Stack: (OPTIONAL) -1: host object
        OP_LOAD_FIELD,
        // Store a value into an object field
        // Argument: 0: field number (not data_size offset)
        // Stack: -2: host object; -1: value to store
        OP_STORE_FIELD,
        // Pop the topmost value of the stack1
        OP_POP,
        // Declare a label
        OP_LABEL_DECLARE,
        // Remove a label
        OP_LABEL_REMOVE,
        // Goto a label
        // Argument: 0: label's name
        OP_GOTO,
        // Goto a label if the conditional_lock is satisfied. Will trigger if stack1 No.-1 is not 0
        // Argument: 0: label's name
        // Stack: -1: unsigned 32-bit integer or equivalent
        OP_GOTO_IF_TRUE,
        // Goto a label if the conditional_lock is unsatisfied. Will trigger if stack1 No.-1 is 0
        // Argument: 0: label's name
        // Stack: -1: unsigned 32-bit integer or equivalent
        OP_GOTO_IF_FALSE,

        //--------------------------------------------------------------------- //
        //                              Operators                               //
        //--------------------------------------------------------------------- //

        // Add two value from stack1 and push the result onto the stack1. Can call overloaded method if necessary
        // Stack: -2: operand 1; -1: operand 2
        OP_ADD,
        // Subtract two value from stack1 and push the result onto the stack1. Can call overloaded method if necessary
        // Stack: -2: operand 1; -1: operand 2
        OP_SUBTRACT,
        // Multiply two value from stack1 and push the result onto the stack1. Can call overloaded method if necessary
        // Stack: -2: operand 1; -1: operand 2
        OP_MULTIPLY,
        // Divide two value from stack1 and push the result onto the stack1. Can call overloaded method if necessary
        // Stack: -2: operand 1; -1: operand 2
        OP_DIVIDE,
        // Ceiling
        OPCODE_END,
    };

    virtual void deserialize(const FilePointer& p_file) = 0;
    virtual void serialize(FilePointer& p_file) const = 0;
};

struct NexusBytecodeMetadata : public NexusSerializedBytecode{
    uint32_t magic{};
    uint32_t version{};

    NexusBytecodeMetadata() : NexusSerializedBytecode() {}
    NexusBytecodeMetadata(const uint32_t& p_magic, const uint32_t& p_version) : NexusBytecodeMetadata() {
        magic = p_magic;
        version = p_version;
    }

    void deserialize(const FilePointer& p_file) override {
        magic = p_file->get_32();
        version = p_file->get_32();
    }
    void serialize(FilePointer& p_file) const override {
        p_file->store_32(magic);
        p_file->store_32(version);
    }
};

struct NexusBytecodeMethodMetadata : public NexusSerializedBytecode {
    enum MethodAttribute : unsigned int {
        MA_NORMAL = 0,
        MA_EXTERNAL = 1,
        MA_INTRINSIC = 2,
        MA_INLINE = 4,
        MA_MAX
    };
//    uint64_t id;
    uint32_t attributes;
    VString method_name;
    // Address of method body in file
    uint64_t method_body_offset{};
private:
    mutable uint64_t offset_from_last_serialization{};
public:

    void deserialize(const FilePointer& p_file) override {
//        id = p_file->get_64();
        attributes = p_file->get_32();
        method_name = p_file->get_string();
        method_body_offset = (uint64_t)p_file->get_64();
    }
    void serialize(FilePointer& p_file) const override {
//        p_file->store_64(id);
        p_file->store_32(attributes);
        p_file->store_string(method_name);
        offset_from_last_serialization = (uint64_t)p_file->get_pos();
        p_file->store_64(0);
    }
    uint64_t get_method_offset() const { return offset_from_last_serialization; }
};

struct NexusBytecodeArgument : public NexusSerializedBytecode {
    NexusSerializedBytecode::DataType type;
    void* data{};

    template <class T>
    _FORCE_INLINE_ void auto_store(const T& p_value){
        data = new T(p_value);
    }
    template <class T>
    _FORCE_INLINE_ const T& get_data() const {
        return *(const T*)data;
    }
    _FORCE_INLINE_ explicit NexusBytecodeArgument(NexusSerializedBytecode::DataType p_type) {
        type = p_type;
        data = nullptr;
    }
    _FORCE_INLINE_ NexusBytecodeArgument() : NexusBytecodeArgument(NONE) {}
    NexusBytecodeArgument(StackStructMetadata* p_struct_metadata) {
        type = STACK_STRUCT;
        data = p_struct_metadata;
    }
    _FORCE_INLINE_ explicit NexusBytecodeArgument(const uint32_t& p_value){
        type = UNSIGNED_32_BIT_INTEGER;
        auto_store(p_value);
    }
    _FORCE_INLINE_ explicit NexusBytecodeArgument(const int32_t& p_value){
        type = SIGNED_32_BIT_INTEGER;
        auto_store(p_value);
    }
    _FORCE_INLINE_ explicit NexusBytecodeArgument(const uint64_t& p_value){
        type = UNSIGNED_64_BIT_INTEGER;
        auto_store(p_value);
    }
    _FORCE_INLINE_ explicit NexusBytecodeArgument(const int64_t& p_value){
        type = SIGNED_64_BIT_INTEGER;
        auto_store(p_value);
    }
    _FORCE_INLINE_ explicit NexusBytecodeArgument(const float& p_value){
        type = SINGLE_PRECISION_FLOATING_POINT;
        auto_store(p_value);
    }
    _FORCE_INLINE_ explicit NexusBytecodeArgument(const double& p_value){
        type = DOUBLE_PRECISION_FLOATING_POINT;
        auto_store(p_value);
    }
    _FORCE_INLINE_ explicit NexusBytecodeArgument(const VString& p_value){
        type = STRING_LITERAL;
        auto_store(p_value);
    }
    ~NexusBytecodeArgument() override;
    void deserialize(const FilePointer& p_file) override {
        throw BytecodeParseException("NexusBytecodeArgument cannot deserialize by itself");
    }
    void serialize(FilePointer& p_file) const override {
        throw BytecodeParseException("NexusBytecodeArgument cannot serialize by itself");
    }
};
struct NexusBytecodeRawInstruction : public NexusSerializedBytecode {
public:
public:
    NexusSerializedBytecode::OpCode opcode{};
    Vector<Ref<NexusBytecodeArgument>> arguments{};

    void deserialize(const FilePointer& p_file) override;
    void serialize(FilePointer& p_file) const override;
};

struct NexusBytecodeMethodBody : public NexusSerializedBytecode {
private:
public:
    // VString instead of CharString for greater compatibility
    VString method_name{};
    Vector<Ref<NexusBytecodeArgument>> arguments{};
    uint16_t max_stack{};
    Vector<Ref<NexusBytecodeArgument>> locals_init{};
    Vector<Ref<NexusBytecodeRawInstruction>> instructions{};

    void deserialize(const FilePointer& p_file) override;
    void serialize(FilePointer& p_file) const override;

    void read_header(const FilePointer& p_file);
    static Ref<NexusBytecodeRawInstruction> read_next_instruction(const FilePointer& p_file);
};

class NexusBytecode : public NexusSerializedBytecode {
public:
    // NEX
    static constexpr uint32_t MAGIC = 0x6E6578;
    static constexpr uint32_t VERSION = 0x000100;
    static constexpr uint32_t INSTRUCTIONS_RESERVED = 32; // 256 bit
private:
    // Constants
//    Vector<uint32_t> u32_constants{};
//    Vector<uint64_t> u64_constants{};
//    Vector<float> f32_constants{};
//    Vector<double> f64_constants{};
//    Vector<VString> string_literals{};
    // Method metadata_collection
    Vector<Ref<NexusBytecodeMethodMetadata>> methods_metadata{};
    Vector<Ref<NexusBytecodeMethodBody>> method_bodies{};
    HashMap<VString, Ref<NexusBytecodeMethodMetadata>> metadata_map{};
    HashMap<VString, Ref<NexusBytecodeMethodBody>> bodies_map{};
public:
    void deserialize(const FilePointer& p_file) override;
    void serialize(FilePointer& p_file) const override;
    void load_header(const FilePointer& p_file);
    void load_methods_body(const FilePointer& p_file);
    _FORCE_INLINE_ const Vector<Ref<NexusBytecodeMethodMetadata>>& get_methods_metadata() const { return methods_metadata; }
    _FORCE_INLINE_ const Vector<Ref<NexusBytecodeMethodBody>>& get_method_bodies() const { return method_bodies; }
    _FORCE_INLINE_ const HashMap<VString, Ref<NexusBytecodeMethodMetadata>>& get_metadata_map() const { return metadata_map; }
    _FORCE_INLINE_ const HashMap<VString, Ref<NexusBytecodeMethodBody>>& get_bodies_map() const { return bodies_map; }

private:
    _FORCE_INLINE_ void parse_from_file(const FilePointer& p_file) { deserialize(p_file); }
public:
    _FORCE_INLINE_ void build_metadata_map(){
        metadata_map.clear();
        for (const auto& it : methods_metadata){
            metadata_map[it->method_name] = it;
        }
    }
    _FORCE_INLINE_ void build_bodies_map(){
        bodies_map.clear();
        for (const auto& it : method_bodies){
            bodies_map[it->method_name] = it;
        }
    }
    NexusBytecode() : NexusSerializedBytecode() {}
    explicit NexusBytecode(const FilePointer& p_from) { parse_from_file(p_from); }

    _FORCE_INLINE_ void clear(){
        methods_metadata.clear();
        method_bodies.clear();
    }
};

struct NexusBytecodeInstance;

struct NexusMethodPointer : public ManagedObject {
private:
    const TypeInfoServer* type_info_server;
public:
    explicit NexusMethodPointer(const TypeInfoServer* p_type_info_server);

    virtual int64_t get_iterator() const = 0;
    virtual void move_iterator(const int64_t& p_new_pos) = 0;
    virtual Ref<NexusBytecodeRawInstruction> get_next_instruction() = 0;
    virtual Ref<NexusBytecodeMethodMetadata> get_method_metadata() const = 0;
    virtual Vector<Ref<NexusBytecodeArgument>> get_arguments() const = 0;
    
    virtual void load_method(const Ref<NexusBytecodeInstance>& p_bci, const VString& p_method_name) = 0;

    const TypeInfoServer* get_type_info_server() const;
};

struct NexusMethodPointerJIT : public NexusMethodPointer {
private:
    FilePointer file{};
    Ref<NexusBytecodeMethodBody> method_body{};
    Ref<NexusBytecodeMethodMetadata> method_metadata{};
    Vector<size_t> offsets{};
    int64_t instructions_iter = -1;
public:
    explicit NexusMethodPointerJIT(const TypeInfoServer* p_type_info_server) : NexusMethodPointer(p_type_info_server) {}

    int64_t get_iterator() const override { return instructions_iter; }
    void move_iterator(const int64_t& p_new_pos) override;
    Ref<NexusBytecodeRawInstruction> get_next_instruction() override;
    Ref<NexusBytecodeMethodMetadata> get_method_metadata() const override;
    Vector<Ref<NexusBytecodeArgument>> get_arguments() const override;
    
    void load_method(const Ref<NexusBytecodeInstance>& p_bci, const VString& p_method_name) override;
};

struct NexusMethodPointerMemory : public NexusMethodPointer {
private:
    Ref<NexusBytecodeMethodBody> method_body{};
    Ref<NexusBytecodeMethodMetadata> method_metadata{};
    int64_t instructions_iter = -1;
public:
    explicit NexusMethodPointerMemory(const TypeInfoServer* p_type_info_server) : NexusMethodPointer(p_type_info_server) {}

    int64_t get_iterator() const override { return instructions_iter; }
    void move_iterator(const int64_t& p_new_pos) override { instructions_iter = p_new_pos; }
    Ref<NexusBytecodeRawInstruction> get_next_instruction() override;
    Ref<NexusBytecodeMethodMetadata> get_method_metadata() const override;
    Vector<Ref<NexusBytecodeArgument>> get_arguments() const override;

    void load_method(const Ref<NexusBytecodeInstance>& p_bci, const VString& p_method_name) override;
};

struct NexusBytecodeInstance : public ManagedObject {
public:
    enum BytecodeLoadMode : unsigned int {
        LOAD_HEADER,
        LOAD_ALL,
    };
private:
    mutable BinaryMutex lock{};
    const TypeInfoServer* type_info_server;

    explicit NexusBytecodeInstance(const TypeInfoServer* p_type_info_server) : type_info_server(p_type_info_server) {}
public:
    BytecodeLoadMode load_mode;
    FilePointer file_pointer;
    Ref<NexusBytecode> bytecode;
    HashMap<VString, size_t> bodies_location{};
    uint64_t header_end{};
    NexusBytecodeInstance(const TypeInfoServer* p_type_info_server, const FilePointer& p_fp, BytecodeLoadMode p_load_mode);
    NexusBytecodeInstance(const TypeInfoServer* p_type_info_server, const VString& p_bc_path, BytecodeLoadMode p_load_mode);

    _FORCE_INLINE_ Ref<NexusMethodPointer> get_method(const VString& p_method_name) const {
        GUARD(lock);
        if (!bytecode->get_metadata_map().has(p_method_name)) throw BytecodeException("Method not found");
        Ref<NexusMethodPointer> ptr = Ref<NexusMethodPointer>::null();
        switch (load_mode){
            case LOAD_HEADER:
                ptr = Ref<NexusMethodPointerJIT>::make_ref(type_info_server).safe_cast<NexusMethodPointer>();
                break;
            case LOAD_ALL:
                ptr = Ref<NexusMethodPointerMemory>::make_ref(type_info_server).safe_cast<NexusMethodPointer>();
                break;
            default:
                throw BytecodeException("Load mode not supported");
        }
        auto ref_self = Ref<NexusBytecodeInstance>::from_initialized_object(const_cast<NexusBytecodeInstance *>(this));
        ptr->load_method(ref_self, p_method_name);
        return ptr;
    }
};

#endif //NEXUS_Bytecode_H
