#include "idt.h"
#include "draw.h"
#include "rtc.h"
#include "timer.h"
#include "tasks.h"

void idt_syscall_handler_s();

void syscall_setup(){

    idt_set_irq_ent(0xf0, idt_syscall_handler_s);
    idt_flush();

}


void syscall_sleep(){
    task_enter_krnl();

    asm("sti");


    unsigned long count = rtc_get_count() + 10;

    while(count >= rtc_get_count()){
    //    task_yield();
    }

    asm("cli");

    task_exit_krnl();
}

void (*syscall_table[200])() = {syscall_sleep};

void syscall_main(int func){

    if(syscall_table[func]){
        syscall_table[func]();
    }
    else{
        draw_string("UNKNOWN SYSCALL ");
        draw_hex (func);
    }

}

