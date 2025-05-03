.text
.LC0:
    .asciz "%ld\n"
printint:
    pushq   %rbp
    movq    %rsp, %rbp
    subq    $16, %rsp
    movq    %rdi, -8(%rbp)
    movq    -8(%rbp), %rax
    movq    %rax, %rsi
    leaq    .LC0(%rip), %rdi
    movl    $0, %eax
    call    _printf
    nop
    leave
    ret
.globl _main
_main:
    pushq %rbp
    movq %rsp, %rbp
.globl foo
foo:
    pushq %rbp
    movq %rsp, %rbp
    movq $42, %r8
    movq %r8, %rax
    movq %rbp, %rsp
    popq %rbp
    ret
    movq %rbp, %rsp
    popq %rbp
    ret
.globl main
main:
    pushq %rbp
    movq %rsp, %rbp
    call foo
    movq %rax, %r8
    movq %r8, %rax
    movq %rbp, %rsp
    popq %rbp
    ret
    movl $0, %eax
    popq %rbp
    ret
