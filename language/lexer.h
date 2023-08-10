//
// Created by cycastic on 7/17/2023.
//

#ifndef NEXUS_LEXER_H
#define NEXUS_LEXER_H

#include "../core/types/vstring.h"
#include "../core/types/vector.h"
#include "../core/types/hashmap.h"
#include "../core/types/interned_string.h"

class NexusLexer {
public:
    enum TokenType : unsigned int {
        TK_EMPTY,
        TK_CHARACTER,
        TK_STRING,
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
        TK_DE_CHAR_ESCAPE,
        // Operators
        TK_OP_ADD,
        TK_OP_SELF_INCREMENT,
        TK_OP_SUBTRACT,
        TK_OP_SELF_DECREMENT,
        TK_OP_ASSIGN,
        TK_OP_TYPE_DECLARATION,
        TK_OP_TYPE_DEDUCED_ASSIGN,
        TK_OP_EQUALITY,
        TK_OP_DIFFERENTIAL,
        TK_OP_LOWER,
        TK_OP_LOWER_OR_EQUAL,
        TK_OP_HIGHER,
        TK_OP_HIGHER_OR_EQUAL,
        // Forbidden
        TK_FO_BACK_QUOTE,
        TK_FO_TILDA,
        TK_FO_AT,
        TK_FO_HASH,
        TK_FO_DOLLAR,
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
        InternedString lexeme;
        uint32_t line;
        uint32_t column;
        Token(const TokenType& token_type, const InternedString& lex, const uint32_t& row, const uint32_t& col){
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
        LE_INVALID_CHARACTER,
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
    static HashMap<wchar_t, TokenType>* tokens_map;
    static HashMap<InternedString, TokenType>* operators_map;

    Vector<Token> tokens{};

    static _FORCE_INLINE_ HashMap<wchar_t, NexusLexer::TokenType>* allocate_tokens_map(){
        auto map = new HashMap<wchar_t, NexusLexer::TokenType>(0.75, 512, false);
        for (int i = TokenType::TK_EMPTY; i < TokenType::TK_MAX; i++){
            map->operator[](token_characters[i]) = (TokenType)i;
        }
        return map;
    }
    static HashMap<InternedString, NexusLexer::TokenType>* allocate_operators_map();
public:
    static _FORCE_INLINE_ const HashMap<wchar_t, TokenType>& get_tokens_map() { return *tokens_map; }
    static _FORCE_INLINE_ const HashMap<InternedString, NexusLexer::TokenType>& get_operators_map() { return *operators_map; }

    static _FORCE_INLINE_ void init_cache() {
        if (tokens_map == nullptr)
            tokens_map = allocate_tokens_map();
        if (operators_map == nullptr)
            operators_map = allocate_operators_map();
    }
    static _FORCE_INLINE_ void free_cache() {
        delete tokens_map;
        delete operators_map;
    }

    _FORCE_INLINE_ void clear_tokens() { tokens.clear(); }
    _NO_DISCARD_ _FORCE_INLINE_ const Vector<Token>& extract_tokens() const { return tokens; }
    LexicalError tokenize_text(const InternedString& text);

    NexusLexer()= default;
};

#endif //NEXUS_LEXER_H
