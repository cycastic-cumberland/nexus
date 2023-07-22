#include <iostream>
#include "core/types/vstring.h"
#include "language/lexer.h"
#include "core/types/object.h"
#include "language/parser.h"
#include "core/io/file_access_server.h"
#include "core/output.h"

int main() {
//    init_locale();
    NexusLexer lexer{};
    VString text = R"(
        func hello_world(): void {
            var a := 237;
            a = a + 1;
            print("Hello += world!");
        }
)";
    auto re = lexer.tokenize_text(text);
    const auto& tokens = lexer.extract_tokens();
    for (const auto & token : tokens){
        std::cout << "(type: " << (size_t)token.type << ", lexeme: \"" << token.lexeme
                  << "\", coord: (" << token.line << ", " << token.column << "))\n";
    }
    return 0;
}
