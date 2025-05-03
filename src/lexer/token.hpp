#pragma once
#include <string>
#include <sstream>

enum class TokenType {
    // Keywords
    RETURN,
    PRINT,
    IF,
    ELSE,
    WHILE,
    FOR,
    INT,
    CHAR,
    SHORT,
    LONG,
    UNSIGNED,
    FLOAT,
    DOUBLE,
    BOOL,
    
    // Literals
    NUMBER,
    IDENTIFIER,
    
    // Operators
    PLUS,          // +
    MINUS,         // -
    MULTIPLY,      // *
    DIVIDE,        // /
    MODULO,        // %
    ASSIGN,        // =
    EQUAL,         // ==
    NOT_EQUAL,     // !=
    LESS,          // <
    GREATER,       // >
    LESS_EQUAL,    // <=
    GREATER_EQUAL, // >=
    LOGICAL_AND,   // &&
    LOGICAL_OR,    // ||
    LOGICAL_NOT,   // !
    BITWISE_NOT,   // ~
    
    // Punctuation
    SEMICOLON,     // ;
    COMMA,         // ,
    LEFT_PAREN,    // (
    RIGHT_PAREN,   // )
    LEFT_BRACE,    // {
    RIGHT_BRACE,   // }
    
    // Special
    END_OF_FILE,
    INVALID
};

struct Token {
    TokenType type;
    std::string value;
    int line;
    int column;
    
    Token(TokenType t, std::string v = "", int l = 0, int c = 0)
        : type(t), value(std::move(v)), line(l), column(c) {}
        
    std::string toString() const {
        std::stringstream ss;
        ss << "Token(type=" << static_cast<int>(type) 
           << ", value='" << value << "'"
           << ", line=" << line
           << ", col=" << column << ")";
        return ss.str();
    }
};
