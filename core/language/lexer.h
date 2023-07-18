//
// Created by cycastic on 7/17/2023.
//

#ifndef NEXUS_LEXER_H
#define NEXUS_LEXER_H

#include "../types/vstring.h"
#include "../types/vector.h"
#include "../types/hashmap.h"

class NexusLexer {
public:
    enum TokenType : unsigned int {
        TK_EMPTY,
        TK_CHARACTER,
        TK_IDENTIFIER,
        TK_NUMBER,
        TK_CONSTANT,
        // Keywords
        TK_KW_FUNC,
        // Types
        TK_TY_INT,
        // Delimiters
        TK_EOF,
        TK_SPACE,
        TK_TAB,
        TK_EOL,
        TK_DE_SEMICOLON,
        TK_DE_COMMA,
        TK_DE_DOT,
        TK_DE_COLON,
        TK_DE_CHAR_ESCAPE,
        // Operators
        TK_OP_ADD,
        TK_OP_SELF_INCREMENT,
        TK_OP_SUBTRACT,
        TK_OP_SELF_DECREMENT,
        TK_OP_ASSIGN,
        TK_OP_EQUALITY,
        TK_OP_LOWER,
        TK_OP_LOWER_OR_EQUAL,
        TK_OP_HIGHER,
        TK_OP_HIGHER_OR_EQUAL,
        // Punctuation
        TK_PU_QUOTE,
        TK_PU_DOUBLE_QUOTE,
        TK_PU_OPENING_PARENTHESIS,
        TK_PU_CLOSING_PARENTHESIS,
        TK_PU_OPENING_CURLY_BRACKET,
        TK_PU_CLOSING_CURLY_BRACKET,
        TK_MAX
    };
    struct Token {
        TokenType type;
        VString lexeme;
        uint32_t line;
        uint32_t column;
        Token(const TokenType& token_type, const VString& lex, const uint32_t& row, const uint32_t& col){
            type = token_type;
            lexeme = lex;
            line = row;
            column = col;
        }
    };
    enum LexicalErrorType : unsigned int {
        LE_NONE,
        LE_UNEXPECTED_LINE_BREAK,
        LE_MISMATCH_CLOSING_PATTERN,
        LE_INVALID_OPENING_PATTERN_PLACEMENT,
        LE_INVALID_CLOSING_PATTERN_PLACEMENT,
        LE_INVALID_STRING_ESCAPE_PLACEMENT,
        LE_INVALID_ESCAPE_SEQUENCE_PLACEMENT,
        LE_INVALID_OPERATOR_PLACEMENT,
        LE_MAX,
    };
    struct LexicalError {
        LexicalErrorType type{LE_NONE};
        uint32_t line{};
        uint32_t column{};
        LexicalError() = default;
        LexicalError(LexicalErrorType t, const uint32_t& x, const uint32_t& y) {
            type = t;
            line = x;
            column = y;
        }
    };
private:
    static const char token_characters[TK_MAX];
    static HashMap<wchar_t, TokenType, 512>* tokens_map;

    Vector<Token> tokens{};

    static _FORCE_INLINE_ HashMap<wchar_t, NexusLexer::TokenType, 512>* allocate_tokens_map(){
        auto map = new HashMap<wchar_t, NexusLexer::TokenType, 512>();
        for (int i = TokenType::TK_EMPTY; i < TokenType::TK_MAX; i++){
            map->operator[](token_characters[i]) = (TokenType)i;
        }
        return map;
    }
public:
    static _FORCE_INLINE_ const HashMap<wchar_t, TokenType, 512>& get_tokens_map() { return *tokens_map; }

    _FORCE_INLINE_ void clear_tokens() { tokens.clear(); }
    _NO_DISCARD_ _FORCE_INLINE_ const Vector<Token>& extract_tokens() const { return tokens; }
    LexicalError tokenize_text(const VString& text);

    NexusLexer(){
        if (tokens_map == nullptr) tokens_map = allocate_tokens_map();
    }
};

#endif //NEXUS_LEXER_H
