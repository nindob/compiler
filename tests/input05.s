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
    .comm	y,4,4
    movq $6, %r8
    movl	%r8d, x(%rip)
    movq $12, %r8
    movl	%r8d, y(%rip)
    movslq	x(%rip), %r8
    movslq	y(%rip), %r9
    cmpq	%r9, %r8
    setl	%r9b
    movzbq	%r9b, %r9
    cmpq	%r9, %r9
    jge	L1
    movslq	x(%rip), %r8
    movq %r8, %rdi
    call printint
    jmp	L2
L1:
    movslq	y(%rip), %r8
    movq %r8, %rdi
    call printint
L2:
    movslq	x(%rip), %r8
    movq $6, %r9
    cmpq	%r9, %r8
    sete	%r9b
    movzbq	%r9b, %r9
    cmpq	%r9, %r9
    jne	L3
    movq $1, %r8
    movq %r8, %rdi
    call printint
    jmp	L4
L3:
    movq $0, %r8
    movq %r8, %rdi
    call printint
L4:
    movslq	x(%rip), %r8
    movslq	y(%rip), %r9
    cmpq	%r9, %r8
    setg	%r9b
    movzbq	%r9b, %r9
    cmpq	%r9, %r9
    jle	L5
    movq $0, %r8
    movq %r8, %rdi
    call printint
    jmp	L6
L5:
    movq $1, %r8
    movq %r8, %rdi
    call printint
L6:
    movslq	x(%rip), %r8
    movq $6, %r9
    cmpq	%r9, %r8
    setle	%r9b
    movzbq	%r9b, %r9
    cmpq	%r9, %r9
    jg	L7
    movslq	y(%rip), %r8
    movq $12, %r9
    cmpq	%r9, %r8
    setge	%r9b
    movzbq	%r9b, %r9
    cmpq	%r9, %r9
    jl	L9
    movq $1, %r8
    movq %r8, %rdi
    call printint
    jmp	L10
L9:
    movq $0, %r8
    movq %r8, %rdi
    call printint
L10:
    jmp	L8
L7:
    movq $0, %r8
    movq %r8, %rdi
    call printint
L8:
    movl $0, %eax
    popq %rbp
    ret
