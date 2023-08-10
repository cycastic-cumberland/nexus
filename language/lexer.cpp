//
// Created by cycastic on 7/17/2023.
//

#include "lexer.h"
#include "../core/types/stack.h"

const char NexusLexer:: token_characters[TK_MAX] = {
        '\0',
        '\0',
        '\0',
        '\0',
        '\0',
        '\0',
        '\0',
        '\0',
        '\0',
        ' ',
        '\t',
        '\n',
        ';',
        ',',
        '.',
        '\\',
        // Operators are not looked up
        '\0',
        '\0',
        '\0',
        '\0',
        '\0',
        '\0',
        '\0',
        '\0',
        '\0',
        '\0',
        '\0',
        '\0',
        '\0',
        '\'',

        '`',
        '~',
        '@',
        '#',
        '$',

        '\"',
        '(',
        ')',
        '{',
        '}',
};

HashMap<wchar_t, NexusLexer::TokenType>* NexusLexer::tokens_map = nullptr;
HashMap<InternedString, NexusLexer::TokenType>* NexusLexer::operators_map = nullptr;


NexusLexer::LexicalError NexusLexer::tokenize_text(const InternedString &text) {
#define FLUSH_LEXEME() {                    \
    LexicalError err;                       \
    flush_lexeme(&err);                     \
    if (err.type != LE_NONE) return err;    \
}
    clear_tokens();
    uint32_t current_line = 1;
    uint32_t current_column = 1;
    uint32_t last_line = 1;
    uint32_t last_column = 1;
    bool escape_enabled = false;
    InternedString current_lexeme{};
    Stack<TokenType> punctuations{};
    const auto record_coordinate = [&]() -> void {
        last_line = current_line;  last_column = current_column;
    };
    const auto flush_lexeme_detailed = [&](const InternedString& lex,
            const uint32_t& x, const uint32_t& y, TokenType type = TK_IDENTIFIER) -> void {
        Token tok = (type == TK_IDENTIFIER && lex[0] >= L'0' && lex[0] <= L'9') ?
                    Token(TK_NUMBER, lex, x, y) : Token(type, lex, x, y);

        tokens.push_back(tok);
    };
    const auto flush_lexeme = [&](LexicalError* err = nullptr) -> void {
        if (current_lexeme.empty()) return;
        if (!tokens.empty() && tokens.last().type == TK_PU_DOUBLE_QUOTE) {
            flush_lexeme_detailed(current_lexeme, last_line, last_column, TK_STRING);
            current_lexeme = "";
            record_coordinate();
            return;
        }
        InternedString buffer{};
        wchar_t last_char = 0;
        uint32_t current_column_cursor = last_column;
#define ASSIGNMENT_FLUSH_HELPER(type){                                          \
        buffer += L'=';                                                         \
        flush_lexeme_detailed(buffer, last_line, current_column_cursor, type);  \
        last_char = 0;                                                          \
        buffer = "";                                                            \
    }
#define THROW_LEX_ERR(what){                                                    \
    *err = {LE_INVALID_OPERATOR_PLACEMENT, last_line, current_column_cursor};   \
    return;                                                                     \
}
        for (auto i = 0; i < current_lexeme.length(); i++){
            const auto& curr_char = current_lexeme[i];
            switch (curr_char) {
                case L'+':
                case L'-':
                case L':':
                case L'!':
                case L'=':
                    if (last_char == L'+'){
                        ASSIGNMENT_FLUSH_HELPER(TK_OP_SELF_INCREMENT);
                        break;
                    } else if (last_char == L'-'){
                        ASSIGNMENT_FLUSH_HELPER(TK_OP_SELF_DECREMENT);
                        break;
                    } else if (last_char == L'='){
                        ASSIGNMENT_FLUSH_HELPER(TK_OP_EQUALITY);
                        break;
                    } else if (last_char == L':'){
                        ASSIGNMENT_FLUSH_HELPER(TK_OP_TYPE_DEDUCED_ASSIGN);
                        break;
                    } else if (last_char == L'<'){
                        ASSIGNMENT_FLUSH_HELPER(TK_OP_LOWER_OR_EQUAL);
                        break;
                    } else if (last_char == L'>'){
                        ASSIGNMENT_FLUSH_HELPER(TK_OP_HIGHER_OR_EQUAL);
                        break;
                    } else if (last_char == L'!'){
                        ASSIGNMENT_FLUSH_HELPER(TK_OP_DIFFERENTIAL);
                        break;
                    } else if (!buffer.empty()){
                        if (buffer == "!") THROW_LEX_ERR(LE_INVALID_OPERATOR_PLACEMENT);
                        auto type = get_operators_map().has(buffer) ? get_operators_map()[buffer] : TK_IDENTIFIER;
                        flush_lexeme_detailed(buffer, last_line, current_column_cursor, type);
                        last_char = 0;
                        buffer = "";
                        // No break
                    }
                    goto append_character;
                case L'>':
                case L'<':
                append_character:
                default:
                    last_char = curr_char;
                    buffer += curr_char;
                    if (buffer.length() == 0) current_column_cursor = last_column + i;
            }
        }
#undef ASSIGNMENT_FLUSH_HELPER
        if (!buffer.empty()){
            if (buffer == "!") THROW_LEX_ERR(LE_INVALID_OPERATOR_PLACEMENT);
            auto type = get_operators_map().has(buffer) ? get_operators_map()[buffer] : TK_IDENTIFIER;
            flush_lexeme_detailed(buffer, last_line, current_column_cursor, type);
        }
        current_lexeme = "";
        record_coordinate();
    };
#undef THROW_LEX_ERR

    for (size_t i = 0, s = text.length(); i < s; i++, current_column++){
        const auto& curr_char = text[i];
        auto mapped_type = get_tokens_map().has(curr_char) ?
                           get_tokens_map()[curr_char] : TK_CHARACTER;
        switch (mapped_type) {
            case TK_FO_BACK_QUOTE:
            case TK_FO_TILDA:
            case TK_FO_AT:
            case TK_FO_HASH:
            case TK_FO_DOLLAR: if (!punctuations.empty()){
                if (punctuations.peek_last() == TK_PU_DOUBLE_QUOTE) goto continue_as_usual;
                else return {LE_INVALID_CHARACTER, current_line, current_column};
            } else return {LE_INVALID_CHARACTER, current_line, current_column};
            case TK_EOL:{
                if (!punctuations.empty()){
                    auto sample = punctuations.peek_last();
                    if (sample != TK_PU_OPENING_CURLY_BRACKET && sample != TK_PU_OPENING_PARENTHESIS)
                        return {LE_UNEXPECTED_LINE_BREAK, current_line, current_column};
                }
                current_line++;
                current_column = 0;
                // No break here
            }
            case TK_SPACE:
            case TK_TAB: if ((punctuations.empty() || punctuations.peek_last() != TK_PU_DOUBLE_QUOTE) && current_lexeme.empty()) break;
            case TK_DE_SEMICOLON:
            case TK_DE_COMMA:
            case TK_DE_DOT: if (punctuations.empty() || punctuations.peek_last() != TK_PU_DOUBLE_QUOTE || mapped_type == TK_EOL) {
                FLUSH_LEXEME();
                if (!(mapped_type >= TK_EOF && mapped_type <= TK_EOL))
                    tokens.push_back(Token(mapped_type, InternedString(curr_char),
                                       last_line, last_column));
                record_coordinate();
                break;
            } else goto continue_as_usual;
            case TK_DE_CHAR_ESCAPE:
                if (punctuations.empty() || punctuations.peek_last() != TK_PU_DOUBLE_QUOTE)
                    return {LE_INVALID_STRING_ESCAPE_PLACEMENT, current_line, current_column};
                else {
                    escape_enabled = true;
                    break;
                }
            case TK_PU_DOUBLE_QUOTE:
                if (escape_enabled) goto continue_as_usual;
                else {
                    if (!punctuations.empty()) {
                        auto sample = punctuations.pop();
                        // If another double quote was spotted, continue as usual
                        // If not, return the pattern
                        if (sample != TK_PU_DOUBLE_QUOTE)
                            punctuations.push(sample);
                        else goto skip_punctuation_push;
                    }
                }
            case TK_PU_OPENING_PARENTHESIS:
            case TK_PU_OPENING_CURLY_BRACKET:
                punctuations.push(mapped_type);
                skip_punctuation_push:
                FLUSH_LEXEME();
                tokens.push_back(Token(mapped_type, InternedString(curr_char),
                                       last_line, last_column));
                record_coordinate();
                break;
            case TK_PU_CLOSING_PARENTHESIS:
            case TK_PU_CLOSING_CURLY_BRACKET: if (punctuations.empty() || punctuations.peek_last() != TK_PU_DOUBLE_QUOTE) {
                if (punctuations.empty()) return {LE_INVALID_CLOSING_PATTERN_PLACEMENT, current_line, current_column};
                auto sample = punctuations.pop();
                // Mismatch closing pattern
                if ((sample != TK_PU_OPENING_PARENTHESIS && mapped_type == TK_PU_CLOSING_PARENTHESIS) ||
                     sample != TK_PU_OPENING_CURLY_BRACKET && mapped_type == TK_PU_CLOSING_CURLY_BRACKET)
                    return {LE_MISMATCH_CLOSING_PATTERN, current_line, current_column};
                // Does not return pattern
                FLUSH_LEXEME();
                tokens.push_back(Token(mapped_type, InternedString(curr_char),
                                       last_line, last_column));
                record_coordinate();
                break;
            }
            case TK_EOF:
                break;
            case TK_CHARACTER:
            continue_as_usual:
            default:
                if (escape_enabled){
                    escape_enabled = false;
                    switch (curr_char) {
                        case L'\n':
                            current_lexeme += L'\n';
                            break;
                        case L'\r':
                            current_lexeme += L'\r';
                            break;
                        case L'\t':
                            current_lexeme += L'\t';
                            break;
                        case L'\\':
                            current_lexeme += L'\\';
                            break;
                        case L'\"':
                            current_lexeme += L'\"';
                            break;
                        default:
                            return {LE_INVALID_ESCAPE_SEQUENCE_PLACEMENT, current_line, current_column};
                    }
                } else
                    current_lexeme += curr_char;
                if (current_lexeme.length() == 1){
                    record_coordinate();
                }
        }
    }
#undef FLUSH_LEXEME
    tokens.push_back({TK_EOF, "<EOF>", current_line, current_column});

    return {LE_NONE, 0, 0};
}

HashMap<InternedString, NexusLexer::TokenType> *NexusLexer::allocate_operators_map() {
    auto map = new HashMap<InternedString, NexusLexer::TokenType>(0.75, 64, false);
    map->operator[]("+") = NexusLexer::TK_OP_ADD;
    map->operator[]("+=") = NexusLexer::TK_OP_SELF_INCREMENT;
    map->operator[]("-") = NexusLexer::TK_OP_SUBTRACT;
    map->operator[]("-=") = NexusLexer::TK_OP_SELF_DECREMENT;
    map->operator[]("=") = NexusLexer::TK_OP_ASSIGN;
    map->operator[](":") = NexusLexer::TK_OP_TYPE_DECLARATION;
    map->operator[](":=") = NexusLexer::TK_OP_TYPE_DEDUCED_ASSIGN;
    map->operator[]("==") = NexusLexer::TK_OP_EQUALITY;
    map->operator[]("<") = NexusLexer::TK_OP_LOWER;
    map->operator[]("<=") = NexusLexer::TK_OP_LOWER_OR_EQUAL;
    map->operator[](">") = NexusLexer::TK_OP_HIGHER;
    map->operator[](">=") = NexusLexer::TK_OP_HIGHER_OR_EQUAL;
    return map;
}
