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
    .comm	c,1,1
    .comm	u,8,8
    .comm	b,1,1
    .comm	s,2,2
    .comm	l,8,8
    movq $65, %r8
    movb	%r8b, c(%rip)
    movq $42, %r8
    movq	%r8, u(%rip)
    movq $1, %r8
    movb	%r8b, b(%rip)
    movq $1234, %r8
    movw	%r8w, s(%rip)
    movq $987654321, %r8
    movq	%r8, l(%rip)
    movzbq	c(%rip), %r8
    movq %r8, %rdi
    call printint
    movq	u(%rip), %r8
    movq %r8, %rdi
    call printint
    movzbq	b(%rip), %r8
    movq %r8, %rdi
    call printint
    movzwq	s(%rip), %r8
    movq %r8, %rdi
    call printint
    movq	l(%rip), %r8
    movq %r8, %rdi
    call printint
    movl $0, %eax
    popq %rbp
    ret
