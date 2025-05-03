#pragma once
#include <memory>
#include <string>
#include <vector>

// Supported types
enum class TypeKind {
    Int,
    Char,
    Short,
    Long,
    Unsigned,
    Float,
    Double,
    Bool
};

// AST node types
enum class ASTNodeType {
    NUMBER,     // Integer literal
    ADD,        // +
    SUBTRACT,   // -
    MULTIPLY,   // *
    DIVIDE,     // /
    MODULO,     // %
    EQ,         // ==
    NE,         // !=
    LT,         // <
    GT,         // >
    LE,         // <=
    GE,         // >=
    LOGICAL_AND,// &&
    LOGICAL_OR, // ||
    LOGICAL_NOT,// !
    BITWISE_NOT,// ~
    NEGATE,     // unary -
    VAR_DECL,   // int x;
    ASSIGN,     // x = expr;
    VAR,        // variable reference
    IF,         // if statement
    WHILE,      // while statement
    FOR,        // for statement
    GLUE,       // glue multiple statements
    FUNCTION_DECL, // Function declaration
    FUNCTION_CALL, // Function call
    FUNCTION_PROTO, // Function prototype (forward declaration)
    RETURN        // Return statement
};

// Abstract base class for AST nodes
class ASTNode {
public:
    virtual ~ASTNode() = default;
    virtual int evaluate() const = 0;  // For interpreter
    virtual ASTNodeType getType() const = 0;
};

// Number literal node
class NumberNode : public ASTNode {
public:
    explicit NumberNode(int value) : value(value) {}
    int evaluate() const override { return value; }
    ASTNodeType getType() const override { return ASTNodeType::NUMBER; }
    int getValue() const { return value; }

private:
    int value;
};

// Unary operation node
class UnaryOpNode : public ASTNode {
public:
    UnaryOpNode(ASTNodeType op, std::shared_ptr<ASTNode> child)
        : opType(op), child(child) {}
    int evaluate() const override {
        int v = child->evaluate();
        switch (opType) {
            case ASTNodeType::NEGATE: return -v;
            case ASTNodeType::BITWISE_NOT: return ~v;
            case ASTNodeType::LOGICAL_NOT: return !v;
            default: return 0;
        }
    }
    ASTNodeType getType() const override { return opType; }
    std::shared_ptr<ASTNode> getChild() const { return child; }
private:
    ASTNodeType opType;
    std::shared_ptr<ASTNode> child;
};

// Variable reference node
class VarNode : public ASTNode {
public:
    VarNode(const std::string& name, TypeKind type, bool is_local = false, int offset = 0)
        : name(name), type(type), is_local(is_local), offset(offset) {}
    int evaluate() const override { return 0; } // Not used in codegen
    ASTNodeType getType() const override { return ASTNodeType::VAR; }
    const std::string& getName() const { return name; }
    TypeKind getVarType() const { return type; }
    bool isLocal() const { return is_local; }
    int getOffset() const { return offset; }
private:
    std::string name;
    TypeKind type;
    bool is_local;
    int offset;
};

// Variable declaration node
class VarDeclNode : public ASTNode {
public:
    VarDeclNode(const std::string& name, TypeKind type, bool is_local = false, int offset = 0)
        : name(name), type(type), is_local(is_local), offset(offset) {}
    int evaluate() const override { return 0; }
    ASTNodeType getType() const override { return ASTNodeType::VAR_DECL; }
    const std::string& getName() const { return name; }
    TypeKind getVarType() const { return type; }
    bool isLocal() const { return is_local; }
    int getOffset() const { return offset; }
private:
    std::string name;
    TypeKind type;
    bool is_local;
    int offset;
};

// Assignment node
class AssignNode : public ASTNode {
public:
    AssignNode(const std::string& name, std::shared_ptr<ASTNode> expr, TypeKind type, bool is_local = false, int offset = 0)
        : name(name), expr(expr), type(type), is_local(is_local), offset(offset) {}
    int evaluate() const override { return 0; }
    ASTNodeType getType() const override { return ASTNodeType::ASSIGN; }
    const std::string& getName() const { return name; }
    std::shared_ptr<ASTNode> getExpr() const { return expr; }
    TypeKind getVarType() const { return type; }
    bool isLocal() const { return is_local; }
    int getOffset() const { return offset; }
private:
    std::string name;
    std::shared_ptr<ASTNode> expr;
    TypeKind type;
    bool is_local;
    int offset;
};

// Binary operation node
class BinaryOpNode : public ASTNode {
public:
    BinaryOpNode(ASTNodeType op, std::shared_ptr<ASTNode> left, std::shared_ptr<ASTNode> right)
        : opType(op), leftChild(left), rightChild(right) {}

    int evaluate() const override {
        int left = leftChild->evaluate();
        int right = rightChild->evaluate();
        
        switch (opType) {
            case ASTNodeType::ADD: return left + right;
            case ASTNodeType::SUBTRACT: return left - right;
            case ASTNodeType::MULTIPLY: return left * right;
            case ASTNodeType::DIVIDE: return left / right;
            case ASTNodeType::MODULO: return left % right;
            case ASTNodeType::EQ: return left == right;
            case ASTNodeType::NE: return left != right;
            case ASTNodeType::LT: return left < right;
            case ASTNodeType::GT: return left > right;
            case ASTNodeType::LE: return left <= right;
            case ASTNodeType::GE: return left >= right;
            case ASTNodeType::LOGICAL_AND: return left && right;
            case ASTNodeType::LOGICAL_OR: return left || right;
            default: return 0; // Should never happen
        }
    }

    ASTNodeType getType() const override { return opType; }
    std::shared_ptr<ASTNode> getLeft() const { return leftChild; }
    std::shared_ptr<ASTNode> getRight() const { return rightChild; }

private:
    ASTNodeType opType;
    std::shared_ptr<ASTNode> leftChild;
    std::shared_ptr<ASTNode> rightChild;
};

// Glue node to combine multiple statements
class GlueNode : public ASTNode {
public:
    GlueNode(std::shared_ptr<ASTNode> left, std::shared_ptr<ASTNode> right)
        : left(left), right(right) {}
    int evaluate() const override { return 0; }
    ASTNodeType getType() const override { return ASTNodeType::GLUE; }
    std::shared_ptr<ASTNode> getLeft() const { return left; }
    std::shared_ptr<ASTNode> getRight() const { return right; }
private:
    std::shared_ptr<ASTNode> left;
    std::shared_ptr<ASTNode> right;
};

// If statement node
class IfNode : public ASTNode {
public:
    IfNode(std::shared_ptr<ASTNode> condition, std::shared_ptr<ASTNode> thenBranch, std::shared_ptr<ASTNode> elseBranch = nullptr)
        : condition(condition), thenBranch(thenBranch), elseBranch(elseBranch) {}
    int evaluate() const override { return 0; }
    ASTNodeType getType() const override { return ASTNodeType::IF; }
    std::shared_ptr<ASTNode> getCondition() const { return condition; }
    std::shared_ptr<ASTNode> getThenBranch() const { return thenBranch; }
    std::shared_ptr<ASTNode> getElseBranch() const { return elseBranch; }
private:
    std::shared_ptr<ASTNode> condition;
    std::shared_ptr<ASTNode> thenBranch;
    std::shared_ptr<ASTNode> elseBranch;
};

// While statement node
class WhileNode : public ASTNode {
public:
    WhileNode(std::shared_ptr<ASTNode> condition, std::shared_ptr<ASTNode> body)
        : condition(condition), body(body) {}
    int evaluate() const override { return 0; }
    ASTNodeType getType() const override { return ASTNodeType::WHILE; }
    std::shared_ptr<ASTNode> getCondition() const { return condition; }
    std::shared_ptr<ASTNode> getBody() const { return body; }
private:
    std::shared_ptr<ASTNode> condition;
    std::shared_ptr<ASTNode> body;
};

// Function declaration node
class FunctionDeclNode : public ASTNode {
public:
    FunctionDeclNode(const std::string& name, const std::vector<std::string>& params, std::shared_ptr<ASTNode> body)
        : name(name), params(params), body(body) {}
    int evaluate() const override { return 0; }
    ASTNodeType getType() const override { return ASTNodeType::FUNCTION_DECL; }
    const std::string& getName() const { return name; }
    const std::vector<std::string>& getParams() const { return params; }
    std::shared_ptr<ASTNode> getBody() const { return body; }
private:
    std::string name;
    std::vector<std::string> params;
    std::shared_ptr<ASTNode> body;
};

// Function call node
class FunctionCallNode : public ASTNode {
public:
    FunctionCallNode(const std::string& name, const std::vector<std::shared_ptr<ASTNode>>& args)
        : name(name), args(args) {}
    int evaluate() const override { return 0; }
    ASTNodeType getType() const override { return ASTNodeType::FUNCTION_CALL; }
    const std::string& getName() const { return name; }
    const std::vector<std::shared_ptr<ASTNode>>& getArgs() const { return args; }
private:
    std::string name;
    std::vector<std::shared_ptr<ASTNode>> args;
};

// Function prototype (forward declaration) node
class FunctionProtoNode : public ASTNode {
public:
    FunctionProtoNode(const std::string& name, const std::vector<std::string>& params)
        : name(name), params(params) {}
    int evaluate() const override { return 0; }
    ASTNodeType getType() const override { return ASTNodeType::FUNCTION_PROTO; }
    const std::string& getName() const { return name; }
    const std::vector<std::string>& getParams() const { return params; }
private:
    std::string name;
    std::vector<std::string> params;
};

// Return statement node
class ReturnNode : public ASTNode {
public:
    ReturnNode(std::shared_ptr<ASTNode> expr) : expr(expr) {}
    int evaluate() const override { return 0; }
    ASTNodeType getType() const override { return ASTNodeType::RETURN; }
    std::shared_ptr<ASTNode> getExpr() const { return expr; }
private:
    std::shared_ptr<ASTNode> expr;
}; 