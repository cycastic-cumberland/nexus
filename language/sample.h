//
// Created by cycastic on 07/08/2023.
//

#ifndef NEXUS_SAMPLE_H
#define NEXUS_SAMPLE_H

#include "lexer.h"
#include "../runtime/config.h"
#include "../runtime/nexus_output.h"

static void lexing_test(){
    initialize_nexus_runtime(true);
    static constexpr char text[] = R"(
func main(): void {
    var text := String.parse("Hello World!");
    print(text);
}
)";
    NexusLexer::init_cache();
    NexusLexer lexer{};
    auto re = lexer.tokenize_text(text);
    print_line(VString("Error: ") + itos(re.type));
    const auto& tokens = lexer.extract_tokens();
    for (const auto& token : tokens)
        print_line(VString("(type: ") + itos(token.type) + ", lexeme: \""
                    + token.lexeme + "\", coord: (" + itos(token.line) + ", "
                    + itos(token.column) + "))");
    NexusLexer::free_cache();
    destroy_nexus_runtime();
}

#endif //NEXUS_SAMPLE_H
