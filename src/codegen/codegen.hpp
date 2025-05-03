#pragma once
#include <string>
#include <sstream>
#include <vector>
#include "../ast/ast.hpp"

class CodeGenerator {
public:
    CodeGenerator();
    std::string generateAssembly(const std::shared_ptr<ASTNode>& ast);
    void cgpreamble();
    void cgpostamble();
    void freeall_registers();
    int genAST(const std::shared_ptr<ASTNode>& node);
    void cgprintint(int r);
    void emitGlobalVar(const std::string& name, TypeKind type);

private:
    std::stringstream output;
    std::vector<bool> registers; // true if register is in use
    static constexpr const char* reglist[4] = {"%r8", "%r9", "%r10", "%r11"};
    static constexpr const char* breglist[4] = {"%r8b", "%r9b", "%r10b", "%r11b"};
    int label_count = 0;

    int alloc_register();
    void free_register(int r);
    int cgload(int value);
    int cgadd(int r1, int r2);
    int cgmul(int r1, int r2);
    int cgsub(int r1, int r2);
    int cgdiv(int r1, int r2);
    int cgmod(int r1, int r2);
    int cgloadglob(const std::string& name, TypeKind type);
    int cgstorglob(int r, const std::string& name, TypeKind type);
    int cgcompare(int r1, int r2, const char* how);
    int cgequal(int r1, int r2);
    int cgnotequal(int r1, int r2);
    int cglessthan(int r1, int r2);
    int cggreaterthan(int r1, int r2);
    int cglessequal(int r1, int r2);
    int cggreaterequal(int r1, int r2);
    int cglogicaland(int r1, int r2);
    int cglogicalor(int r1, int r2);
    int cgnegate(int r);
    int cgbitwisenot(int r);
    int cglogicalnot(int r);
    
    // New methods for if statements
    int genIfAST(const std::shared_ptr<IfNode>& node);
    int genGlueAST(const std::shared_ptr<GlueNode>& node);
    int cgcompare_and_jump(ASTNodeType op, int r1, int r2, int label);
    void cgjump(int label);
    void cglabel(int label);
    int new_label();
};
