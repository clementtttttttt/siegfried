#include "idt.h"
#include "draw.h"
#include "io.h"
#include "acpiman.h"

#define KB_CMD 0x64
#define KB_DAT 0x60
#define KB_STAT 0x64

char kb_port = 0; //eith 1 or 2

unsigned char knb_send_cmd_s(unsigned char cmd, unsigned char cmd2){

        io_outb(KB_CMD, cmd);
        while(io_inb(KB_STAT) & 0b10);

        io_outb(KB_CMD,cmd2);

        while(!(io_inb(KB_STAT) & 1)){}

                return io_inb(KB_DAT);

}

unsigned char kb_send_cmd_b(unsigned char cmd){

        io_outb(KB_CMD, cmd);

        while(!(io_inb(KB_STAT) & 1)){}

        return io_inb(KB_DAT);

}

unsigned char kb_to_ascii(unsigned char in){

    char kbd_US [256] =
{
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
  '\t', /* <-- Tab */
  'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, /* <-- control key */
  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',  0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/',   0,
  '*',
    0,  /* Alt */
  ' ',  /* Space bar */
    0,  /* Caps lock */
    0,  /* 59 - F1 key ... > */
    0,   0,   0,   0,   0,   0,   0,   0,
    0,  /* < ... F10 */
    0,  /* 69 - Num lock*/
    0,  /* Scroll Lock */
    0,  /* Home key */
    0,  /* Up Arrow */
    0,  /* Page Up */
  '-',
    0,  /* Left Arrow */
    0,
    0,  /* Right Arrow */
  '+',
    0,  /* 79 - End key*/
    0,  /* Down Arrow */
    0,  /* Page Down */
    0,  /* Insert Key */
    0,  /* Delete Key */
    0,   0,   0,
    0,  /* F11 Key */
    0,  /* F12 Key */
    0,  /* All other keys are undefined */
};



    return kbd_US[in];
}

void kb_send_cmd_port1(unsigned char cmd, unsigned char dat){

    resend:

    io_outb(KB_DAT, cmd);

    io_outb(KB_DAT, dat);

    unsigned char resend_check;

    while((resend_check = io_inb(KB_DAT) ) != 0xfa){
        if(resend_check == 0xfe){
            goto resend;
        }
    }



}

void kb_handler(){

    char test;
        test = io_inb(KB_DAT);
    if(!(test & 0x80)){

        test = kb_to_ascii(test);

        draw_string_w_sz(&test, 1);
    }
}

void idt_kb_handler_s();

void kb_setup(){

    kb_send_cmd_port1(0xf0, 2);

    idt_set_irq_ent(0x21, idt_kb_handler_s);

}
