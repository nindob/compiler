cmake_minimum_required(VERSION 3.10)
project(compiler)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add source files
add_executable(compiler
    src/main.cpp
    src/lexer/lexer.cpp
    src/lexer/token.cpp
    src/parser/parser.cpp
    src/codegen/codegen.cpp
)

# Add include directories
target_include_directories(compiler PRIVATE src)
