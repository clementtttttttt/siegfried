
.globl idt_pagefault_handler_s
.globl idt_timer_handler_s
.globl idt_gpf_handler_s
.globl idt_mce_handler_s
.globl idt_rtc_handler_s
.globl idt_syscall_handler_s

.extern apic_addr


tmp_r15: .quad 0

rip_save: .quad 0

apic_ack_int:
    movq apic_addr, %rdi
    movl $0, 0xb0(%rdi)
    ret

idt_save_regs:
    pushq %rdi
    pushq %rsi
    pushq %rdx
    pushq %rcx
    pushq %r8
    pushq %r9
    pushq %r10
    pushq %r11
    pushq %r12
    pushq %r13
    pushq %r14
    pushq %r15
    pushq %rbx    
    pushq %rax

    pushq rip_save
    ret


idt_load_regs:
    popq %rax
    popq %rbx
    popq %r15
    popq %r14
    popq %r13
    popq %r12
    popq %r11
    popq %r10
    popq %r9
    popq %r8
    popq %rcx
    popq %rdx
    popq %rsi
    popq %rdi

    pushq rip_save
    ret


idt_timer_handler_s:
	
    movq $1f, rip_save
    jmp idt_save_regs
    1:

    movq %rsp, %rdi // save trap frame.
    call task_next_task

    call timer_handler

    call apic_ack_int


    movq $2f, rip_save
    jmp idt_load_regs
    2:
  
    iretq

idt_rtc_handler_s:
    movq $1f, rip_save
    jmp idt_save_regs
    1:

    call rtc_handler

    call apic_ack_int

    movq $2f, rip_save
    jmp idt_load_regs
    2:

    iretq

idt_syscall_handler_s:
    movq $1f, rip_save
    jmp idt_save_regs
    1:

    call syscall_main

    movq $2f, rip_save
    jmp idt_load_regs
    2:

    iretq


idt_gpf_handler_s:

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

    call idt_gpf_handler

    jmp .

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

idt_mce_handler_s:

	
	iretq
