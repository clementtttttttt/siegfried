#include "idt.h"
#include "draw.h"
#include "io.h"
#include "acpiman.h"
#include "tasks.h"
#include "devfs.h"
#include "syscall.h"
#include "kb.h"

#define KB_CMD 0x64
#define KB_DAT 0x60
#define KB_STAT 0x64


static kb_ev_t kb_buf[256] = {0}; //circular key ev buffer 
static unsigned char key_pop_ptr=0, key_push_ptr=0;

static int has_shift = 0;
void kb_push_ev(int code, int pressed){
		kb_buf[key_push_ptr].type = pressed?EV_PRESSED:EV_DEPRESSED;
		kb_buf[key_push_ptr].code = code;
		++key_push_ptr;
}


unsigned char kb_to_ascii(unsigned char in){

    char kbd_US [256] =
{
    0,  27/*ESC*/, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
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
    char kbd_US_shift [256] =
{
    0,  27/*ESC*/, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
  '\t', /* <-- Tab */
  'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
    0, /* <-- control key */
  'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '\"', '~',  0, '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?',   0,
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
	if(has_shift){
			return kbd_US_shift[in];
	}
	else{
		return kbd_US[in];
	}
}
kb_ev_t kb_pop_ev (){
		kb_ev_t ret = kb_buf[key_pop_ptr];
		kb_buf[key_pop_ptr++].type = EV_NULL; //clear it just in case
		return ret;
}



kb_ev_t kb_wait_and_pop_ev(){
	key_pop_ptr = key_push_ptr;
	kb_buf[key_pop_ptr].type = EV_NULL;//clear for wait
	
	while(!kb_buf[key_pop_ptr].type){
		task_yield();
		asm("sti"); //enable ints for kb int 
	}
	return kb_pop_ev();
}

DEVFS_READ_FUNC(kb_read){
	while(len){
		kb_ev_t e = kb_wait_and_pop_ev();
		if(e.type == EV_PRESSED){
			*(buf++) = kb_to_ascii(e.code);
			--len;
		}
		
	}
	return 0;
}

unsigned char kb_send_cmd_s(unsigned char cmd, unsigned char cmd2){

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

void kb_send_dat(unsigned char dat){
		while(io_inb(KB_STAT) & 0b10){}
		
		io_outb(KB_DAT, dat);
}



void kb_send_cmd_port1(unsigned char cmd, unsigned char dat){

    resend:
    

	kb_send_dat(cmd);

    kb_send_dat(dat);

    unsigned char resend_check = 0;

    while((resend_check) != 0xfa){
		
		while(!(io_inb(KB_STAT) & 0x1)){}
		
		resend_check = io_inb(KB_DAT) ;
		
        if(resend_check == 0xfe){
            goto resend;
        }
    }



}

static int prev_has_80=0;

void kb_handler(struct task_sframe *frame){

    int code;
        code = io_inb(KB_DAT);
    if(!(code & 0x80)){
		
		if(prev_has_80){
			prev_has_80 = 0;
			if(code == 0x2a || code == 0x36){ //shift
				has_shift = 0;
			}
			else
			
			
    		kb_push_ev(code,0);
    		return;
		}
		
		if(code == 1){ //esc
				draw_string("\n===DEBUG DUMPS TRIGGERED==\n");
				
				task_dump_sframe(frame);
				idt_print_stacktrace_depth((void*)frame->rbp,3);
				
		}	
		
		if(code == 0x2a || code == 0x36){ //shift
				has_shift = 1;
			
		}
		else
    		kb_push_ev(code,1);

    }
    else{
		if(code == 0xe0){
			prev_has_80 = 1;
		}
		else{
			code &= 0x7f;// remove release bit
			if(code == 0x2a || code == 0x36){ //shift
				has_shift = 0;
			}
			else
			kb_push_ev(code,0);
		}
	}

}

void idt_kb_handler_s();

void kb_setup(){

    
    acpi_fadt *fadt = (acpi_fadt*) acpiman_get_tab("FACP");
    
    if(!fadt){
		draw_string("FADT NOT FOUND\n");
		return;//


	}


    
    if(!(fadt->arch_flags & 0b10) && fadt->header.rev >= 2){
		draw_hex(fadt->arch_flags);
		draw_string("PS2 CTRL NOT FOUND\n");
			return; //no ps2 ctrl
	}
    
        apic_map_irq(0x1, 0x21);

    
    kb_send_cmd_port1(0xf0, 2);
    
    idt_set_irq_ent(0x21, idt_kb_handler_s);
	
    devfs_ent *d = devfs_make_ent("ps2kb");
    d->read_func = kb_read;
}
