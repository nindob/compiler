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
.globl main
main:
    pushq %rbp
    movq %rsp, %rbp
    movq $42, %r8
    movq %r8, -8(%rbp)
    movq -8(%rbp), %r8
    movq %r8, %rdi
    call printint
    movq $99, %r8
    movq %r8, -16(%rbp)
    movq -16(%rbp), %r8
    movq %r8, %rdi
    call printint
    movq -8(%rbp), %r8
    movq %r8, %rdi
    call printint
    movq %rbp, %rsp
    popq %rbp
    ret
    movl $0, %eax
    popq %rbp
    ret
