#include "idt.h"
#include "draw.h"
#include "rtc.h"
#include "timer.h"
#include "tasks.h"
#include "diskman.h"
#include "draw.h"
#include "page.h"

void idt_syscall_handler_s();

void syscall_setup(){

    idt_set_irq_ent(0xf0, idt_syscall_handler_s);
    idt_flush();

}


void syscall_sleep(unsigned long in1){

    asm("sti");


    unsigned long count = rtc_get_count() + in1;

    while(count >= rtc_get_count()){
    }

    asm("cli");

}

void syscall_diskman_read(unsigned long disk_inode, unsigned long off_sects, unsigned long num_sects, void* buf){

    diskman_ent *e = diskman_find_ent(disk_inode);
    e -> read_func(disk_inode, off_sects, num_sects, page_lookup_paddr_tab(curr_task -> page_tab, buf));
}

void (*syscall_table[200])() = {syscall_sleep, syscall_diskman_read, draw_string_w_sz};

void syscall_main(unsigned long func,unsigned long i1, unsigned long i2, unsigned long i3, unsigned long i4){


    if(syscall_table[func]){
        asm("xchg %%bx,%%bx;callq *%0"::"r"(syscall_table[func]), "D"(i1), "S"(i2), "d"(i3), "c"(i4) : "rbx");
    }
    else{
        draw_string("UNKNOWN SYSCALL ");
        draw_hex (func);
    }

}

