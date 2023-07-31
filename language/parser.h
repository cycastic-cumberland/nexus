//
// Created by cycastic on 7/18/2023.
//

#ifndef NEXUS_PARSER_H
#define NEXUS_PARSER_H

#include "../core/types/object.h"
#include "bytecode.h"
#include "lexer.h"

struct NexusAbstractSyntaxToken;
struct NexusASTSymbol;
struct NexusASTExpression;

struct NexusAbstractSyntaxToken : public Object {

};

struct NexusASTSymbol : public NexusAbstractSyntaxToken {
    VString symbol;
    NexusSerializedBytecode::DataType data_type = NexusSerializedBytecode::NONE;
    // If it's a constant
    NexusSerializedBytecode::DataType constant_type = NexusSerializedBytecode::NONE;
};

struct NexusASTExpression : public NexusAbstractSyntaxToken {
    enum Operator {
        OP_ADD,
        OP_SUBTRACT,
        OP_MULTIPLY,
        OP_DIVIDE,
        OP_MODULUS,

        OP_ASSIGN,
        OP_ADD_ASSIGN,
        OP_SUBTRACT_ASSIGN,
        OP_MULTIPLY_ASSIGN,
        OP_DIVIDE_ASSIGN,
        OP_MODULUS_ASSIGN,

        OP_EQUAL,
        OP_NOT_EQUAL,
        OP_GREATER,
        OP_GREATER_OR_EQUAL,
        OP_LESSER,
        OP_LESSER_OR_EQUAL,

        OP_AND,
        OP_OR,
        OP_NOT,

        OP_BIT_AND,
        OP_BIT_OR,
        OP_BIT_XOR,
        OP_BIT_NOT,
        OP_LEFT_SHIRT,
        OP_RIGHT_SHIFT,

        OP_INCREMENT,
        OP_DECREMENT,

        OP_MEMBER_DOT,

        OP_SPECIAL,
        OP_END,
    };

    static HashMap<VString, Operator, 128>* operators_map;

    Operator op = OP_END;
    Ref<NexusASTExpression> right_operand{};
    Ref<NexusASTSymbol> left_operand{};
};


struct NexusASTArgumentList : public NexusAbstractSyntaxToken {
    Vector<Ref<NexusASTExpression>> arguments{}; // As expression
};

struct NexusASTInstructionRETURN;
struct NexusASTInstructionBRANCH;
struct NexusASTInstructionIF;
struct NexusASTInstructionELSE;
struct NexusASTInstructionWHILE;

struct NexusASTInstruction : public NexusAbstractSyntaxToken {
    VString method_invoke{};
    VString return_to_symbol{};
    Ref<NexusASTArgumentList> arguments_list;

    Ref<NexusASTInstructionRETURN> to_return_instruction();
    Ref<NexusASTInstructionIF> to_if_instruction();
    Ref<NexusASTInstructionELSE> to_else_instruction();
    Ref<NexusASTInstructionWHILE> to_while_instruction();
};

struct NexusASTInstructionRETURN : public NexusASTInstruction {
    // return_to_symbol will act as the return value;
};

struct NexusASTInstructionBRANCH : public NexusASTInstruction {
    Vector<Ref<NexusASTInstruction>> code_block{};
};

struct NexusASTInstructionIF : public NexusASTInstructionBRANCH {
    // arguments_list will act as the conditional_lock;
};

struct NexusASTInstructionELSE : public NexusASTInstructionBRANCH {

};

struct NexusASTInstructionWHILE : public NexusASTInstructionBRANCH {

};

struct NexusASTMethodDeclaration : public NexusAbstractSyntaxToken {
    VString partial_method_name;
    VString full_method_name;
    VString parent_class;
    Ref<NexusASTArgumentList> arguments;
    Vector<Ref<NexusASTInstruction>> instructions;
};

class NexusParser {
public:
    static void initialize_parser_symbols_map();
    static void delete_parser_symbols_map();
public:
    NexusParser() = default;

    void parse_tokens(const Vector<NexusLexer::Token>& p_tokens);
};

#endif //NEXUS_PARSER_H
