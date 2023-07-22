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

class BytecodeParseException : public Exception {
public:
//    explicit BytecodeParseException(const CharString& message) : Exception(message) {}
    explicit BytecodeParseException(const char* message) : Exception(message) {}
};

struct NexusSerializedBytecode : public Object {
public:
    enum DataType : unsigned int {
        VOID,
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
    };
    enum OpCode : unsigned short {
        // Unused/deleted instruction
        OPCODE_UNUSED,
        // Load constant onto the stack
        // Argument: a constant (of suitable type)
        // Example (in CIL):
        //     IL_0005: ldc.i4       300 // 0x0000012c
        OPCODE_LOAD_CONSTANT_I32,
        OPCODE_LOAD_CONSTANT_I64,
        OPCODE_LOAD_CONSTANT_U32,
        OPCODE_LOAD_CONSTANT_U64,
        OPCODE_LOAD_CONSTANT_FP32,
        OPCODE_LOAD_CONSTANT_FP64,
        // Copies the current topmost value on the evaluation stack, and then pushes the copy onto the evaluation stack.
        OP_DUPLICATE,
        // Call a method
        // Argument: a string constant indicating method name
        // Stack: -1: host object
        OP_CALL,
        // Call a virtual method
        // Argument: a string constant indicating method name
        // Stack: -1: host object
        OP_CALL_VIRTUAL,
        // Load an argument onto the stack
        // Argument: unsigned 32-bit integer, indicating argument position
        OP_LOAD_ARG,
        // Replace the value of an argument.
        // Argument: unsigned 32-bit integer, indicating argument position
        // Stack: -1: value to be replaced with
        OP_STORE_ARG,
        // Load an object from the stack, and push it on top
        // Argument: unsigned 32-bit integer, indicating object position on the stack
        OP_LOAD_STACK,
        // Store a value to a stack object
        // Argument: unsigned 32-bit integer, indicating object position on the stack.
        // Stack: -1: value to be replaced with
        OP_STORE_STACK,
        // Load an object field onto the stack. If said field is not static, load an object pointer first
        // Argument: 0: field number (not data_size offset)
        // Stack: (OPTIONAL) -1: host object
        OP_LOAD_FIELD,
        // Store a value into an object field
        // Argument: 0: field number (not data_size offset)
        // Stack: -2: host object; -1: value to store
        OP_STORE_FIELD,
        // Pop the topmost value of the stack
        OP_POP,
        // Declare a label
        OP_LABEL_DECLARE,
        // Remove a label
        OP_LABEL_REMOVE,
        // Goto a label
        // Argument: 0: label's name
        OP_GOTO,
        // Goto a label if the condition is satisfied. Will trigger if stack No.-1 is not 0
        // Argument: 0: label's name
        // Stack: -1: unsigned 32-bit integer or equivalent
        OP_GOTO_IF_TRUE,
        // Goto a label if the condition is unsatisfied. Will trigger if stack No.-1 is 0
        // Argument: 0: label's name
        // Stack: -1: unsigned 32-bit integer or equivalent
        OP_GOTO_IF_FALSE,

        //--------------------------------------------------------------------- //
        //                              Operators                               //
        //--------------------------------------------------------------------- //

        // Add two value from stack and push the result onto the stack. Can call overloaded method if necessary
        // Stack: -2: operand 1; -1: operand 2
        OP_ADD,
        // Subtract two value from stack and push the result onto the stack. Can call overloaded method if necessary
        // Stack: -2: operand 1; -1: operand 2
        OP_SUBTRACT,
        // Multiply two value from stack and push the result onto the stack. Can call overloaded method if necessary
        // Stack: -2: operand 1; -1: operand 2
        OP_MULTIPLY,
        // Divide two value from stack and push the result onto the stack. Can call overloaded method if necessary
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

struct NexusMethodMetadata : public NexusSerializedBytecode {
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
    _FORCE_INLINE_ NexusBytecodeArgument() : NexusBytecodeArgument(VOID) {}
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
    ~NexusBytecodeArgument() override {
        switch (type) {
            case UNSIGNED_32_BIT_INTEGER: memdelete((uint32_t*)data);
            case SIGNED_32_BIT_INTEGER: memdelete((int32_t*)data);
            case UNSIGNED_64_BIT_INTEGER: memdelete((uint64_t*)data);
            case SIGNED_64_BIT_INTEGER: memdelete((int64_t*)data);
            case SINGLE_PRECISION_FLOATING_POINT: memdelete((float*)data);
            case DOUBLE_PRECISION_FLOATING_POINT: memdelete((double*)data);
            case STRING_LITERAL: memdelete((VString*)data);
            // Not supported / Should be deleted as address
            case VOID:
            case STRING:
            case REFERENCE_COUNTED_OBJECT:
            case METHOD:
                if (data) memdelete((size_t*)data);
                break;
            case NONE:
                break;
        }
    }
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
    // VString instead of CharString for greater compatibility
    VString method_name{};
    Vector<Ref<NexusBytecodeArgument>> arguments{};
    uint16_t max_stack{};
    Vector<Ref<NexusBytecodeArgument>> locals_init{};
    Vector<Ref<NexusBytecodeRawInstruction>> instructions{};

    void deserialize(const FilePointer& p_file) override;
    void serialize(FilePointer& p_file) const override;
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
    // Method metadata
    Vector<Ref<NexusMethodMetadata>> methods_metadata{};
    Vector<Ref<NexusBytecodeMethodBody>> method_bodies{};

public:
    void deserialize(const FilePointer& p_file) override;
    void serialize(FilePointer& p_file) const override;
    void load_header(const FilePointer& p_file);
    void load_methods_body(const FilePointer& p_file);
    _FORCE_INLINE_ const Vector<Ref<NexusMethodMetadata>>& get_methods_metadata() const { return methods_metadata; }
    _FORCE_INLINE_ const Vector<Ref<NexusBytecodeMethodBody>>& get_method_bodies() const { return method_bodies; }

private:
    _FORCE_INLINE_ void parse_from_file(const FilePointer& p_file) { deserialize(p_file); }
public:
    NexusBytecode() : NexusSerializedBytecode() {}
    explicit NexusBytecode(const FilePointer& p_from) { parse_from_file(p_from); }

    _FORCE_INLINE_ void clear(){
        methods_metadata.clear();
        method_bodies.clear();
    }
};

#endif //NEXUS_Bytecode_H
