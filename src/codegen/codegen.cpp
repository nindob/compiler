#include "codegen.hpp"
#include <stdexcept>
#include <iostream>

CodeGenerator::CodeGenerator() : registers(4, false) {}

void CodeGenerator::freeall_registers() {
    for (size_t i = 0; i < registers.size(); ++i) registers[i] = false;
}

int CodeGenerator::alloc_register() {
    for (int i = 0; i < registers.size(); ++i) {
        if (!registers[i]) {
            registers[i] = true;
            return i;
        }
    }
    throw std::runtime_error("Out of registers!");
}

void CodeGenerator::free_register(int r) {
    if (r < 0 || r >= registers.size() || !registers[r])
        throw std::runtime_error("Attempt to free invalid register");
    registers[r] = false;
}

void CodeGenerator::cgpreamble() {
    output << ".text\n"
           << ".LC0:\n"
           << "    .asciz \"%ld\\n\"\n"
           << "printint:\n"
           << "    pushq   %rbp\n"
           << "    movq    %rsp, %rbp\n"
           << "    subq    $16, %rsp\n"
           << "    movq    %rdi, -8(%rbp)\n"
           << "    movq    -8(%rbp), %rax\n"
           << "    movq    %rax, %rsi\n"
           << "    leaq    .LC0(%rip), %rdi\n"
           << "    movl    $0, %eax\n"
           << "    call    _printf\n"
           << "    nop\n"
           << "    leave\n"
           << "    ret\n"
           << ".globl _main\n"
           << "_main:\n"
           << "    pushq %rbp\n"
           << "    movq %rsp, %rbp\n";
}

void CodeGenerator::cgpostamble() {
    output << "    movl $0, %eax\n"
           << "    popq %rbp\n"
           << "    ret\n";
}

int CodeGenerator::cgload(int value) {
    int r = alloc_register();
    output << "    movq $" << value << ", " << reglist[r] << "\n";
    return r;
}

int CodeGenerator::cgadd(int r1, int r2) {
    output << "    addq " << reglist[r1] << ", " << reglist[r2] << "\n";
    free_register(r1);
    return r2;
}

int CodeGenerator::cgmul(int r1, int r2) {
    output << "    imulq " << reglist[r1] << ", " << reglist[r2] << "\n";
    free_register(r1);
    return r2;
}

int CodeGenerator::cgsub(int r1, int r2) {
    output << "    subq " << reglist[r2] << ", " << reglist[r1] << "\n";
    free_register(r2);
    return r1;
}

int CodeGenerator::cgdiv(int r1, int r2) {
    output << "    movq " << reglist[r1] << ", %rax\n";
    output << "    cqo\n";
    output << "    idivq " << reglist[r2] << "\n";
    output << "    movq %rax, " << reglist[r1] << "\n";
    free_register(r2);
    return r1;
}

int CodeGenerator::cgmod(int r1, int r2) {
    output << "    movq " << reglist[r1] << ", %rax\n";
    output << "    cqo\n";
    output << "    idivq " << reglist[r2] << "\n";
    output << "    movq %rdx, " << reglist[r1] << "\n";
    free_register(r2);
    return r1;
}

// List of comparison instructions,
// in AST order: A_EQ, A_NE, A_LT, A_GT, A_LE, A_GE
static const char* cmplist[] = {
    "sete",  // EQ
    "setne", // NE
    "setl",  // LT
    "setg",  // GT
    "setle", // LE
    "setge"  // GE
};

int CodeGenerator::cgcompare(int r1, int r2, const char* how) {
    output << "    cmpq\t" << reglist[r2] << ", " << reglist[r1] << "\n";
    output << "    " << how << "\t" << breglist[r2] << "\n";
    output << "    movzbq\t" << breglist[r2] << ", " << reglist[r2] << "\n";
    free_register(r1);
    return r2;
}

int CodeGenerator::cgequal(int r1, int r2) { return cgcompare(r1, r2, "sete"); }
int CodeGenerator::cgnotequal(int r1, int r2) { return cgcompare(r1, r2, "setne"); }
int CodeGenerator::cglessthan(int r1, int r2) { return cgcompare(r1, r2, "setl"); }
int CodeGenerator::cggreaterthan(int r1, int r2) { return cgcompare(r1, r2, "setg"); }
int CodeGenerator::cglessequal(int r1, int r2) { return cgcompare(r1, r2, "setle"); }
int CodeGenerator::cggreaterequal(int r1, int r2) { return cgcompare(r1, r2, "setge"); }
int CodeGenerator::cglogicaland(int r1, int r2) {
    output << "    andq " << reglist[r1] << ", " << reglist[r2] << "\n";
    free_register(r1);
    return r2;
}
int CodeGenerator::cglogicalor(int r1, int r2) {
    output << "    orq " << reglist[r1] << ", " << reglist[r2] << "\n";
    free_register(r1);
    return r2;
}
int CodeGenerator::cgnegate(int r) {
    output << "    negq " << reglist[r] << "\n";
    return r;
}
int CodeGenerator::cgbitwisenot(int r) {
    output << "    notq " << reglist[r] << "\n";
    return r;
}
int CodeGenerator::cglogicalnot(int r) {
    output << "    cmpq $0, " << reglist[r] << "\n";
    output << "    sete " << breglist[r] << "\n";
    output << "    andq $255, " << reglist[r] << "\n";
    return r;
}

void CodeGenerator::cgprintint(int r) {
    output << "    movq " << reglist[r] << ", %rdi\n";
    output << "    call printint\n";
    free_register(r);
}

void CodeGenerator::emitGlobalVar(const std::string& name, TypeKind type) {
    int size = 8; // default to 8 bytes (int, long, double)
    switch (type) {
        case TypeKind::Char: size = 1; break;
        case TypeKind::Short: size = 2; break;
        case TypeKind::Int: size = 4; break;
        case TypeKind::Long: size = 8; break;
        case TypeKind::Unsigned: size = 8; break;
        case TypeKind::Bool: size = 1; break;
        case TypeKind::Float: size = 4; std::cerr << "[WARN] float treated as int for now.\n"; break;
        case TypeKind::Double: size = 8; std::cerr << "[WARN] double treated as int for now.\n"; break;
    }
    output << "    .comm\t" << name << "," << size << "," << size << "\n";
}

int CodeGenerator::cgloadglob(const std::string& name, TypeKind type) {
    int r = alloc_register();
    if (type == TypeKind::Char || type == TypeKind::Bool) {
        output << "    movzbq\t" << name << "(%rip), " << reglist[r] << "\n";
    } else if (type == TypeKind::Short) {
        output << "    movzwq\t" << name << "(%rip), " << reglist[r] << "\n";
    } else if (type == TypeKind::Int || type == TypeKind::Float) {
        output << "    movslq\t" << name << "(%rip), " << reglist[r] << "\n";
    } else {
        output << "    movq\t" << name << "(%rip), " << reglist[r] << "\n";
    }
    return r;
}

static std::string get_subreg(int r, TypeKind type) {
    // r: 0 = r8, 1 = r9, 2 = r10, 3 = r11
    static const char* q[] = {"%r8", "%r9", "%r10", "%r11"};
    static const char* d[] = {"%r8d", "%r9d", "%r10d", "%r11d"};
    static const char* w[] = {"%r8w", "%r9w", "%r10w", "%r11w"};
    static const char* b[] = {"%r8b", "%r9b", "%r10b", "%r11b"};
    switch (type) {
        case TypeKind::Char:
        case TypeKind::Bool: return b[r];
        case TypeKind::Short: return w[r];
        case TypeKind::Int:
        case TypeKind::Float: return d[r];
        case TypeKind::Long:
        case TypeKind::Unsigned:
        case TypeKind::Double: return q[r];
    }
    return q[r];
}

int CodeGenerator::cgstorglob(int r, const std::string& name, TypeKind type) {
    if (type == TypeKind::Char || type == TypeKind::Bool) {
        output << "    movb\t" << get_subreg(r, type) << ", " << name << "(%rip)\n";
    } else if (type == TypeKind::Short) {
        output << "    movw\t" << get_subreg(r, type) << ", " << name << "(%rip)\n";
    } else if (type == TypeKind::Int || type == TypeKind::Float) {
        output << "    movl\t" << get_subreg(r, type) << ", " << name << "(%rip)\n";
    } else {
        output << "    movq\t" << get_subreg(r, type) << ", " << name << "(%rip)\n";
    }
    return r;
}

int CodeGenerator::new_label() {
    return ++label_count;
}

void CodeGenerator::cglabel(int l) {
    output << "L" << l << ":\n";
}

void CodeGenerator::cgjump(int l) {
    output << "    jmp\tL" << l << "\n";
}

// List of jump instructions for the opposite of each comparison
static const char* opposite_jumps[] = {
    "jne",  // A_EQ -> jump if not equal
    "je",   // A_NE -> jump if equal
    "jge",  // A_LT -> jump if greater or equal
    "jle",  // A_GT -> jump if less or equal
    "jg",   // A_LE -> jump if greater
    "jl"    // A_GE -> jump if less
};

int CodeGenerator::cgcompare_and_jump(ASTNodeType op, int r1, int r2, int label) {
    if (op < ASTNodeType::EQ || op > ASTNodeType::GE) {
        throw std::runtime_error("Bad operator in cgcompare_and_jump");
    }

    output << "    cmpq\t" << reglist[r2] << ", " << reglist[r1] << "\n";
    output << "    " << opposite_jumps[static_cast<int>(op) - static_cast<int>(ASTNodeType::EQ)]
           << "\tL" << label << "\n";
    freeall_registers();
    return -1;  // No register to return
}

int CodeGenerator::genIfAST(const std::shared_ptr<IfNode>& node) {
    int Lfalse = new_label();
    int Lend = new_label();

    // Generate condition code and jump to false label if condition is false
    auto condition = node->getCondition();
    int r1 = genAST(condition);
    
    // For logical operators, we need to compare with zero
    if (condition->getType() == ASTNodeType::LOGICAL_AND ||
        condition->getType() == ASTNodeType::LOGICAL_OR ||
        condition->getType() == ASTNodeType::LOGICAL_NOT) {
        output << "    cmpq\t$0, " << reglist[r1] << "\n";
        output << "    je\tL" << Lfalse << "\n";
    } else {
        cgcompare_and_jump(condition->getType(), r1, r1, Lfalse);
    }
    freeall_registers();

    // Generate the true compound statement
    int r2 = genAST(node->getThenBranch());
    if (r2 >= 0) {
        cgprintint(r2);  // Print if there's a value to print
    }
    freeall_registers();

    // If there is an else clause, generate jump to end
    if (node->getElseBranch()) {
        cgjump(Lend);
    }

    // Now the false label
    cglabel(Lfalse);

    // Optional else clause
    if (node->getElseBranch()) {
        int r3 = genAST(node->getElseBranch());
        if (r3 >= 0) {
            cgprintint(r3);  // Print if there's a value to print
        }
        freeall_registers();
        cglabel(Lend);
    }

    return -1;  // No register to return
}

int CodeGenerator::genGlueAST(const std::shared_ptr<GlueNode>& node) {
    // Generate the left subtree
    if (node->getLeft()) {
        int r1 = genAST(node->getLeft());
        if (r1 >= 0) {
            cgprintint(r1);  // Print if there's a value to print
        }
        freeall_registers();
    }
    
    // Generate the right subtree
    if (node->getRight()) {
        int r2 = genAST(node->getRight());
        if (r2 >= 0) {
            cgprintint(r2);  // Print if there's a value to print
        }
        freeall_registers();
    }
    
    return -1;  // No register to return
}

int CodeGenerator::genAST(const std::shared_ptr<ASTNode>& node) {
    if (!node) return -1;

    int leftreg, rightreg;

    switch (node->getType()) {
        case ASTNodeType::IF: {
            auto ifNode = std::dynamic_pointer_cast<IfNode>(node);
            if (!ifNode) return -1;
            
            // Generate condition code
            leftreg = genAST(ifNode->getCondition());
            
            // Create labels
            int falseLabel = new_label();
            int endLabel = new_label();
            
            // Jump to false label if condition is false
            output << "    cmpq $0, " << reglist[leftreg] << "\n";
            output << "    je L" << falseLabel << "\n";
            free_register(leftreg);
            
            // Generate then branch
            int thenReg = genAST(ifNode->getThenBranch());
            if (thenReg >= 0) cgprintint(thenReg);
            
            // Jump to end
            cgjump(endLabel);
            
            // False label
            cglabel(falseLabel);
            
            // Generate else branch if it exists
            if (ifNode->getElseBranch()) {
                int elseReg = genAST(ifNode->getElseBranch());
                if (elseReg >= 0) cgprintint(elseReg);
            }
            
            // End label
            cglabel(endLabel);
            return -1;
        }
        
        case ASTNodeType::WHILE: {
            auto whileNode = std::dynamic_pointer_cast<WhileNode>(node);
            if (!whileNode) return -1;
            
            // Create labels
            int startLabel = new_label();
            int endLabel = new_label();
            
            // Start label
            cglabel(startLabel);
            
            // Generate condition code
            leftreg = genAST(whileNode->getCondition());
            
            // Jump to end if condition is false
            output << "    cmpq $0, " << reglist[leftreg] << "\n";
            output << "    je L" << endLabel << "\n";
            free_register(leftreg);
            
            // Generate body
            auto body = whileNode->getBody();
            if (body) {
                genAST(body);
            }
            
            // Jump back to start
            cgjump(startLabel);
            
            // End label
            cglabel(endLabel);
            return -1;
        }
        
        case ASTNodeType::FOR: {
            // For loops are implemented using while loops in the AST
            // The AST structure is: GLUE(init, WHILE(cond, GLUE(body, incr)))
            auto glueNode = std::dynamic_pointer_cast<GlueNode>(node);
            if (!glueNode) return -1;
            
            // Generate initialization code
            genAST(glueNode->getLeft());
            
            // Get the while node
            auto whileNode = std::dynamic_pointer_cast<WhileNode>(glueNode->getRight());
            if (!whileNode) return -1;
            
            // Create labels
            int startLabel = new_label();
            int endLabel = new_label();
            
            // Start label
            cglabel(startLabel);
            
            // Generate condition code
            leftreg = genAST(whileNode->getCondition());
            
            // Jump to end if condition is false
            output << "    cmpq $0, " << reglist[leftreg] << "\n";
            output << "    je L" << endLabel << "\n";
            free_register(leftreg);
            
            // Generate body and increment
            auto bodyWithIncr = std::dynamic_pointer_cast<GlueNode>(whileNode->getBody());
            if (bodyWithIncr) {
                // Generate body
                auto body = bodyWithIncr->getLeft();
                if (body) {
                    genAST(body);
                }
                
                // Generate increment
                auto incr = bodyWithIncr->getRight();
                if (incr) {
                    genAST(incr);
                }
            }
            
            // Jump back to start
            cgjump(startLabel);
            
            // End label
            cglabel(endLabel);
            return -1;
        }
        
        case ASTNodeType::GLUE: {
            auto glueNode = std::dynamic_pointer_cast<GlueNode>(node);
            if (!glueNode) return -1;
            
            // Generate the left subtree
            if (glueNode->getLeft()) {
                leftreg = genAST(glueNode->getLeft());
                if (leftreg >= 0) {
                    cgprintint(leftreg);
                }
            }
            
            // Generate the right subtree
            if (glueNode->getRight()) {
                rightreg = genAST(glueNode->getRight());
                if (rightreg >= 0) {
                    cgprintint(rightreg);
                }
            }
            
            return -1;
        }
        
        case ASTNodeType::NUMBER:
            return cgload(static_cast<NumberNode*>(node.get())->getValue());
            
        case ASTNodeType::VAR: {
            auto var = static_cast<VarNode*>(node.get());
            if (var->isLocal()) {
                int r = alloc_register();
                output << "    movq " << var->getOffset() << "(%rbp), " << reglist[r] << "\n";
                return r;
            } else {
                return cgloadglob(var->getName(), var->getVarType());
            }
        }
        
        case ASTNodeType::VAR_DECL: {
            auto varDecl = static_cast<VarDeclNode*>(node.get());
            if (varDecl->isLocal()) {
                // No codegen needed for local variable declaration
                return -1;
            } else {
                emitGlobalVar(varDecl->getName(), varDecl->getVarType());
                return -1;
            }
        }
        
        case ASTNodeType::ASSIGN: {
            auto assign = static_cast<AssignNode*>(node.get());
            int reg = genAST(assign->getExpr());
            if (assign->isLocal()) {
                output << "    movq " << reglist[reg] << ", " << assign->getOffset() << "(%rbp)\n";
                free_register(reg);
                return -1;
            } else {
                return cgstorglob(reg, assign->getName(), assign->getVarType());
            }
        }
        
        case ASTNodeType::ADD: {
            auto bin = static_cast<BinaryOpNode*>(node.get());
            int l = genAST(bin->getLeft());
            int r = genAST(bin->getRight());
            return cgadd(l, r);
        }
        
        case ASTNodeType::SUBTRACT: {
            auto bin = static_cast<BinaryOpNode*>(node.get());
            int l = genAST(bin->getLeft());
            int r = genAST(bin->getRight());
            return cgsub(l, r);
        }
        
        case ASTNodeType::MULTIPLY: {
            auto bin = static_cast<BinaryOpNode*>(node.get());
            int l = genAST(bin->getLeft());
            int r = genAST(bin->getRight());
            return cgmul(l, r);
        }
        
        case ASTNodeType::DIVIDE: {
            auto bin = static_cast<BinaryOpNode*>(node.get());
            int l = genAST(bin->getLeft());
            int r = genAST(bin->getRight());
            return cgdiv(l, r);
        }
        
        case ASTNodeType::MODULO: {
            auto bin = static_cast<BinaryOpNode*>(node.get());
            int l = genAST(bin->getLeft());
            int r = genAST(bin->getRight());
            return cgmod(l, r);
        }
        
        case ASTNodeType::EQ:
        case ASTNodeType::NE:
        case ASTNodeType::LT:
        case ASTNodeType::GT:
        case ASTNodeType::LE:
        case ASTNodeType::GE: {
            auto bin = static_cast<BinaryOpNode*>(node.get());
            int l = genAST(bin->getLeft());
            int r = genAST(bin->getRight());
            
            // Compare the values in the correct order
            output << "    cmpq " << reglist[r] << ", " << reglist[l] << "\n";
            
            // Set the result based on the comparison type
            int result = alloc_register();
            switch (node->getType()) {
                case ASTNodeType::EQ:
                    output << "    sete " << breglist[result] << "\n";
                    break;
                case ASTNodeType::NE:
                    output << "    setne " << breglist[result] << "\n";
                    break;
                case ASTNodeType::LT:
                    output << "    setl " << breglist[result] << "\n";
                    break;
                case ASTNodeType::GT:
                    output << "    setg " << breglist[result] << "\n";
                    break;
                case ASTNodeType::LE:
                    output << "    setle " << breglist[result] << "\n";
                    break;
                case ASTNodeType::GE:
                    output << "    setge " << breglist[result] << "\n";
                    break;
                default:
                    throw std::runtime_error("Unknown comparison operator");
            }
            
            // Zero-extend the result to 64 bits
            output << "    movzbq " << breglist[result] << ", " << reglist[result] << "\n";
            
            free_register(l);
            free_register(r);
            return result;
        }
        
        case ASTNodeType::LOGICAL_AND: {
            auto bin = static_cast<BinaryOpNode*>(node.get());
            int l = genAST(bin->getLeft());
            int r = genAST(bin->getRight());
            return cglogicaland(l, r);
        }
        
        case ASTNodeType::LOGICAL_OR: {
            auto bin = static_cast<BinaryOpNode*>(node.get());
            int l = genAST(bin->getLeft());
            int r = genAST(bin->getRight());
            return cglogicalor(l, r);
        }
        
        case ASTNodeType::NEGATE: {
            auto unary = static_cast<UnaryOpNode*>(node.get());
            int r = genAST(unary->getChild());
            return cgnegate(r);
        }
        
        case ASTNodeType::BITWISE_NOT: {
            auto unary = static_cast<UnaryOpNode*>(node.get());
            int r = genAST(unary->getChild());
            return cgbitwisenot(r);
        }
        
        case ASTNodeType::LOGICAL_NOT: {
            auto unary = static_cast<UnaryOpNode*>(node.get());
            int r = genAST(unary->getChild());
            return cglogicalnot(r);
        }
        
        case ASTNodeType::FUNCTION_DECL: {
            // Function definition: emit label, prologue, body, epilogue
            auto func = std::dynamic_pointer_cast<FunctionDeclNode>(node);
            if (!func) return -1;
            output << ".globl " << func->getName() << "\n";
            output << func->getName() << ":\n";
            output << "    pushq %rbp\n";
            output << "    movq %rsp, %rbp\n";
            // TODO: handle parameters and local variables
            int bodyReg = genAST(func->getBody());
            if (bodyReg >= 0) {
                output << "    movq " << reglist[bodyReg] << ", %rax\n";
                free_register(bodyReg);
            }
            output << "    movq %rbp, %rsp\n";
            output << "    popq %rbp\n";
            output << "    ret\n";
            return -1;
        }
        case ASTNodeType::FUNCTION_CALL: {
            // Function call: push args right-to-left, call, clean up stack
            auto call = std::dynamic_pointer_cast<FunctionCallNode>(node);
            if (!call) return -1;
            int nargs = call->getArgs().size();
            std::vector<int> argRegs;
            for (int i = nargs - 1; i >= 0; --i) {
                int reg = genAST(call->getArgs()[i]);
                output << "    pushq " << reglist[reg] << "\n";
                free_register(reg);
            }
            output << "    call " << call->getName() << "\n";
            if (nargs > 0) {
                output << "    addq $" << (8 * nargs) << ", %rsp\n";
            }
            int result = alloc_register();
            output << "    movq %rax, " << reglist[result] << "\n";
            return result;
        }
        case ASTNodeType::FUNCTION_PROTO: {
            // No codegen needed for forward declarations
            return -1;
        }
        case ASTNodeType::RETURN: {
            // Return statement: evaluate expr, move to %rax, jump to epilogue
            auto ret = std::dynamic_pointer_cast<ReturnNode>(node);
            if (!ret) return -1;
            int reg = genAST(ret->getExpr());
            output << "    movq " << reglist[reg] << ", %rax\n";
            free_register(reg);
            // For now, just emit ret (no jump to epilogue label)
            output << "    movq %rbp, %rsp\n";
            output << "    popq %rbp\n";
            output << "    ret\n";
            return -1;
        }
        
        default:
            throw std::runtime_error("Unknown AST node type");
    }
    
    return -1;
}

std::string CodeGenerator::generateAssembly(const std::shared_ptr<ASTNode>& ast) {
    if (!ast) {
        return output.str();
    }
    output.str("");
    freeall_registers();
    cgpreamble();
    int reg = genAST(ast);
    cgprintint(reg); // Print result
    cgpostamble();
    return output.str();
}
