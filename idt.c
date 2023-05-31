#include "idt.h"
#include "debug.h"

idt_desc idt_table[255];

idt_tab_desc idtr;

void idt_pagefault_handler(unsigned long rdi, unsigned long rsi, unsigned long rdx, unsigned long rcx, unsigned long r8, unsigned long r9, unsigned long rax, unsigned long rbx, unsigned long rsp, unsigned long rbp, unsigned long r10, unsigned long r11, unsigned long r12, unsigned long r13, unsigned long r14, unsigned long r15, unsigned long cr2, unsigned int errcode, unsigned long rip, unsigned long seg){

    dbgconout("PAGE FAULT: ERRCODE=");
    dbgnumout_bin(errcode);

    dbgconout("RIP: ");
    dbgnumout_hex(rip);

    dbgconout("CR2: ");
    dbgnumout_hex(cr2);

    while(1){

    };
}

void idt_pagefault_handler_s();

void idt_set_addr(idt_desc* desc, unsigned long addr){

    desc->offset_l16 = addr & 0xffff;
    desc->offset_h16 = (addr >> 16) & 0xffff;
    desc->offset_hh32 = (addr >> 32) & 0xffffffff;
}

void idt_set_trap_ent(unsigned long no, void* addr){
    idt_table[no].type_attr.raw = 0x8f;
    idt_table[no].code_seg = 0x8;
    idt_set_addr(&idt_table[no], (unsigned long)addr);

}

void idt_set_irq_ent(unsigned long no, void* addr){
    idt_table[no].type_attr.raw = 0x8e;
    idt_table[no].code_seg = 0x8;
    idt_set_addr(&idt_table[no], (unsigned long)addr);

}

void idt_flush(){

    asm inline("lidtq idtr");
}

void idt_setup(){

    idtr.addr = &idt_table[0];
    idtr.sz = sizeof(idt_table) - 1;


    idt_set_trap_ent(0xe, idt_pagefault_handler_s);

    idt_flush();

}
