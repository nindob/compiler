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
    .comm	x,4,4
    movq $5, %r8
    pushq %r8
    movq $4, %r8
    pushq %r8
    call add
    addq $16, %rsp
    movq %rax, %r8
    movl	%r8d, x(%rip)
    movq %r8, %rdi
    call printint
    movslq	x(%rip), %r8
    movq %r8, %rdi
    call printint
    movq %rbp, %rsp
    popq %rbp
    ret
.globl add
add:
    pushq %rbp
    movq %rsp, %rbp
    movslq	a(%rip), %r8
    movslq	b(%rip), %r9
    addq %r8, %r9
    movq %r9, %rax
    movq %rbp, %rsp
    popq %rbp
    ret
    movq %rbp, %rsp
    popq %rbp
    ret
    movl $0, %eax
    popq %rbp
    ret
