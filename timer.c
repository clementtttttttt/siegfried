#include "timer.h"
#include "io.h"
#include "idt.h"
#include "draw.h"
#include "apic.h"
#include "rtc.h"

void idt_timer_handler_s();

unsigned int timer_read_count(){
    return apic_read_reg(0x390);
}

void timer_setup(){

    idt_set_irq_ent(0x20,idt_timer_handler_s);
        idt_flush();


    lvt_tmr_reg lvt_tmr;
    lvt_tmr.raw = apic_read_reg(0x320);

    lvt_tmr.int_base = 0x20; //timer handler int int_base
    draw_hex(lvt_tmr.raw);
    lvt_tmr.is_disabled = 0;
    lvt_tmr.mode = 1; //periodic mode set

    draw_string("LVT TIMER REG: ");
    draw_hex(lvt_tmr.raw);

    //enable timer irq
    apic_write_reg(0x320, lvt_tmr.raw);

    asm("sti");
    rtc_sleep_for_TTEth_sec(1);
    asm("cli");

    //measure timer speed
    apic_write_reg(0x3e0, 8); //set timer divider

    apic_write_reg(0x380, 0xffffffff);

    asm("sti");
    rtc_sleep_for_TTEth_sec(1);
    asm("cli");

    unsigned int cnts_in_TTEth = 0xffffffff - apic_read_reg(0x390);

    draw_string("APIC TIMER INIT COUNT=");
    draw_hex(cnts_in_TTEth);
    

    apic_write_reg(0x380, cnts_in_TTEth); //set timer init count



 
}

void timer_wait(){

}

void timer_handler(){
}
