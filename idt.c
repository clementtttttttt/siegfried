#include "idt.h"
#include "debug.h"
#include "draw.h"
#include "tasks.h"

idt_desc idt_table[256];

idt_tab_desc idtr;

void idt_pagefault_handler(unsigned long rdi, unsigned long rsi, unsigned long rdx, unsigned long rcx, unsigned long r8, unsigned long r9, unsigned long rax, unsigned long rbx, unsigned long rsp, unsigned long rbp, unsigned long r10, unsigned long r11, unsigned long r12, unsigned long r13, unsigned long r14, unsigned long r15, unsigned long cr2, unsigned int errcode, unsigned long rip, unsigned long seg){

    dbgconout("PAGE FAULT: ERRCODE=");
    dbgnumout_bin(errcode);

    dbgconout("RIP: ");
    dbgnumout_hex(rip);

    dbgconout("CR2: ");
    dbgnumout_hex(cr2);

    draw_string("\xcd\xcd\xcd\xcdPAGE FAULT\xcd\xcd\xcd\xcd\nERRCODE=");
    draw_hex(errcode);
    draw_string("RIP=");
    draw_hex(rip);
    draw_string("CR2=");
    draw_hex(cr2);


    asm("cli;hlt;");
    while(1){

    };
}

struct gpf_err{
    unsigned int e : 1;
    unsigned int tbl : 2;
    unsigned int idx : 13;
    unsigned int rsvd : 16;
};

void idt_gpf_handler(task_trap_sframe *frame){

    if(curr_task){
     //   if(curr_task -> tid != 1){return;}
    }
    draw_hex(curr_task->tid);

    draw_string("FAULT TASK TID IS 1: PANICKING!!!\n");

    dbgconout("GENERAL PROPTECTION FAULT: ERRCODE=");
    dbgnumout_bin(frame->errcode);

    dbgconout("RIP: ");
    dbgnumout_hex(frame->rip);

    draw_string("\xcd\xcd\xcd\xcdGENERAL PROTECTION FAULT\xcd\xcd\xcd\xcd\nRAW ERRCODE=");
    draw_hex(frame->errcode);
    struct gpf_err err;

    *(unsigned int*)(&err) = frame->errcode;

    draw_string("IS_EXTERNAL=");
    draw_hex(err.e);
    draw_string("TBL=");
    draw_hex(err.tbl);
    draw_string("SEL=");
    draw_hex(err.idx);

    draw_string("RIP=");
    draw_hex(frame->rip);

    asm("cli;hlt;");
    while(1){

    };
}

void idt_pagefault_handler_s();

void idt_gpf_handler_s();

void idt_mce_handler_s();

void idt_spurious_handler_s();

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
    idt_table[no].type_attr.raw = 0x8e | 0b01100000;
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
    idt_set_trap_ent(0xd, idt_gpf_handler_s);
    idt_set_trap_ent(0x12, idt_mce_handler_s);
    idt_set_trap_ent(0xff, idt_spurious_handler_s);

    idt_flush();

}
