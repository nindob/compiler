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
    .comm	i,4,4
    movq $1, %r8
    movl	%r8d, i(%rip)
L1:
    movslq	i(%rip), %r8
    movq $10, %r9
    cmpq %r9, %r8
    setle %r10b
    movzbq %r10b, %r10
    cmpq $0, %r10
    je L2
    movslq	i(%rip), %r8
    movq %r8, %rdi
    call printint
    movslq	i(%rip), %r8
    movq $1, %r9
    addq %r8, %r9
    movl	%r9d, i(%rip)
    movq %r9, %rdi
    call printint
    jmp	L1
L2:
    .comm	j,4,4
    movq $1, %r8
    movl	%r8d, j(%rip)
    movq %r8, %rdi
    call printint
L3:
    movslq	j(%rip), %r8
    movq $10, %r9
    cmpq %r9, %r8
    setle %r10b
    movzbq %r10b, %r10
    cmpq $0, %r10
    je L4
    movslq	j(%rip), %r8
    movq %r8, %rdi
    call printint
    movslq	j(%rip), %r8
    movq $1, %r9
    addq %r8, %r9
    movl	%r9d, j(%rip)
    movq %r9, %rdi
    call printint
    jmp	L3
L4:
    movl $0, %eax
    popq %rbp
    ret
