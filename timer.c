#include "timer.h"
#include "io.h"
#include "idt.h"
#include "draw.h"
#include "apic.h"

void idt_timer_handler_s();



void timer_setup(){

    idt_set_irq_ent(0x20,idt_timer_handler_s);

    lvt_tmr_reg lvt_tmr;
    lvt_tmr.raw = apic_read_reg(0x320);

    lvt_tmr.int_base = 0x20; //timer handler int int_base
    draw_hex(lvt_tmr.raw);
    lvt_tmr.is_disabled = 0;
    lvt_tmr.mode = 1; //periodic mode set

    draw_string("LVT TIMER REG: ");
    draw_hex(lvt_tmr.raw);

    apic_write_reg(0x320, lvt_tmr.raw);

    apic_write_reg(0x3e0, 32); //set timer divider

    apic_write_reg(0x380, 100); //set timer init count


    idt_flush();

 
}

void timer_wait(){

}

void timer_handler(){
    draw_string("TIMER IRQ!\n");
}
