//
// Created by cycastic on 7/17/2023.
//

#ifndef NEXUS_LEXER_H
#define NEXUS_LEXER_H

class NexusLexer {
public:
    enum TokenTypes {
        TK_EMPTY,
        TK_KEYWORD,

        TK_MAX
    };
    enum KeywordTypes {
        // Types
        K_FUNC,
        K_INT,
        K_STRING,
        K_MAX
    };
    struct Tokens {

    };
    static const char* token_names[TK_MAX];
    static const char* keyword_names[K_MAX];
private:
};

#endif //NEXUS_LEXER_H
