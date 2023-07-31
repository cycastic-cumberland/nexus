//
// Created by cycastic on 7/19/2023.
//

#include "parser.h"

HashMap<VString, NexusASTExpression::Operator, 128>* NexusASTExpression::operators_map = nullptr;

void NexusParser::initialize_parser_symbols_map() {
    if (!NexusASTExpression::operators_map) {
        NexusASTExpression::operators_map = new HashMap<VString, NexusASTExpression::Operator, 128>();
        auto& map = *NexusASTExpression::operators_map;
        map["+"]    = NexusASTExpression::OP_ADD;
        map["-"]    = NexusASTExpression::OP_SUBTRACT;
        map["*"]    = NexusASTExpression::OP_MULTIPLY;
        map["/"]    = NexusASTExpression::OP_DIVIDE;
        map["%"]    = NexusASTExpression::OP_MODULUS;

        map["="]    = NexusASTExpression::OP_ASSIGN;
        map["+="]   = NexusASTExpression::OP_ADD_ASSIGN;
        map["-="]   = NexusASTExpression::OP_SUBTRACT_ASSIGN;
        map["*="]   = NexusASTExpression::OP_MULTIPLY_ASSIGN;
        map["/="]   = NexusASTExpression::OP_DIVIDE_ASSIGN;
        map["%="]   = NexusASTExpression::OP_MODULUS_ASSIGN;

        map["=="]   = NexusASTExpression::OP_EQUAL;
        map["!="]   = NexusASTExpression::OP_NOT_EQUAL;
        map[">"]    = NexusASTExpression::OP_GREATER;
        map[">="]   = NexusASTExpression::OP_GREATER_OR_EQUAL;
        map["<"]    = NexusASTExpression::OP_LESSER;
        map["<="]   = NexusASTExpression::OP_LESSER_OR_EQUAL;

        map["&&"]   = NexusASTExpression::OP_AND;
        map["||"]   = NexusASTExpression::OP_OR;
        map["!"]    = NexusASTExpression::OP_NOT;

        map["&"]    = NexusASTExpression::OP_BIT_AND;
        map["|"]    = NexusASTExpression::OP_BIT_OR;
        map["^"]    = NexusASTExpression::OP_BIT_XOR;
        map["~"]    = NexusASTExpression::OP_BIT_NOT;
        map["<<"]   = NexusASTExpression::OP_LEFT_SHIRT;
        map[">>"]   = NexusASTExpression::OP_RIGHT_SHIFT;

        map["--"]   = NexusASTExpression::OP_INCREMENT;
        map["++"]   = NexusASTExpression::OP_DECREMENT;

        map["."]    = NexusASTExpression::OP_MEMBER_DOT;
    }
}

void NexusParser::delete_parser_symbols_map() {
    delete NexusASTExpression::operators_map;
}

void NexusParser::parse_tokens(const Vector<NexusLexer::Token> &p_tokens) {

}

template <class TTo, class TFrom>
static Ref<TTo> nexus_ast_cast_helper(TFrom* p_from){
    auto ptr = dynamic_cast<TTo*>(p_from);
    return Ref<TTo>::from_initialized_object(ptr);
}

Ref<NexusASTInstructionRETURN> NexusASTInstruction::to_return_instruction() {
    return nexus_ast_cast_helper<NexusASTInstructionRETURN>(this);
}

Ref<NexusASTInstructionIF> NexusASTInstruction::to_if_instruction() {
    return nexus_ast_cast_helper<NexusASTInstructionIF>(this);
}

Ref<NexusASTInstructionELSE> NexusASTInstruction::to_else_instruction() {
    return nexus_ast_cast_helper<NexusASTInstructionELSE>(this);
}

Ref<NexusASTInstructionWHILE> NexusASTInstruction::to_while_instruction() {
    return nexus_ast_cast_helper<NexusASTInstructionWHILE>(this);
}
