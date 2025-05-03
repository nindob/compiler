#include "lexer.hpp"
#include <cctype>
#include <unordered_map>

namespace {
    // Keywords lookup table
    const std::unordered_map<std::string, TokenType> keywords = {
        {"return", TokenType::RETURN},
        {"print", TokenType::PRINT},
        {"if", TokenType::IF},
        {"else", TokenType::ELSE},
        {"while", TokenType::WHILE},
        {"for", TokenType::FOR},
        {"int", TokenType::INT},
        {"char", TokenType::CHAR},
        {"short", TokenType::SHORT},
        {"long", TokenType::LONG},
        {"unsigned", TokenType::UNSIGNED},
        {"float", TokenType::FLOAT},
        {"double", TokenType::DOUBLE},
        {"bool", TokenType::BOOL}
    };
}

Lexer::Lexer(const std::string& source) : source(source) {}

char Lexer::peek() {
    if (position >= source.length()) return '\0';
    return source[position];
}

char Lexer::advance() {
    if (position >= source.length()) return '\0';
    char current = source[position++];
    column++;
    lastChar = current;
    return current;
}

void Lexer::putback() {
    if (position > 0) {
        position--;
        column--;
    }
}

bool Lexer::match(char expected) {
    if (isAtEnd() || peek() != expected) return false;
    advance();
    return true;
}

void Lexer::skipWhitespace() {
    while (true) {
        char c = peek();
        switch (c) {
            case ' ':
            case '\r':
            case '\t':
                advance();
                break;
            case '\n':
                newLine();
                advance();
                break;
            case '/':
                if (position + 1 < source.length() && source[position + 1] == '/') {
                    skipComment();
                } else {
                    return;
                }
                break;
            default:
                return;
        }
    }
}

void Lexer::skipComment() {
    // Skip until end of line
    while (peek() != '\n' && !isAtEnd()) {
        advance();
    }
}

Token Lexer::number() {
    std::string num;
    int startColumn = currentColumn();
    
    while (isDigit(peek())) {
        num += advance();
    }
    
    // Handle decimal point
    if (peek() == '.' && isDigit(source[position + 1])) {
        num += advance(); // consume '.'
        while (isDigit(peek())) {
            num += advance();
        }
    }
    
    return Token(TokenType::NUMBER, num, currentLine(), startColumn);
}

Token Lexer::identifier() {
    std::string id;
    int startColumn = currentColumn();
    
    while (isAlphaNumeric(peek())) {
        id += advance();
    }
    
    // Check if it's a keyword
    auto it = keywords.find(id);
    if (it != keywords.end()) {
        return Token(it->second, id, currentLine(), startColumn);
    }
    
    return Token(TokenType::IDENTIFIER, id, currentLine(), startColumn);
}

Token Lexer::handleOperator() {
    int startColumn = currentColumn();
    char c = advance();
    
    switch (c) {
        case '+': return makeToken(TokenType::PLUS, "+");
        case '-': return makeToken(TokenType::MINUS, "-");
        case '*': return makeToken(TokenType::MULTIPLY, "*");
        case '/': return makeToken(TokenType::DIVIDE, "/");
        case '%': return makeToken(TokenType::MODULO, "%");
        case '(': return makeToken(TokenType::LEFT_PAREN, "(");
        case ')': return makeToken(TokenType::RIGHT_PAREN, ")");
        case '{': return makeToken(TokenType::LEFT_BRACE, "{");
        case '}': return makeToken(TokenType::RIGHT_BRACE, "}");
        case ';': return makeToken(TokenType::SEMICOLON, ";");
        case ',': return makeToken(TokenType::COMMA, ",");
        
        case '=':
            if (match('=')) return makeToken(TokenType::EQUAL, "==");
            return makeToken(TokenType::ASSIGN, "=");
        case '!':
            if (match('=')) return makeToken(TokenType::NOT_EQUAL, "!=");
            return makeToken(TokenType::INVALID, std::string(1, c));
        case '<':
            if (match('=')) return makeToken(TokenType::LESS_EQUAL, "<=");
            return makeToken(TokenType::LESS, "<");
        case '>':
            if (match('=')) return makeToken(TokenType::GREATER_EQUAL, ">=");
            return makeToken(TokenType::GREATER, ">");
            
        default:
            return makeToken(TokenType::INVALID, std::string(1, c));
    }
}

Token Lexer::nextToken() {
    skipWhitespace();
    
    if (isAtEnd()) {
        return makeToken(TokenType::END_OF_FILE);
    }
    
    char c = peek();
    
    // Handle numbers
    if (isDigit(c)) {
        return number();
    }
    
    // Handle identifiers and keywords
    if (isAlpha(c)) {
        return identifier();
    }
    
    // Handle operators and punctuation
    switch (c) {
        case ';': advance(); return makeToken(TokenType::SEMICOLON, ";");
        case '=':
            advance();
            if (peek() == '=') { advance(); return makeToken(TokenType::EQUAL, "=="); }
            return makeToken(TokenType::ASSIGN, "=");
        case '!':
            advance();
            if (peek() == '=') { advance(); return makeToken(TokenType::NOT_EQUAL, "!="); }
            return makeToken(TokenType::LOGICAL_NOT, "!");
        case '<':
            advance();
            if (peek() == '=') { advance(); return makeToken(TokenType::LESS_EQUAL, "<="); }
            return makeToken(TokenType::LESS, "<");
        case '>':
            advance();
            if (peek() == '=') { advance(); return makeToken(TokenType::GREATER_EQUAL, ">="); }
            return makeToken(TokenType::GREATER, ">" );
        case '&':
            advance();
            if (peek() == '&') { advance(); return makeToken(TokenType::LOGICAL_AND, "&&"); }
            break;
        case '|':
            advance();
            if (peek() == '|') { advance(); return makeToken(TokenType::LOGICAL_OR, "||"); }
            break;
        case '~': advance(); return makeToken(TokenType::BITWISE_NOT, "~");
        default: return handleOperator();
    }
    
    return makeToken(TokenType::INVALID);
}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;
    
    while (true) {
        Token token = nextToken();
        tokens.push_back(token);
        
        if (token.type == TokenType::END_OF_FILE) {
            break;
        }
    }
    
    return tokens;
}

// Helper methods
bool Lexer::isAtEnd() const {
    return position >= source.length();
}

void Lexer::newLine() {
    line++;
    column = 0;
}

int Lexer::currentLine() const {
    return line;
}

int Lexer::currentColumn() const {
    return column;
}

bool Lexer::isAlpha(char c) {
    return (c >= 'a' && c <= 'z') ||
           (c >= 'A' && c <= 'Z') ||
            c == '_';
}

bool Lexer::isDigit(char c) {
    return c >= '0' && c <= '9';
}

bool Lexer::isAlphaNumeric(char c) {
    return isAlpha(c) || isDigit(c);
}

Token Lexer::makeToken(TokenType type, std::string value) {
    return Token(type, value, currentLine(), currentColumn());
}
