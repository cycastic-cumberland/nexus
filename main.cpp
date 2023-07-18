#include <iostream>
#include "core/types/vstring.h"
#include "core/types/vector.h"
#include "core/types/hashmap.h"
#include "core/language/lexer.h"

int main() {
    NexusLexer lexer{};
    VString text = R"(
        func hello_world(){
            var a = 237;
            print("Hello += world!");
        }
)";
    auto re = lexer.tokenize_text(text);
    const auto& tokens = lexer.extract_tokens();
    for (int i = 0; i < tokens.size(); i++){
        const auto& token = tokens[i];
        std::cout << "(" << (size_t)token.type << ", " << token.lexeme
                  << ", (" << token.line << ", " << token.column << "))\n";
    }
    return 0;
}
