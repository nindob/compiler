#pragma once
#include <string>
#include <vector>
#include <memory>
#include "token.hpp"

class Lexer {
public:
    explicit Lexer(const std::string& source);
    std::vector<Token> tokenize();
    Token nextToken();  // Get next token

private:
    std::string source;
    size_t position = 0;
    size_t line = 1;      // Current line number
    size_t column = 1;    // Current column number
    char lastChar = ' ';  // Last character read
    
    // Character handling
    char peek();          // Look at current character without consuming
    char advance();       // Get current character and advance
    bool match(char expected);  // Check if current char matches and advance if true
    void putback();      // Put back last character read
    
    // Utility functions
    void skipWhitespace();
    void skipComment();  // Skip single-line comments
    bool isAtEnd() const;
    
    // Token recognition
    Token number();      // Handle numeric literals
    Token identifier(); // Handle identifiers and keywords
    Token handleOperator(); // Handle operators and punctuation
    
    // Position tracking
    void newLine();     // Handle newline
    int currentLine() const;
    int currentColumn() const;
    
    // Helpers
    static bool isAlpha(char c);
    static bool isAlphaNumeric(char c);
    static bool isDigit(char c);
    Token makeToken(TokenType type, std::string value = "");
}; 