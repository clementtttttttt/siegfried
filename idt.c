#include "idt.h"
#include "debug.h"
#include "draw.h"
#include "tasks.h"

idt_desc idt_table[256];

idt_tab_desc idtr;

struct stackframe {
  struct stackframe* ebp;
  unsigned long eip;
};

void idt_print_stacktrace(unsigned long *stack){
    draw_string("\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\315BEGIN STACK TRACE\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\n");

    draw_string("STACK=");
    draw_hex((unsigned long)stack);

    stack = __builtin_frame_address(0);

    while(stack != 0){
            draw_hex(stack[1]);
            stack = (unsigned long*) stack[0];
    }

    draw_string("\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\315END STACK TRACE\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\n");
}

unsigned long idt_dump_cr2();
asm("idt_dump_cr2: movq %cr2, %rax; retq");

void idt_pagefault_handler(task_trap_sframe *fr){

    dbgconout("PAGE FAULT: ERRCODE=");
    dbgnumout_bin(fr->errcode);

    dbgconout("RIP: ");
    dbgnumout_hex(fr->rip);

    dbgconout("CR2: ");
    dbgnumout_hex(idt_dump_cr2());

    draw_string("\n\xcd\xcd\xcd\xcdPAGE FAULT\xcd\xcd\xcd\xcd\nRAW ERRCODE=");
    draw_hex(fr -> errcode);
    draw_string("ERRBITS: ");

    if(fr->errcode & 0b1) draw_string("PRSNT ");
    if(fr->errcode & 0b10) draw_string("WRITE ");
    if(fr->errcode & 0b100) draw_string("USR ");
    if(fr->errcode & 0b1000) draw_string("RSVDWRITE ");
    if(fr->errcode & 0b10000) draw_string("INSTRGET ");
    if(fr->errcode & 0b100000) draw_string("PROTKEY ");
    if(fr->errcode & 0b1000000) draw_string("SHADOWSTK ");
    if(fr->errcode & 0b10000000) draw_string("SGXEX ");

    draw_string("\n");


    draw_string("RIP=");
    draw_hex(fr->rip);
    draw_string("CR2=");
    draw_hex(idt_dump_cr2());

    idt_print_stacktrace((unsigned long*)fr->rbp);

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

    draw_string("RCX=");
    draw_hex(frame->rcx);    

    idt_print_stacktrace((unsigned long*)frame->rsp);

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
    idt_table[no].type_attr.raw = 0x8e;
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
