//
// Created by cycastic on 7/18/2023.
//

#ifndef NEXUS_PARSER_H
#define NEXUS_PARSER_H

#include "../core/types/interned_string.h"
#include "../core/types/hashset.h"
#include "../core/types/object.h"
#include "standard_types.h"
#include "lexer.h"
#include "../core/nfa.h"

struct NexusASTNode : public ThreadUnsafeObject {
    enum Type {
        IntegerValue,
        RealValue,
        StringLiteral,

        Identifier,

        MethodDeclaration,
        ArgumentsDeclaration,     // (int a, double b = 1.2)
        ArgumentDeclaration,      //         double b = 1.2
        Arguments,                // (32, 1.1)
        Argument,                 //      1.1
        MethodBody,
        MethodInvocation,

        VariableDeclaration,
        VariableInitialization,

        UnaryOperator,
        BinaryOperator,
    };
    enum OperatorType {
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
        OP_LEFT_SHIFT,
        OP_RIGHT_SHIFT,

        OP_INCREMENT,
        OP_DECREMENT,

        OP_MEMBER_DOT,
    };
    enum KeywordType {
        KW_FUNC,
        KW_VOID,
        KW_RETURN,
    };
protected:
    Type type;
    explicit NexusASTNode(const Type& p_type) : type(p_type) {}
};

struct IntegerValueNode : public NexusASTNode {
    uint64_t value;

    explicit IntegerValueNode(const uint64_t& p_value = 0) : NexusASTNode(IntegerValue), value(p_value) {}
};

struct RealValueNode : public NexusASTNode {
    double value;

    explicit RealValueNode(const double& p_value = 0.0) : NexusASTNode(RealValue), value(p_value) {}
};

struct StringLiteralValueNode : public NexusASTNode {
    InternedString value;

    explicit StringLiteralValueNode(const InternedString& p_value) : NexusASTNode(StringLiteral), value(p_value) {}
};

struct IdentifierNode : public NexusASTNode {
    InternedString identifier;

    explicit IdentifierNode(const InternedString& p_value) : NexusASTNode(Identifier), identifier(p_value) {}
};

struct ArgumentPropertyNode {
    enum PropertyType : uint32_t {
        NO_PROP = 0,
        FINAL = 1,
        WRITE_RESTRICTED = 2,
        STATIC = 4
    };
    uint32_t properties;
};

struct ArgumentDeclarationNode : public NexusASTNode {
    // TODO: Add type
    NexusStandardType type;
    Ref<IdentifierNode> identifier;
    ArgumentPropertyNode argument_property;
    Ref<NexusASTNode> default_value;

    ArgumentDeclarationNode(const NexusStandardType& p_type, const Ref<IdentifierNode>& p_identifier,
                            const ArgumentPropertyNode& p_prop,
                            const Ref<NexusASTNode>& p_default_value = Ref<NexusASTNode>::null())
                                  : NexusASTNode(ArgumentDeclaration),
                                  type(p_type), identifier(p_identifier), argument_property(p_prop), default_value(p_default_value) {}
};

struct ArgumentsDeclarationNode : public NexusASTNode {
    Vector<Ref<ArgumentDeclarationNode>> arguments;

    explicit ArgumentsDeclarationNode(const Vector<Ref<ArgumentDeclarationNode>>& p_arguments
    = Vector<Ref<ArgumentDeclarationNode>>()) : NexusASTNode(ArgumentsDeclaration),
                                                arguments(p_arguments) {}
};

struct ArgumentNode : public NexusASTNode {
    Ref<NexusASTNode> value; // Could either be an identifier, a constant, a call or a binary operator

    explicit ArgumentNode(const Ref<NexusASTNode>& p_value)
    : NexusASTNode(Argument), value(p_value) {}
};

struct ArgumentsNode : public NexusASTNode {
    Vector<Ref<ArgumentNode>> arguments;

    explicit ArgumentsNode(const Vector<Ref<ArgumentNode>>& p_arguments = Vector<Ref<ArgumentNode>>())
    : NexusASTNode(Arguments), arguments(p_arguments) {}
};

struct MethodBodyNode : public NexusASTNode {
    Vector<Ref<NexusASTNode>> instructions;

    explicit MethodBodyNode(const Vector<Ref<NexusASTNode>>& p_instructions = Vector<Ref<NexusASTNode>>())
    : NexusASTNode(MethodBody), instructions(p_instructions) {}
};

struct MethodDeclarationNode : public NexusASTNode {
    InternedString method_name;
    Ref<ArgumentsDeclarationNode> arguments;
    Ref<MethodBodyNode> method_body;

    explicit MethodDeclarationNode(const InternedString& p_method_name,
                                   const Ref<ArgumentsDeclarationNode>& p_arguments = Ref<ArgumentsDeclarationNode>::null(),
                                   const Ref<MethodBodyNode>& p_body = Ref<MethodBodyNode>::null())
                                   : NexusASTNode(MethodDeclaration), method_name(p_method_name),
                                     arguments(p_arguments), method_body(p_body) {}
};

struct MethodInvocationNode : public NexusASTNode {
    InternedString method_name;
    Ref<ArgumentsNode> arguments;

    explicit MethodInvocationNode(const InternedString& p_method_name,
                                  const Ref<ArgumentsNode>& p_args = Ref<ArgumentsNode>::null())
                                  : NexusASTNode(MethodInvocation),
                                  method_name(p_method_name), arguments(p_args) {}
};

struct VariableInitializationNode : public NexusASTNode {
    Ref<NexusASTNode> value; // Could be constant, could be identifier/call, could be binary operator

    explicit VariableInitializationNode(const Ref<NexusASTNode>& p_value = Ref<NexusASTNode>::null())
    : NexusASTNode(VariableInitialization), value(p_value) {}
};

struct VariableDeclarationNode : public NexusASTNode {
    Ref<IdentifierNode> identifier;
    NexusStandardType data_type;
    ArgumentPropertyNode variable_property;
    Ref<VariableInitializationNode> initialization; // null for default constructor

    explicit VariableDeclarationNode(const Ref<IdentifierNode>& p_identifier = Ref<IdentifierNode>::null(),
                            const NexusStandardType& p_data_type = NexusStandardType::MAX_TYPE,
                            const ArgumentPropertyNode& p_prop = ArgumentPropertyNode{.properties = ArgumentPropertyNode::NO_PROP},
                            const Ref<VariableInitializationNode>& p_init = Ref<VariableInitializationNode>::null())
                            : NexusASTNode(VariableDeclaration), identifier(p_identifier), data_type(p_data_type),
                            variable_property(p_prop), initialization(p_init) {}

};

struct UnaryOperatorNode : public NexusASTNode {
    OperatorType op;
    Ref<NexusASTNode> operand;

    explicit UnaryOperatorNode(const OperatorType& p_op,
                               const Ref<NexusASTNode>& p_operand = Ref<NexusASTNode>::null())
                               : NexusASTNode(UnaryOperator), op(p_op), operand(p_operand) {}
};

struct BinaryOperatorNode : public NexusASTNode {
    OperatorType op;
    Ref<NexusASTNode> left_operand;
    Ref<NexusASTNode> right_operand;

    explicit BinaryOperatorNode(const OperatorType& p_op,
                            const Ref<NexusASTNode>& p_left_operand = Ref<NexusASTNode>::null(),
                            const Ref<NexusASTNode>& p_right_operand = Ref<NexusASTNode>::null())
            : NexusASTNode(BinaryOperator), op(p_op),
            left_operand(p_left_operand), right_operand(p_right_operand) {}
};

class NexusParser {
public:
    struct NexusASTPackage : public NexusASTNode {
        HashMap<InternedString, Ref<MethodDeclarationNode>> method_declarations{};
    };
private:
    static HashMap<InternedString, NexusASTNode::OperatorType>* operators_map;
    static HashMap<InternedString, NexusASTNode::KeywordType>* keywords_map;
    static NFA<NexusParser*>* state_machine;
private:
    Ref<NexusASTPackage> result{};
    Vector<NexusLexer::Token> tokens_list{};
    mutable int64_t current_index = -1;

    _NO_DISCARD_ _FORCE_INLINE_ const NexusLexer::Token& get_token() const { current_index++; return tokens_list[size_t(current_index)]; }
public:
    static void init_cache();
    static void free_cache();
};

#endif //NEXUS_PARSER_H
