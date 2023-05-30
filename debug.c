#include "debug.h"
#include "io.h"

void dbgconchar(char in){


 	while (( ! io_inb( 0x379 ) )& 0x80 )
 	{
 		io_wait();
 	}

 	// Now put the data onto the data lines
 	io_outb( 0x378, in);

    unsigned char ctrl;
 	// Now pulse the strobe line to tell the printer to read the data
 	ctrl = io_inb( 0x37A);
 	io_outb( 0x37A, ctrl | 1 );
 	io_wait();
 	io_outb( 0x37A, ctrl);


 	// Now wait for the printer to finish processing
 	while (( ! io_inb( 0x379 )) & 0x80 )
 	{
 		io_wait();
 	}
}

void dbg_crash(){
        asm("movq $0, %rax;movq %rax,%cr3");

}

void dbgnumout_hex(unsigned long in){
    char hexlookup[] = "0123456789ABCDEF";

    for(int i=0;i<16;++i){
        char c = hexlookup[((in>>((15-i)*4))&0xf)];
        dbgconchar(c);
    }
    dbgconchar('\r');
    dbgconchar('\n');
}
void dbgnumout_bin(unsigned long in){
    for(int i=0;i<64;++i){
        char c = ((in>>((63-i)))&0x1) + '0';
        dbgconchar(c);
    }
    dbgconchar('\r');
    dbgconchar('\n');
}

void dbgconout(char* str){
    while(*str){
        dbgconchar(*str);
        ++str;
    }
}
