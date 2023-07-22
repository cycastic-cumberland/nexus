//
// Created by cycastic on 7/20/2023.
//

#include "Bytecode.h"
#include "../core/types/stack.h"

void NexusBytecode::load_header(const FilePointer &p_file) {
#define COLLECT_CONSTANTS(collector, storage) {         \
    auto size = p_file->get_32();                       \
    for (size_t i = 0; i < size; i++){                  \
        storage.push_back(p_file->collector());         \
    }                                                   \
}
    Ref<NexusBytecodeMetadata> bytecode_metadata = Ref<NexusBytecodeMetadata>::make_ref();
    bytecode_metadata->deserialize(p_file);
    if (bytecode_metadata->magic != MAGIC) throw BytecodeParseException("Incorrect magic sequence");
    if (bytecode_metadata->version != VERSION) throw BytecodeParseException("Version not supported");
    bytecode_metadata.unref();
//    COLLECT_CONSTANTS(get_32, u32_constants)
//    COLLECT_CONSTANTS(get_64, u64_constants)
//    COLLECT_CONSTANTS(get_float, f32_constants)
//    COLLECT_CONSTANTS(get_double, f64_constants)
//    COLLECT_CONSTANTS(get_string, string_literals)
    // Read method metadata
    auto method_count = p_file->get_32();
    methods_metadata = Vector<Ref<NexusMethodMetadata>>(method_count);
    for (int i = 0; i < method_count; i++){
        methods_metadata.push_back(Ref<NexusMethodMetadata>::init());
        methods_metadata[i]->deserialize(p_file);
    }
    // Reserved space
    for (int i = 0; i < INSTRUCTIONS_RESERVED; i++)
        p_file->get_8();
#undef COLLECT_CONSTANTS
}

void NexusBytecode::load_methods_body(const FilePointer &p_file) {
    // Method body
    //
    // That's sus...
    auto body_count = p_file->get_16();
    method_bodies = Vector<Ref<NexusBytecodeMethodBody>>(body_count);
    for (uint16_t i = 0; i < body_count; i++){
        Ref<NexusBytecodeMethodBody> body = Ref<NexusBytecodeMethodBody>::make_ref();
        body->deserialize(p_file);
        method_bodies.push_back(body);
    }
}

void NexusBytecode::deserialize(const FilePointer& p_file) {
    load_header(p_file);
    load_methods_body(p_file);
}

void NexusBytecode::serialize(FilePointer &p_file) const {
#define STORE_CONSTANTS(dispenser, storage) {           \
    auto size = storage.size();                         \
    p_file->store_32(size);                             \
    for (const auto& val : storage){                    \
        p_file->dispenser(val);                         \
    }                                                   \
}
    // Cache
    HashMap<VString, uint64_t> method_metadata_offset_loc{};
    HashMap<VString, uint64_t> method_body_offset_loc{};
    // Store header
    Ref<NexusBytecodeMetadata> header = Ref<NexusBytecodeMetadata>::make_ref(MAGIC, VERSION);
    header->serialize(p_file);
    header.unref();
    // Store constants
//    STORE_CONSTANTS(store_32, u32_constants)
//    STORE_CONSTANTS(store_64, u64_constants)
//    STORE_CONSTANTS(store_float, f32_constants)
//    STORE_CONSTANTS(store_double, f64_constants)
//    STORE_CONSTANTS(store_string, string_literals)
    // Store methods metadata
    p_file->store_32(methods_metadata.size());
    for (const auto& item : methods_metadata){
        item->serialize(p_file);
        auto offset = item->get_method_offset();
        if (offset) method_metadata_offset_loc[item->method_name] = (uint64_t)offset;
    }
    // Reserved space
    for (int i = 0; i < INSTRUCTIONS_RESERVED; i++)
        p_file->store_8(0);
    // Method body
    p_file->store_16(method_bodies.size());
    for (const auto& body : method_bodies){
        method_body_offset_loc[body->method_name] = (uint64_t)p_file->get_pos();
        body->serialize(p_file);
    }
    // Replace offsets from metadata with actual offsets
    auto it = method_body_offset_loc.const_iterator();
    while (it.move_next()){
        auto metadata_offset = method_metadata_offset_loc[it.get_pair().key];
        p_file->seek(metadata_offset);
        p_file->store_64(it.get_pair().value);
    }
#undef STORE_CONSTANTS
}

void NexusBytecodeRawInstruction::deserialize(const FilePointer& p_file) {
    opcode = (NexusSerializedBytecode::OpCode)p_file->get_16();
    auto argc = p_file->get_8();
    arguments = Vector<Ref<NexusBytecodeArgument>>(argc);
    for (unsigned char i = 0; i < argc; i++){
        auto arg_type = (NexusSerializedBytecode::DataType)p_file->get_8();
        NexusBytecodeArgument* argument = nullptr;
        switch (arg_type){
            case VOID:
                argument = new NexusBytecodeArgument();
                break;
            case UNSIGNED_32_BIT_INTEGER:
                argument = new NexusBytecodeArgument(p_file->get_32());
                break;
            case SIGNED_32_BIT_INTEGER:
                argument = new NexusBytecodeArgument((int32_t)p_file->get_32());
                break;
            case UNSIGNED_64_BIT_INTEGER:
                argument = new NexusBytecodeArgument(p_file->get_64());
                break;
            case SIGNED_64_BIT_INTEGER:
                argument = new NexusBytecodeArgument((int64_t)p_file->get_64());
                break;
            case SINGLE_PRECISION_FLOATING_POINT:
                argument = new NexusBytecodeArgument(p_file->get_float());
                break;
            case DOUBLE_PRECISION_FLOATING_POINT:
                argument = new NexusBytecodeArgument(p_file->get_double());
                break;
            case STRING_LITERAL:
                argument = new NexusBytecodeArgument(p_file->get_string());
                break;
            // Not supported / Can never be here
            case METHOD:
            case STRING:
            case REFERENCE_COUNTED_OBJECT:
            case NONE:
                throw BytecodeParseException("Type not supported");
        }
        Ref<NexusBytecodeArgument> as_ref = Ref<NexusBytecodeArgument>::init(argument);
        arguments.push_back(as_ref);
    }
}
void NexusBytecodeRawInstruction::serialize(FilePointer& p_file) const {
    p_file->store_16(opcode);
    p_file->store_8(arguments.size());
    for (const auto& item : arguments){
        auto type = item->type;
        p_file->store_8((uint8_t)type);
        switch (type){
            case VOID:
                break;
            case UNSIGNED_32_BIT_INTEGER:
                p_file->store_32(item->get_data<uint32_t>());
                break;
            case SIGNED_32_BIT_INTEGER:
                p_file->store_32(item->get_data<int32_t>());
                break;
            case UNSIGNED_64_BIT_INTEGER:
                p_file->store_64(item->get_data<uint64_t>());
                break;
            case SIGNED_64_BIT_INTEGER:
                p_file->store_64(item->get_data<int64_t>());
                break;
            case SINGLE_PRECISION_FLOATING_POINT:
                p_file->store_float(item->get_data<float>());
                break;
            case DOUBLE_PRECISION_FLOATING_POINT:
                p_file->store_double(item->get_data<double>());
                break;
            case STRING_LITERAL:
                p_file->store_string(item->get_data<VString>());
                break;
            // Not supported / Can never be here
            case METHOD:
            case STRING:
            case REFERENCE_COUNTED_OBJECT:
            case NONE:
                throw BytecodeParseException("Type not supported");
        }
    }
}

void NexusBytecodeMethodBody::deserialize(const FilePointer& p_file){
    method_name = p_file->get_string();
    auto argc = p_file->get_8();
    arguments = Vector<Ref<NexusBytecodeArgument>>(argc);
    for (unsigned char i = 0; i < argc; i++){
        auto arg_type = (NexusSerializedBytecode::DataType)p_file->get_8();
        Ref<NexusBytecodeArgument> arg = Ref<NexusBytecodeArgument>::make_ref(arg_type);
        arguments.push_back(arg);
    }
    // Max possible value stored inside the stack
    max_stack = p_file->get_16();
    auto locals_init_count = p_file->get_32();
    locals_init = Vector<Ref<NexusBytecodeArgument>>(locals_init_count);
    for (uint32_t i = 0; i < locals_init_count; i++){
        auto arg_type = (NexusSerializedBytecode::DataType)p_file->get_8();
        Ref<NexusBytecodeArgument> arg = Ref<NexusBytecodeArgument>::make_ref(arg_type);
        locals_init.push_back(arg);
    }
    auto instructions_count = p_file->get_32();
    instructions = Vector<Ref<NexusBytecodeRawInstruction>>(instructions_count);
    for (uint32_t i = 0; i < instructions_count; i++){
        Ref<NexusBytecodeRawInstruction> instruction = Ref<NexusBytecodeRawInstruction>::make_ref();
        instruction->deserialize(p_file);
        instructions.push_back(instruction);
    }
}
void NexusBytecodeMethodBody::serialize(FilePointer& p_file) const {
    p_file->store_string(method_name);
    p_file->store_8(arguments.size());
    for (const auto& arg : arguments){
        p_file->store_8(arg->type);
    }
    p_file->store_16(max_stack);
    p_file->store_32(locals_init.size());
    for (const auto& local : locals_init){
        p_file->store_8(local->type);
    }
    p_file->store_32(instructions.size());
    for (const auto& instruction : instructions){
        instruction->serialize(p_file);
    }
}
