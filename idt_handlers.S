
.globl idt_pagefault_handler_s

tmp_r15: .quad 0
idt_pagefault_handler_s:

    movq %r15, tmp_r15
    movq %cr2, %r15

    pushq %r15

    movq tmp_r15, %r15

    pushq %r15
    pushq %r14
    pushq %r13
    pushq %r12
    pushq %r11
    pushq %r10
    pushq %rbp
    pushq %rsp
    pushq %rbx
    pushq %rax

    call idt_pagefault_handler

    jmp .

