//
// Created by cycastic on 7/19/2023.
//

#include "parser.h"

HashMap<InternedString, NexusASTNode::OperatorType>* NexusParser::operators_map = nullptr;
HashMap<InternedString, NexusASTNode::KeywordType>*  NexusParser::keywords_map  = nullptr;
NFA<NexusParser*>* NexusParser::state_machine = nullptr;

void NexusParser::init_cache() {
    if (!operators_map){
        operators_map = new HashMap<InternedString, NexusASTNode::OperatorType>(0.75, 128, false);
        auto& op_map = *operators_map;
        op_map["+"]    = NexusASTNode::OP_ADD;
        op_map["-"]    = NexusASTNode::OP_SUBTRACT;
        op_map["*"]    = NexusASTNode::OP_MULTIPLY;
        op_map["/"]    = NexusASTNode::OP_DIVIDE;
        op_map["%"]    = NexusASTNode::OP_MODULUS;

        op_map["="]    = NexusASTNode::OP_ASSIGN;
        op_map["+="]   = NexusASTNode::OP_ADD_ASSIGN;
        op_map["-="]   = NexusASTNode::OP_SUBTRACT_ASSIGN;
        op_map["*="]   = NexusASTNode::OP_MULTIPLY_ASSIGN;
        op_map["/="]   = NexusASTNode::OP_DIVIDE_ASSIGN;
        op_map["%="]   = NexusASTNode::OP_MODULUS_ASSIGN;

        op_map["=="]   = NexusASTNode::OP_EQUAL;
        op_map["!="]   = NexusASTNode::OP_NOT_EQUAL;
        op_map[">"]    = NexusASTNode::OP_GREATER;
        op_map[">="]   = NexusASTNode::OP_GREATER_OR_EQUAL;
        op_map["<"]    = NexusASTNode::OP_LESSER;
        op_map["<="]   = NexusASTNode::OP_LESSER_OR_EQUAL;

        op_map["&&"]   = NexusASTNode::OP_AND;
        op_map["||"]   = NexusASTNode::OP_OR;
        op_map["!"]    = NexusASTNode::OP_NOT;

        op_map["&"]    = NexusASTNode::OP_BIT_AND;
        op_map["|"]    = NexusASTNode::OP_BIT_OR;
        op_map["^"]    = NexusASTNode::OP_BIT_XOR;
        op_map["~"]    = NexusASTNode::OP_BIT_NOT;
        op_map["<<"]   = NexusASTNode::OP_LEFT_SHIFT;
        op_map[">>"]   = NexusASTNode::OP_RIGHT_SHIFT;

        op_map["--"]   = NexusASTNode::OP_INCREMENT;
        op_map["++"]   = NexusASTNode::OP_DECREMENT;

        op_map["."]    = NexusASTNode::OP_MEMBER_DOT;
    }
    if (!keywords_map) {
        keywords_map = new HashMap<InternedString, NexusASTNode::KeywordType>(0.75, 128, false);
        auto& k_map = *keywords_map;
        k_map["func"]       = NexusASTNode::KW_FUNC;
        k_map["void"]       = NexusASTNode::KW_VOID;
        k_map["return"]     = NexusASTNode::KW_RETURN;
    }
    if (!state_machine){
        state_machine = new NFA<NexusParser*>();
    }
}

void NexusParser::free_cache() {
    delete operators_map;
    delete keywords_map;
    delete state_machine;
}
