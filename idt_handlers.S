
.globl idt_pagefault_handler_s
.globl idt_timer_handler_s

tmp_r15: .quad 0

rip_save: .quad 0

idt_save_regs:
    pushq %rax
    pushq %rdi
    pushq %rsi
    pushq %rdx
    pushq %rcx
    pushq %r8
    pushq %r9
    pushq %r10
    pushq %r11
    jmp rip_save


idt_load_regs:
    popq %r11
    popq %r10
    popq %r9
    popq %r8
    popq %rcx
    popq %rdx
    popq %rsi
    popq %rdi
    popq %rax
    jmp rip_save


idt_timer_handler_s:
jmp .
    movq $1f, rip_save
    jmp idt_save_regs
    1:

    movb $0x20, %al
    outb %al, $0x20



    movq $2f, rip_save
    jmp idt_load_regs
    2:

    jmp .

    iretq




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

