#include "timer.h"
#include "io.h"
#include "idt.h"

void idt_timer_handler_s();

void timer_setup(){

    idt_set_irq_ent(0x20,idt_timer_handler_s);

    idt_flush();

    //asm("sti");
}

void timer_wait(){

}
