# C-like Compiler

A simple C-like compiler written in C++ I am now sharing that supports:

- **Global and local variables**
- **Arithmetic expressions** (`+`, `-`, `*`, `/`, `%`)
- **Comparison and logical operators** (`==`, `!=`, `<`, `>`, `<=`, `>=`, `&&`, `||`, `!`, `~`)
- **Variable assignment and declaration** (`int x; x = 5;`)
- **If/else statements**
- **While and for loops**
- **Function definitions, calls, and parameters**
- **Forward declarations (prototypes)**
- **Return statements**
- **Block scoping and variable shadowing**
- **Print statement for output**

## Features

### 1. **Variables**
- Supports both global and local variables.
- Local variables are allocated on the stack and support shadowing.

### 2. **Expressions**
- Arithmetic: `+`, `-`, `*`, `/`, `%`
- Comparison: `==`, `!=`, `<`, `>`, `<=`, `>=`
- Logical: `&&`, `||`, `!`, `~`

### 3. **Statements**
- Variable declaration: `int x;`
- Assignment: `x = 5;`
- Print: `print x;`
- If/else: `if (x < 5) { ... } else { ... }`
- While: `while (x < 10) { ... }`
- For: `for (i = 0; i < 10; i = i + 1) { ... }`
- Return: `return expr;`
- Compound/nested blocks: `{ ... }`

### 4. **Functions**
- Define functions with parameters: `int add(int a, int b) { ... }`
- Call functions: `add(2, 3);`
- Forward declarations: `int add(int a, int b);`
- Return values from functions.

### 5. **Scoping**
- Local variables are block-scoped.
- Variable shadowing is supported.

## Example

```c
int add(int a, int b) {
    return a + b;
}

int main() {
    int x;
    x = add(2, 3);
    print x; // 5

    {
        int x;
        x = 99;
        print x; // 99
    }

    print x; // 5
}
```

## How It Works

- **Lexical Analysis:** Tokenizes the input source code.
- **Parsing:** Builds an Abstract Syntax Tree (AST) using recursive descent and Pratt parsing.
- **Code Generation:** Outputs x86-64 assembly code.
- **Assembly:** The generated `.s` file can be assembled and linked with `cc` or `clang`.

## Building and Running

```sh
# Build the compiler
make clean && mkdir -p build && cd build && cmake .. && make && cd ..

# Compile a test program
./build/compiler tests/input11

# Assemble and run the output (on macOS/Linux)
cc -no-pie -o out tests/input11.s -lc
./out
```

## Directory Structure

```
compiler/
├── src/
│   ├── ast/         # AST node definitions
│   ├── codegen/     # Code generation
│   ├── lexer/       # Lexer and token definitions
│   └── parser/      # Parser
├── tests/           # Test programs
├── build/           # Build output (ignored)
├── CMakeLists.txt   # Build configuration
├── Makefile         # Build script
└── README.md        # This file
```

## Limitations & Future Work

- Only supports `int` type for now.
- No arrays, pointers, or structs.
- No function overloading or recursion (yet).
- No error recovery in parser.
- No semantic analysis (type checking, etc).

## Authors

- ani

---

**Enjoy hacking on your own C-like compiler!**
