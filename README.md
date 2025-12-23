# SimpleLang Compiler for an 8-bit CPU

## Introduction

SimpleLang is a minimal high-level programming language designed for educational purposes and targeted at a simple 8-bit CPU.
The objective of this project is to design and implement a complete compiler pipeline that translates SimpleLang programs into
assembly code executable on an 8-bit CPU simulator.

This project demonstrates core concepts of:

- Compiler construction
- Lexical analysis
- Parsing and AST generation
- Symbol table management
- Code generation
- Low-level computer architecture interaction

The compiler is intentionally simple to focus on clarity, correctness, and learning, rather than optimization.

## Folder Structure

- `input.txt` : Example source file used for testing.
- `lexer.c` : Lexer implementation in C.
- `lexer.h` : Lexer header file / public interface.
- `parser.c` : Parser and AST builder implementation in C.
- `output.asm` : Sample generated assembly output.
- `README.md` : Project documentation.

---

## CPU Overview

The target architecture for this compiler is the **lightcode/8bit-computer**, an educational 8-bit CPU implemented in Verilog.

### Key Characteristics

- 8-bit data width
- Register-based architecture
- Simple instruction set
- Supports arithmetic, memory access, branching, and stack operations

### Relevant Instructions Used

| Instruction | Description |
|-----------|------------|
| LOAD A, addr | Load memory into register A |
| STORE A, addr | Store register A into memory |
| LOADI A, value | Load immediate value into A |
| ADD A, B | Add registers |
| SUB A, B | Subtract registers |
| CMP A, value | Compare register A with value |
| JNZ label | Jump if comparison not zero |
| PUSH A | Push register A onto stack |
| POP B | Pop stack value into register B |

The generated assembly is compatible with the provided simulator examples.

---

## SimpleLang Grammar

SimpleLang supports a small but expressive subset of common programming constructs.

### Grammar (EBNF)

```
program     → statement*
statement   → decl | assign | if_stmt

decl        → "int" IDENT ";"
assign      → IDENT "=" expr ";"

expr        → term
            | term ("+" | "-") term

term        → IDENT | NUMBER

if_stmt     → "if" "(" condition ")" "{" statement* "}"

condition   → IDENT "==" IDENT
            | IDENT "==" NUMBER
```

---

## Compiler Architecture

The compiler follows a classic multi-phase architecture:

```
Source Code
   ↓
Lexer
   ↓
Parser
   ↓
Abstract Syntax Tree (AST)
   ↓
Symbol Table
   ↓
Code Generator
   ↓
8-bit Assembly Output
```

Each phase is implemented explicitly and kept independent for clarity.

---

## Lexer

### Purpose

The lexer converts raw source code into a sequence of tokens.

### Responsibilities

- Ignore whitespace
- Identify keywords (`int`, `if`)
- Recognize identifiers and numbers
- Tokenize operators and delimiters

### Supported Tokens

- Keywords: `int`, `if`
- Identifiers: variable names
- Literals: integer constants
- Operators: `=`, `+`, `-`, `==`
- Delimiters: `;`, `{}`, `()`

The lexer is implemented in **C** and exposed via a clean interface using a header file.

---

## Parser

### Parsing Strategy

- Recursive descent parser
- One function per grammar rule
- Single-token lookahead
- Immediate error reporting on syntax violations

### Responsibilities

- Validate syntax
- Enforce declaration-before-use
- Build the Abstract Syntax Tree (AST)

The parser directly maps grammar rules to parsing functions, making the implementation easy to read and extend.

---

## Abstract Syntax Tree (AST)

### Purpose

The AST represents the logical structure of the program independent of syntax.

### Node Types

- NODE_PROGRAM
- NODE_DECL
- NODE_ASSIGN
- NODE_BINOP
- NODE_IF
- NODE_VAR
- NODE_NUM

### Example AST

For:
```
c = a + b;
```

AST structure:
```
ASSIGN
 ├── VAR(c)
 └── BINOP(+)
     ├── VAR(a)
     └── VAR(b)
```

The AST makes code generation straightforward by separating structure from representation.

---

## Symbol Table

### Purpose

The symbol table maps variable names to memory addresses.

### Design

```c
struct Symbol {
    char name[32];
    int memory_address;
};
```

### Memory Allocation Strategy

- Memory starts at `0x10`
- Each declared variable gets the next available address

Example:
```
a → 0x10
b → 0x11
c → 0x12
```

### Guarantees

- Prevents redeclaration
- Prevents use-before-declaration
- Enables direct memory mapping during code generation

---

## Code Generator

### Purpose

Transforms the AST into executable 8-bit assembly code.

### Strategy

- Traverse the AST recursively
- Emit assembly instructions per construct
- Use a stack-based approach for expression evaluation
- Generate unique labels for conditional statements

### Mapping Examples

#### Assignment
```c
a = 10;
```
```asm
LOADI A, 10
STORE A, 0x10
```

#### Arithmetic
```c
c = a + b;
```
```asm
LOAD A, 0x10
PUSH A
LOAD A, 0x11
POP B
ADD A, B
STORE A, 0x12
```

#### Conditional
```c
if (c == 30) {
    c = c + 1;
}
```
```asm
LOAD A, 0x12
CMP A, 30
JNZ if_end_0
LOAD A, 0x12
PUSH A
LOADI A, 1
POP B
ADD A, B
STORE A, 0x12
if_end_0:
```

---

## Example Walkthrough

### SimpleLang Input
```c
int a;
int b;
int c;

a = 3;
b = 4;
c = a + b;
```

### Symbol Table
```
a → 0x10
b → 0x11
c → 0x12
```

### Generated Assembly
```asm
LOADI A, 3
STORE A, 0x10
LOADI A, 4
STORE A, 0x11
LOAD A, 0x10
PUSH A
LOAD A, 0x11
POP B
ADD A, B
STORE A, 0x12
```

### Final Memory State
```
a = 3
b = 4
c = 7
```

---

## Limitations

- No loops (`for`, `while`)
- No `else` clause in conditionals
- No functions or procedures
- No operator precedence beyond single binary operations
- Single global scope only
- No optimizations performed

These limitations are intentional to keep the compiler simple and educational.

---

## Future Improvements

- Add `else` support
- Introduce loops
- Implement operator precedence
- Add basic optimizations (constant folding)
- Support additional data types
- Improve error messages with line numbers
- Add register allocation strategies

---

## Conclusion

This project demonstrates a complete working compiler for a high-level language targeting an 8-bit CPU.
By implementing every stage—from lexing to code generation—it provides practical insight into how programming
languages are translated into executable machine instructions.

The design prioritizes clarity, correctness, and educational value, making it an excellent foundation for
further exploration into compiler design and computer architecture.
