# Nexus

Release under MIT license, see "LICENSE.txt" for details.

## Introduction

Nexus is (going to be) an all-in-one tool for compiling and running application written in... Nexus. 

```
func main(): void {
    var text := String.parse("Hello World!");
    print(text);
}
```

## Features

### Compilation

- Lexer: partially completed
- Parser: in-progress
- Codegen: to be worked on

### Virtual machine (Nexus runtime)

- Bytecodes handling: completed, untested
- JIT: to be worked on 
- Virtual machine stack: completed, partially tested
- Memory management: completed, tested
- Native function calls: to be worked on
- ManagedThread pool: completed, untested
- Task scheduling: completed, untested
- Asynchronous execution: in-progress