#pragma once
#include <vector>
#include <memory>
#include <stdexcept>
#include <unordered_map>
#include "../lexer/token.hpp"
#include "../ast/ast.hpp"

class Parser {
public:
    explicit Parser(const std::vector<Token>& tokens);
    
    // Parse the entire input
    std::shared_ptr<ASTNode> parse();
    
    // Parse statements
    void parseStatements(class CodeGenerator& codegen);
    
    // Symbol table for global variables: name -> type
    std::unordered_map<std::string, TypeKind> globals;
    
private:
    const std::vector<Token>& tokens;
    size_t current = 0;  // Current token index
    // Local variable scopes: stack of name->offset
    std::vector<std::unordered_map<std::string, int>> locals;
    int local_offset = 0;
    
    // Pratt parser
    std::shared_ptr<ASTNode> binexpr(int ptp);
    int get_precedence(TokenType type) const;
    
    // Statement parsing
    void statement(class CodeGenerator& codegen);
    std::shared_ptr<ASTNode> var_declaration(class CodeGenerator* codegen = nullptr);
    std::shared_ptr<ASTNode> assignment_statement(class CodeGenerator* codegen = nullptr, bool consume_semicolon = true);
    std::shared_ptr<ASTNode> if_statement();
    std::shared_ptr<ASTNode> while_statement();
    std::shared_ptr<ASTNode> for_statement();
    std::shared_ptr<ASTNode> compound_statement();
    // Function support
    std::shared_ptr<ASTNode> function_definition();
    std::shared_ptr<ASTNode> function_call();
    std::shared_ptr<ASTNode> function_prototype();
    std::shared_ptr<ASTNode> return_statement();
    
    // Helper methods
    Token peek() const;
    Token advance();
    bool isAtEnd() const;
    bool match(TokenType type);
    bool check(TokenType type) const;
    bool check_ahead(TokenType type, int offset) const;
    Token consume(TokenType type, const std::string& message);
    TypeKind tokenTypeToTypeKind(TokenType type) const;
    
    class ParseError : public std::runtime_error {
    public:
        explicit ParseError(const std::string& message) 
            : std::runtime_error(message) {}
    };
}; 