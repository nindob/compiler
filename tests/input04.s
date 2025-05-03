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
    .comm	x,4,4
    movq $7, %r8
    movq $9, %r9
    cmpq %r9, %r8
    setl %r9b
    andq $255, %r9
    movl	%r9d, x(%rip)
    movslq	x(%rip), %r8
    movq %r8, %rdi
    call printint
    movq $7, %r8
    movq $9, %r9
    cmpq %r9, %r8
    setle %r9b
    andq $255, %r9
    movl	%r9d, x(%rip)
    movslq	x(%rip), %r8
    movq %r8, %rdi
    call printint
    movq $7, %r8
    movq $9, %r9
    cmpq %r9, %r8
    setne %r9b
    andq $255, %r9
    movl	%r9d, x(%rip)
    movslq	x(%rip), %r8
    movq %r8, %rdi
    call printint
    movq $7, %r8
    movq $7, %r9
    cmpq %r9, %r8
    sete %r9b
    andq $255, %r9
    movl	%r9d, x(%rip)
    movslq	x(%rip), %r8
    movq %r8, %rdi
    call printint
    movq $7, %r8
    movq $7, %r9
    cmpq %r9, %r8
    setge %r9b
    andq $255, %r9
    movl	%r9d, x(%rip)
    movslq	x(%rip), %r8
    movq %r8, %rdi
    call printint
    movq $7, %r8
    movq $7, %r9
    cmpq %r9, %r8
    setle %r9b
    andq $255, %r9
    movl	%r9d, x(%rip)
    movslq	x(%rip), %r8
    movq %r8, %rdi
    call printint
    movq $9, %r8
    movq $7, %r9
    cmpq %r9, %r8
    setg %r9b
    andq $255, %r9
    movl	%r9d, x(%rip)
    movslq	x(%rip), %r8
    movq %r8, %rdi
    call printint
    movq $9, %r8
    movq $7, %r9
    cmpq %r9, %r8
    setge %r9b
    andq $255, %r9
    movl	%r9d, x(%rip)
    movslq	x(%rip), %r8
    movq %r8, %rdi
    call printint
    movq $9, %r8
    movq $7, %r9
    cmpq %r9, %r8
    setne %r9b
    andq $255, %r9
    movl	%r9d, x(%rip)
    movslq	x(%rip), %r8
    movq %r8, %rdi
    call printint
    movq $0, %r8
    cmpq $0, %r8
    sete %r8b
    andq $255, %r8
    movl	%r8d, x(%rip)
    movslq	x(%rip), %r8
    movq %r8, %rdi
    call printint
    movq $0, %r8
    notq %r8
    movl	%r8d, x(%rip)
    movslq	x(%rip), %r8
    movq %r8, %rdi
    call printint
    movq $5, %r8
    negq %r8
    movl	%r8d, x(%rip)
    movslq	x(%rip), %r8
    movq %r8, %rdi
    call printint
    movq $1, %r8
    movq $0, %r9
    andq %r8, %r9
    movl	%r9d, x(%rip)
    movslq	x(%rip), %r8
    movq %r8, %rdi
    call printint
    movq $1, %r8
    movq $0, %r9
    orq %r8, %r9
    movl	%r9d, x(%rip)
    movslq	x(%rip), %r8
    movq %r8, %rdi
    call printint
    movl $0, %eax
    popq %rbp
    ret
