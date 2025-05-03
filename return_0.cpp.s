        .section        __TEXT,__text,regular,pure_instructions
        .build_version macos, 15, 0     sdk_version 15, 2
        .intel_syntax noprefix
        .globl  _main
        .p2align        4, 0x90
_main:
        push    rbp
        mov     rbp, rsp
        mov     eax, 0
        pop     rbp
        ret
        .subsections_via_symbols
