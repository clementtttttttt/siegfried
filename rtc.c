#include "idt.h"
#include "apic.h"
#include "io.h"
#include "draw.h"

void idt_rtc_handler_s();

volatile unsigned long rtc_count = 0;

void rtc_setup(){
    asm("cli");

    idt_set_irq_ent(0x28, idt_rtc_handler_s);

    apic_map_irq(0x8, 0x28);

    io_outb(0x70, 0x8b);
    char old_71 = io_inb(0x71);
    io_outb(0x70,0x8b);
    io_outb(0x71,old_71 | 0x40);

    io_outb(0x70, 0x8A);		// set index to register A, disable NMI
    char old_a=io_inb(0x71);	// get initial value of register A
    io_outb(0x70, 0x8A);		// reset index to A
    io_outb(0x71, (old_a & 0xF0) | 5); //write only our rate to A. Note, rate is the bottom 4 bits.
    //2048hz

    asm("sti");


    io_outb(0x70, 0);
    io_inb(0x71);  //clear pending rtc interrupts

}

void rtc_handler(){
    ++rtc_count;

    //rtc EOI
    io_outb(0x70, 0xc);
    io_inb(0x71);

}

unsigned long rtc_get_count(){
    return rtc_count;
}

//TTEth = 2048th

void rtc_sleep_for_TTEth_sec(unsigned int TTEths){
    unsigned long dest_rtc_count = rtc_count + TTEths;
    asm("sti");
    while(rtc_count <= dest_rtc_count){}

    asm("cli");
}
