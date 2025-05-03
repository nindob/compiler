#include "parser.hpp"
#include "../codegen/codegen.hpp"
#include <sstream>

Parser::Parser(const std::vector<Token>& tokens) : tokens(tokens) {}

std::shared_ptr<ASTNode> Parser::parse() {
    try {
        return binexpr(0);
    } catch (const ParseError& error) {
        // In a real compiler, we'd want better error handling
        throw;
    }
}

void Parser::parseStatements(CodeGenerator& codegen) {
    while (!isAtEnd()) {
        statement(codegen);
    }
}

void Parser::statement(CodeGenerator& codegen) {
    if (match(TokenType::PRINT)) {
        auto expr = binexpr(0);
        codegen.cgprintint(codegen.genAST(expr));
        codegen.freeall_registers();
        consume(TokenType::SEMICOLON, "Expected ';' after print statement");
    } else if (check(TokenType::INT) && check_ahead(TokenType::IDENTIFIER, 1) && check_ahead(TokenType::LEFT_PAREN, 2)) {
        // Look ahead to see if this is a prototype or definition
        size_t save = current;
        advance(); // INT
        advance(); // IDENTIFIER
        advance(); // LEFT_PAREN
        // Skip parameter list
        int paren_depth = 1;
        while (!isAtEnd() && paren_depth > 0) {
            if (peek().type == TokenType::LEFT_PAREN) paren_depth++;
            else if (peek().type == TokenType::RIGHT_PAREN) paren_depth--;
            advance();
        }
        // Now check for '{' or ';'
        if (check(TokenType::LEFT_BRACE)) {
            current = save;
            auto funcDef = function_definition();
            codegen.genAST(funcDef);
            codegen.freeall_registers();
        } else if (check(TokenType::SEMICOLON)) {
            current = save;
            auto funcProto = function_prototype();
            codegen.genAST(funcProto);
            codegen.freeall_registers();
        } else {
            throw ParseError("Expected '{' or ';' after function parameter list");
        }
    } else if (check(TokenType::IDENTIFIER) && check_ahead(TokenType::LEFT_PAREN, 1)) {
        // Function call
        auto funcCall = function_call();
        codegen.genAST(funcCall);
        codegen.freeall_registers();
        consume(TokenType::SEMICOLON, "Expected ';' after function call");
    } else if (match(TokenType::RETURN)) {
        auto retStmt = return_statement();
        codegen.genAST(retStmt);
        codegen.freeall_registers();
        consume(TokenType::SEMICOLON, "Expected ';' after return statement");
    } else if (check(TokenType::INT) || check(TokenType::CHAR) || check(TokenType::SHORT) || 
               check(TokenType::LONG) || check(TokenType::UNSIGNED) || check(TokenType::FLOAT) || 
               check(TokenType::DOUBLE) || check(TokenType::BOOL)) {
        var_declaration(&codegen);
    } else if (check(TokenType::IDENTIFIER)) {
        assignment_statement(&codegen);
    } else if (match(TokenType::IF)) {
        auto ifNode = if_statement();
        codegen.genAST(ifNode);
        codegen.freeall_registers();
    } else if (check(TokenType::WHILE)) {
        auto whileNode = while_statement();
        codegen.genAST(whileNode);
        codegen.freeall_registers();
    } else if (check(TokenType::FOR)) {
        auto forNode = for_statement();
        codegen.genAST(forNode);
        codegen.freeall_registers();
    } else {
        throw ParseError("Expected type, 'print', 'if', 'while', 'for', 'return', function, or identifier at start of statement");
    }
}

std::shared_ptr<ASTNode> Parser::var_declaration(CodeGenerator* codegen) {
    Token typeTok = advance();
    TypeKind type = tokenTypeToTypeKind(typeTok.type);
    Token id = consume(TokenType::IDENTIFIER, "Expected identifier after type");
    globals[id.value] = type;
    
    if (codegen) {
        codegen->emitGlobalVar(id.value, type);
    }
    
    consume(TokenType::SEMICOLON, "Expected ';' after variable declaration");
    return std::make_shared<VarDeclNode>(id.value, type);
}

std::shared_ptr<ASTNode> Parser::assignment_statement(CodeGenerator* codegen, bool consume_semicolon) {
    Token id = consume(TokenType::IDENTIFIER, "Expected identifier for assignment");
    auto it = globals.find(id.value);
    if (it == globals.end()) {
        throw ParseError("Undeclared variable: " + id.value);
    }
    TypeKind type = it->second;
    consume(TokenType::ASSIGN, "Expected '=' in assignment");
    auto expr = binexpr(0);
    
    if (codegen) {
        codegen->genAST(std::make_shared<AssignNode>(id.value, expr, type));
        codegen->freeall_registers();
    }
    
    if (consume_semicolon) {
        consume(TokenType::SEMICOLON, "Expected ';' after assignment");
    }
    
    return std::make_shared<AssignNode>(id.value, expr, type);
}

TypeKind Parser::tokenTypeToTypeKind(TokenType type) const {
    switch (type) {
        case TokenType::INT: return TypeKind::Int;
        case TokenType::CHAR: return TypeKind::Char;
        case TokenType::SHORT: return TypeKind::Short;
        case TokenType::LONG: return TypeKind::Long;
        case TokenType::UNSIGNED: return TypeKind::Unsigned;
        case TokenType::FLOAT: return TypeKind::Float;
        case TokenType::DOUBLE: return TypeKind::Double;
        case TokenType::BOOL: return TypeKind::Bool;
        default: throw ParseError("Unknown type keyword");
    }
}

// Operator precedence table (higher = tighter binding)
int Parser::get_precedence(TokenType type) const {
    switch (type) {
        case TokenType::LOGICAL_OR: return 1;
        case TokenType::LOGICAL_AND: return 2;
        case TokenType::EQUAL:
        case TokenType::NOT_EQUAL: return 3;
        case TokenType::LESS:
        case TokenType::GREATER:
        case TokenType::LESS_EQUAL:
        case TokenType::GREATER_EQUAL: return 4;
        case TokenType::PLUS:
        case TokenType::MINUS: return 5;
        case TokenType::MULTIPLY:
        case TokenType::DIVIDE:
        case TokenType::MODULO: return 6;
        default: return 0;
    }
}

// Pratt parser (precedence climbing) with unary support
std::shared_ptr<ASTNode> Parser::binexpr(int ptp) {
    std::shared_ptr<ASTNode> left;
    // Handle unary prefix
    if (match(TokenType::MINUS)) {
        left = std::make_shared<UnaryOpNode>(ASTNodeType::NEGATE, binexpr(100));
    } else if (match(TokenType::LOGICAL_NOT)) {
        left = std::make_shared<UnaryOpNode>(ASTNodeType::LOGICAL_NOT, binexpr(100));
    } else if (match(TokenType::BITWISE_NOT)) {
        left = std::make_shared<UnaryOpNode>(ASTNodeType::BITWISE_NOT, binexpr(100));
    } else if (match(TokenType::NUMBER)) {
        int value = std::stoi(tokens[current - 1].value);
        left = std::make_shared<NumberNode>(value);
    } else if (match(TokenType::IDENTIFIER)) {
        std::string name = tokens[current - 1].value;
        if (check(TokenType::LEFT_PAREN)) {
            // Function call
            advance(); // consume '('
            std::vector<std::shared_ptr<ASTNode>> args;
            if (!check(TokenType::RIGHT_PAREN)) {
                do {
                    args.push_back(binexpr(0));
                } while (match(TokenType::COMMA));
            }
            consume(TokenType::RIGHT_PAREN, "Expected ')' after argument list");
            left = std::make_shared<FunctionCallNode>(name, args);
        } else {
            // Check locals first
            int offset = -1;
            bool is_local = false;
            TypeKind type = TypeKind::Int;
            for (auto it = locals.rbegin(); it != locals.rend(); ++it) {
                auto found = it->find(name);
                if (found != it->end()) {
                    offset = found->second;
                    is_local = true;
                    break;
                }
            }
            if (!is_local) {
                auto it = globals.find(name);
                if (it == globals.end()) {
                    throw ParseError("Undeclared variable: " + name);
                }
                type = it->second;
            }
            left = std::make_shared<VarNode>(name, type, is_local, offset);
        }
    } else if (match(TokenType::LEFT_PAREN)) {
        left = binexpr(0);
        consume(TokenType::RIGHT_PAREN, "Expect ')' after expression.");
    } else {
        throw ParseError("Expected number, variable, or '(' in expression");
    }

    while (true) {
        TokenType opType = peek().type;
        int prec = get_precedence(opType);
        if (prec == 0 || prec <= ptp)
            break;
        advance(); // consume operator
        auto right = binexpr(prec);
        ASTNodeType astOp;
        switch (opType) {
            case TokenType::PLUS: astOp = ASTNodeType::ADD; break;
            case TokenType::MINUS: astOp = ASTNodeType::SUBTRACT; break;
            case TokenType::MULTIPLY: astOp = ASTNodeType::MULTIPLY; break;
            case TokenType::DIVIDE: astOp = ASTNodeType::DIVIDE; break;
            case TokenType::MODULO: astOp = ASTNodeType::MODULO; break;
            case TokenType::EQUAL: astOp = ASTNodeType::EQ; break;
            case TokenType::NOT_EQUAL: astOp = ASTNodeType::NE; break;
            case TokenType::LESS: astOp = ASTNodeType::LT; break;
            case TokenType::GREATER: astOp = ASTNodeType::GT; break;
            case TokenType::LESS_EQUAL: astOp = ASTNodeType::LE; break;
            case TokenType::GREATER_EQUAL: astOp = ASTNodeType::GE; break;
            case TokenType::LOGICAL_AND: astOp = ASTNodeType::LOGICAL_AND; break;
            case TokenType::LOGICAL_OR: astOp = ASTNodeType::LOGICAL_OR; break;
            default: throw ParseError("Unknown operator");
        }
        left = std::make_shared<BinaryOpNode>(astOp, left, right);
    }
    return left;
}

// Helper methods
Token Parser::peek() const {
    if (isAtEnd()) return tokens.back(); // END_OF_FILE token
    return tokens[current];
}

Token Parser::advance() {
    if (!isAtEnd()) current++;
    return tokens[current - 1];
}

bool Parser::isAtEnd() const {
    return current >= tokens.size() || 
           tokens[current].type == TokenType::END_OF_FILE;
}

bool Parser::match(TokenType type) {
    if (check(type)) {
        advance();
        return true;
    }
    return false;
}

bool Parser::check(TokenType type) const {
    if (isAtEnd()) return false;
    return peek().type == type;
}

bool Parser::check_ahead(TokenType type, int offset) const {
    if (current + offset >= tokens.size()) return false;
    return tokens[current + offset].type == type;
}

Token Parser::consume(TokenType type, const std::string& message) {
    if (check(type)) return advance();
    throw ParseError(message);
}

std::shared_ptr<ASTNode> Parser::compound_statement() {
    consume(TokenType::LEFT_BRACE, "Expected '{' at start of compound statement");
    // Enter new local scope
    locals.push_back({});
    int old_offset = local_offset;
    
    std::shared_ptr<ASTNode> left = nullptr;
    
    while (!check(TokenType::RIGHT_BRACE) && !isAtEnd()) {
        std::shared_ptr<ASTNode> stmt;
        
        if (match(TokenType::PRINT)) {
            auto expr = binexpr(0);
            stmt = expr;  // The print operation will be handled by the parent
            consume(TokenType::SEMICOLON, "Expected ';' after print statement");
        } else if (match(TokenType::RETURN)) {
            stmt = return_statement();
            consume(TokenType::SEMICOLON, "Expected ';' after return statement");
        } else if (match(TokenType::IF)) {
            stmt = if_statement();
        } else if (check(TokenType::LEFT_BRACE)) {
            stmt = compound_statement();
        } else if (check(TokenType::IDENTIFIER)) {
            Token id = consume(TokenType::IDENTIFIER, "Expected identifier");
            // Check locals first
            int offset = -1;
            bool is_local = false;
            TypeKind type = TypeKind::Int;
            for (auto it = locals.rbegin(); it != locals.rend(); ++it) {
                auto found = it->find(id.value);
                if (found != it->end()) {
                    offset = found->second;
                    is_local = true;
                    break;
                }
            }
            if (!is_local) {
                auto it = globals.find(id.value);
                if (it == globals.end()) {
                    throw ParseError("Undeclared variable: " + id.value);
                }
                type = it->second;
            }
            consume(TokenType::ASSIGN, "Expected '=' in assignment");
            auto expr = binexpr(0);
            stmt = std::make_shared<AssignNode>(id.value, expr, type, is_local, offset);
            consume(TokenType::SEMICOLON, "Expected ';' after assignment");
        } else if (check(TokenType::INT) || check(TokenType::CHAR) || check(TokenType::SHORT) || 
                  check(TokenType::LONG) || check(TokenType::UNSIGNED) || check(TokenType::FLOAT) || 
                  check(TokenType::DOUBLE) || check(TokenType::BOOL)) {
            Token typeTok = advance();
            TypeKind type = tokenTypeToTypeKind(typeTok.type);
            Token id = consume(TokenType::IDENTIFIER, "Expected identifier after type");
            // Local variable: assign stack offset
            local_offset -= 8;
            locals.back()[id.value] = local_offset;
            stmt = std::make_shared<VarDeclNode>(id.value, type, true, local_offset);
            consume(TokenType::SEMICOLON, "Expected ';' after variable declaration");
        } else {
            throw ParseError("Expected statement");
        }
        
        if (stmt) {
            if (left == nullptr) {
                left = stmt;
            } else {
                left = std::make_shared<GlueNode>(left, stmt);
            }
        }
    }
    
    consume(TokenType::RIGHT_BRACE, "Expected '}' at end of compound statement");
    // Leave local scope
    locals.pop_back();
    local_offset = old_offset;
    return left;
}

std::shared_ptr<ASTNode> Parser::if_statement() {
    // We've already matched the IF token in the calling function
    consume(TokenType::LEFT_PAREN, "Expected '(' after 'if'");
    
    auto condition = binexpr(0);
    
    // Ensure the condition is a comparison or logical operation
    if (condition->getType() != ASTNodeType::EQ && condition->getType() != ASTNodeType::NE &&
        condition->getType() != ASTNodeType::LT && condition->getType() != ASTNodeType::GT &&
        condition->getType() != ASTNodeType::LE && condition->getType() != ASTNodeType::GE &&
        condition->getType() != ASTNodeType::LOGICAL_AND && condition->getType() != ASTNodeType::LOGICAL_OR &&
        condition->getType() != ASTNodeType::LOGICAL_NOT) {
        throw ParseError("Expected comparison operator in if condition");
    }
    
    consume(TokenType::RIGHT_PAREN, "Expected ')' after if condition");
    
    auto thenBranch = compound_statement();
    
    std::shared_ptr<ASTNode> elseBranch = nullptr;
    if (match(TokenType::ELSE)) {
        elseBranch = compound_statement();
    }
    
    return std::make_shared<IfNode>(condition, thenBranch, elseBranch);
}

std::shared_ptr<ASTNode> Parser::while_statement() {
    // Ensure we have 'while' '('
    consume(TokenType::WHILE, "Expected 'while'");
    consume(TokenType::LEFT_PAREN, "Expected '(' after 'while'");
    
    // Parse the condition expression
    auto cond = binexpr(0);
    if (cond->getType() < ASTNodeType::EQ || cond->getType() > ASTNodeType::GE) {
        throw ParseError("Bad comparison operator in while condition");
    }
    
    consume(TokenType::RIGHT_PAREN, "Expected ')' after while condition");
    
    // Parse the body
    auto body = compound_statement();
    
    // Build and return the AST for this statement
    return std::make_shared<WhileNode>(cond, body);
}

std::shared_ptr<ASTNode> Parser::for_statement() {
    // Ensure we have 'for' '('
    consume(TokenType::FOR, "Expected 'for'");
    consume(TokenType::LEFT_PAREN, "Expected '(' after 'for'");
    
    // Parse the initialization statement
    std::shared_ptr<ASTNode> init = nullptr;
    if (check(TokenType::INT)) {
        init = var_declaration(nullptr);
    } else if (check(TokenType::IDENTIFIER)) {
        init = assignment_statement(nullptr, false);
    }
    consume(TokenType::SEMICOLON, "Expected ';' after for initialization");
    
    // Parse the condition
    auto cond = binexpr(0);
    if (cond->getType() < ASTNodeType::EQ || cond->getType() > ASTNodeType::GE) {
        throw ParseError("Bad comparison operator in for condition");
    }
    consume(TokenType::SEMICOLON, "Expected ';' after for condition");
    
    // Parse the increment statement
    std::shared_ptr<ASTNode> incr = nullptr;
    if (check(TokenType::IDENTIFIER)) {
        incr = assignment_statement(nullptr, false);
    }
    consume(TokenType::RIGHT_PAREN, "Expected ')' after for increment");
    
    // Parse the body
    auto body = compound_statement();
    
    // Build the AST structure:
    // GLUE(init, WHILE(cond, GLUE(body, incr)))
    auto bodyWithIncr = std::make_shared<GlueNode>(body, incr);
    auto whileNode = std::make_shared<WhileNode>(cond, bodyWithIncr);
    return std::make_shared<GlueNode>(init, whileNode);
}

std::shared_ptr<ASTNode> Parser::function_definition() {
    // int foo(int a, int b) { ... }
    consume(TokenType::INT, "Expected return type for function");
    Token nameTok = consume(TokenType::IDENTIFIER, "Expected function name");
    consume(TokenType::LEFT_PAREN, "Expected '(' after function name");
    std::vector<std::string> params;
    if (!check(TokenType::RIGHT_PAREN)) {
        do {
            consume(TokenType::INT, "Expected parameter type");
            Token paramName = consume(TokenType::IDENTIFIER, "Expected parameter name");
            params.push_back(paramName.value);
        } while (match(TokenType::COMMA));
    }
    consume(TokenType::RIGHT_PAREN, "Expected ')' after parameter list");
    // Add parameters to globals
    std::vector<std::string> to_remove;
    for (const auto& param : params) {
        if (globals.find(param) == globals.end()) {
            globals[param] = TypeKind::Int;
            to_remove.push_back(param);
        }
    }
    auto body = compound_statement();
    // Remove parameters from globals
    for (const auto& param : to_remove) {
        globals.erase(param);
    }
    return std::make_shared<FunctionDeclNode>(nameTok.value, params, body);
}

std::shared_ptr<ASTNode> Parser::function_call() {
    // foo(1, 2)
    Token nameTok = consume(TokenType::IDENTIFIER, "Expected function name");
    consume(TokenType::LEFT_PAREN, "Expected '(' after function name");
    std::vector<std::shared_ptr<ASTNode>> args;
    if (!check(TokenType::RIGHT_PAREN)) {
        do {
            args.push_back(binexpr(0));
        } while (match(TokenType::COMMA));
    }
    consume(TokenType::RIGHT_PAREN, "Expected ')' after argument list");
    return std::make_shared<FunctionCallNode>(nameTok.value, args);
}

std::shared_ptr<ASTNode> Parser::function_prototype() {
    // int foo(int a, int b);
    consume(TokenType::INT, "Expected return type for function prototype");
    Token nameTok = consume(TokenType::IDENTIFIER, "Expected function name");
    consume(TokenType::LEFT_PAREN, "Expected '(' after function name");
    std::vector<std::string> params;
    if (!check(TokenType::RIGHT_PAREN)) {
        do {
            consume(TokenType::INT, "Expected parameter type");
            Token paramName = consume(TokenType::IDENTIFIER, "Expected parameter name");
            params.push_back(paramName.value);
        } while (match(TokenType::COMMA));
    }
    consume(TokenType::RIGHT_PAREN, "Expected ')' after parameter list");
    consume(TokenType::SEMICOLON, "Expected ';' after function prototype");
    return std::make_shared<FunctionProtoNode>(nameTok.value, params);
}

std::shared_ptr<ASTNode> Parser::return_statement() {
    // return expr;
    auto expr = binexpr(0);
    return std::make_shared<ReturnNode>(expr);
}
