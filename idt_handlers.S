
.globl idt_pagefault_handler_s
.globl idt_timer_handler_s
.globl idt_gpf_handler_s
.globl idt_debug_handler_s
.globl idt_mce_handler_s
.globl idt_rtc_handler_s
.globl idt_df_handler_s
.globl idt_syscall_handler_s
.globl idt_spurious_handler_s
.globl idt_kb_handler_s
.globl idt_debug_handler_s
.globl idt_div0_handler_s

.globl idt_load_regs
.globl rip_save

.extern apic_addr


tmp_r15: .quad 0

rip_save: .quad 0
rax_save: .quad 0

.globl apic_ack_int
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
    pushq %rbp
    pushq %rax


    xorq %rax, %rax
    movw %ds, %ax
    pushq %rax
	
	subq $8, %rsp //fix align
	

    pushq rip_save
    ret


idt_load_regs:
	

    addq $8, %rsp //fix align
    
    
    popq %rax
    movw %ax, %ds

    popq %rax
    popq %rbp
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

    call apic_ack_int

    call timer_handler


    call task_yield


    movq $2f, rip_save
    jmp idt_load_regs
    2:


    iretq

idt_kb_handler_s:
    movq $1f, rip_save
    jmp idt_save_regs
    1:
    
    movq %rsp, %rdi

    call kb_handler

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

    movq %rax, rax_save

    movq $2f, rip_save
    jmp idt_load_regs
    2:
    
    mov rax_save, %rax

    iretq

idt_spurious_handler_s:
    iretq

idt_div0_handler_s:

    movq $1f, rip_save
    jmp idt_save_regs
    1:

    movq %rsp, %rdi

    call idt_div0_handler

    jmp .

idt_debug_handler_s:

    movq $1f, rip_save
    jmp idt_save_regs
    1:

    movq %rsp, %rdi

    call idt_debug_handler
	
    cli
    hlt
    jmp .


idt_gpf_handler_s:

    movq $1f, rip_save
    jmp idt_save_regs
    1:

    movq %rsp, %rdi

    call idt_gpf_handler
	
    cli
    hlt
    jmp .

idt_df_handler_s:

    movq $1f, rip_save
    jmp idt_save_regs
    1:

    movq %rsp, %rdi

    call idt_df_handler

    jmp .

idt_pagefault_handler_s:

    movq $1f, rip_save
    jmp idt_save_regs
    1:

    movq %rsp, %rdi

    call idt_pagefault_handler

    jmp .

idt_mce_handler_s:

	
	iretq
