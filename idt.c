#include "idt.h"
#include "debug.h"
#include "draw.h"
#include "tasks.h"
#include "klib.h"
#include "page.h"
#include "obj_heap.h"

idt_desc idt_table[256];

idt_tab_desc idtr;

struct stackframe {
  struct stackframe* ebp;
  unsigned long eip;
};


void idt_print_stacktrace_con(unsigned long *stack){
    dbgconout("\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\315BEGIN STACK TRACE\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\n");

    dbgconout("STACK=");
    dbgnumout_hex((unsigned long)stack);

   // stack = __builtin_frame_address(0);

    while(stack != 0){
            dbgnumout_hex(stack[1]);
            stack = (unsigned long*) stack[0];
    }

    dbgconout("\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\315END STACK TRACE\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\n");
}

void idt_print_stacktrace(unsigned long *stack){
    draw_string("\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\315BEGIN STACK TRACE\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\n");

    draw_string("STACK=");
    draw_hex((unsigned long)stack);

   // stack = __builtin_frame_address(0);

    while(stack != 0){
            draw_hex(stack[1]);
            stack = (unsigned long*) stack[0];
    }

    draw_string("\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\315END STACK TRACE\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\n");
}

//TODO: ADD SYMBOL READING
void idt_print_stacktrace_depth(unsigned long *stack, int depth){
    draw_string("\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\315BEGIN STACK TRACE\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\n");

    draw_string("STACK=");
    draw_hex((unsigned long)stack);

   // stack = __builtin_frame_address(0);

    for(int i=0;stack != 0 && i<depth; ++i){
            draw_hex(stack[1]);
            stack = (unsigned long*) stack[0];
    }

    draw_string("\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\315END STACK TRACE\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\n");
}

unsigned long idt_dump_cr2();
asm("idt_dump_cr2: movq %cr2, %rax; retq");

void idt_pagefault_handler(task_trap_sframe *fr){

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
    
    draw_string("RIP DUMP(REVERSED)=");
    
    unsigned long dump;
    page_switch_tab(curr_task->page_tab);
    dump = *(unsigned long*)fr->rip;
    
    page_switch_krnl_tab();
    draw_hex(dump);

    draw_string("CR2=");
    draw_hex(idt_dump_cr2());
    
    draw_string("CR3=");
    draw_hex((unsigned long)curr_task->page_tab);

    page_dump_pde(page_lookup_pdei(page_get_curr_tab(),(void*) idt_dump_cr2()));
    
    draw_string("usr stack=");
    draw_hex((unsigned long)curr_task->user_stack_base);

    	task_dump_sframe((task_int_sframe*)((unsigned long)fr + 8));

    
    if(curr_task){
		draw_string("\nERR: userland PF");
		draw_string("TID=");
		draw_hex(curr_task->tid);
		curr_task->tf = (void*)(((unsigned long)fr));
	
        task_exit(DIED_SEGFAULT);
    }
    dbgconout("PAGE FAULT: ERRCODE=");
    dbgnumout_bin(fr->errcode);

    dbgconout("RIP: ");
    dbgnumout_hex(fr->rip);


    dbgconout("CR2: ");
    dbgnumout_hex(idt_dump_cr2());


	page_switch_tab(curr_task->page_tab);
	idt_print_stacktrace((unsigned long*)fr->rbp);

	//page_switch_tab(curr_task->page_tab);

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

void idt_df_handler(task_trap_sframe *frame){

    if(curr_task){
		draw_string("\nERR: userland DF");
		curr_task->tf = (void*)(((unsigned long)frame));

        task_exit(139);
    }
    draw_hex(curr_task->tid);

    draw_string("FAULT TASK TID IS 1: PANICKING!!!\n");

    dbgconout("DOUBLE FAULT: ERRCODE=");
    dbgnumout_bin(frame->errcode);

    dbgconout("RIP: ");
    dbgnumout_hex(frame->rip);

    draw_string("\xcd\xcd\xcd\xcd DOUBLE FAULT\xcd\xcd\xcd\xcd\nRAW ERRCODE=");
    draw_hex(frame->errcode);

    draw_string("RIP=");
    draw_hex(frame->rip);

    draw_string("RCX=");
    draw_hex(frame->rcx);

	if(curr_task->tid != 0)
    idt_print_stacktrace((unsigned long*)curr_task->tf->rbp);
	else{
			idt_print_stacktrace((unsigned long*) frame->rbp);
	}

    asm("cli;hlt;");
    while(1){

    };
}


void idt_div0_handler(task_trap_sframe *frame){

    if(curr_task){
		draw_string("\nERR: userland GPF");
		curr_task->tf = (void*)(((unsigned long)frame));

        task_exit(139);
    }
    draw_hex(curr_task->tid);

    draw_string("FAULT TASK TID IS 1: PANICKING!!!\n");

    draw_string("DIVISION ERROR: ");


    draw_string("RIP=");
    draw_hex(frame->rip);

    draw_string("RCX=");
    draw_hex(frame->rcx);

    idt_print_stacktrace((unsigned long*)frame->rbp);

    asm("cli;hlt;");
    while(1){

    };
}


void idt_debug_handler(task_int_sframe *frame){

    draw_string("DEBUG INTERRUPT: ");


    draw_string("RIP=");
    draw_hex(frame->rip);

    draw_string("RCX=");
    draw_hex(frame->rcx);

    idt_print_stacktrace((unsigned long*)frame->rbp);

	//liballoc_dump();

    asm("cli;hlt;");
    while(1){

    };
}


void idt_gpf_handler(task_trap_sframe *frame){


    draw_string("\xcd\xcd\xcd\xcdGENERAL PROTECTION FAULT\xcd\xcd\xcd\xcd\nRAW ERRCODE=");
    draw_hex(frame->errcode);
    struct gpf_err err;

    mem_cpy(&err, &frame->errcode, 4);

    draw_string("IS_EXTERNAL=");
    draw_hex(err.e);
    draw_string("TBL=");
    draw_hex(err.tbl);
    draw_string("SEL=");
    draw_hex(err.idx);

    draw_string("RIP=");
    draw_hex(frame->rip);
    
        
    draw_string("RIP DUMP(REVERSED)=");
    draw_hex(*(unsigned long*)frame->rip);


    if(curr_task){
		draw_string("\nERR: userland GPF");
		draw_string("\nTID=");
		    draw_hex(curr_task->tid);

		curr_task->tf = (void*)(((unsigned long)frame));
		idt_print_stacktrace_depth((unsigned long*)frame->rbp,8);

        task_exit(139);
    }

    draw_string("FAULT TASK TID IS 1: PANICKING!!!\n");


    draw_string("RCX=");
    draw_hex(frame->rcx);    

    idt_print_stacktrace((unsigned long*)frame->rbp);

    asm("cli;hlt;");
    while(1){

    };
}

void idt_pagefault_handler_s();
void idt_gpf_handler_s();
void idt_mce_handler_s();
void idt_spurious_handler_s();
void idt_df_handler_s();
void idt_div0_handler_s();
void idt_debug_handler_s();

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

    asm volatile("lidtq %0"::"m"(idtr));
}

void idt_setup(){

    idtr.addr = &idt_table[0];
    idtr.sz = sizeof(idt_table) - 1;


    idt_set_trap_ent(0xe, idt_pagefault_handler_s);
    idt_set_trap_ent(0xd, idt_gpf_handler_s);
    idt_set_trap_ent(1, idt_debug_handler_s);
    idt_set_trap_ent(0x12, idt_mce_handler_s);
    idt_set_trap_ent(0xff, idt_spurious_handler_s);
    idt_set_trap_ent(0x8, idt_df_handler_s);
    idt_set_trap_ent(0x0, idt_div0_handler_s);

    idt_flush();

}
