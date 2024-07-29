#include "debug.h"
#include "io.h"
#include "draw.h"

#define STR1(x) #x
#define STR(x) STR1(x)

#define READ_DBGREG(dbgreg, var) asm volatile("movq %%" STR(dbgreg) ", %0 ":"=r"(var));
#define WRITE_DBGREG(dbgreg, var) asm volatile("movq %0, %%" STR(dbgreg)::"r"(var));





unsigned long dbg_read_debug_reg(char reg){
	
	unsigned long tmp=0;
	
	switch(reg){
			case REG_DR0:
				READ_DBGREG(dr0, tmp);
			
			break;
			case REG_DR1:
				READ_DBGREG(dr1, tmp);
			break;
			case REG_DR2:
				READ_DBGREG(dr2, tmp);

			break;
			case REG_DR3:
				READ_DBGREG(dr3, tmp);
			
			break;
			case REG_DR6:
				READ_DBGREG(dr6, tmp);
			
			break;
			case REG_DR7:
				READ_DBGREG(dr7, tmp);
			
			break;
	}
	
	return tmp;
}

void dbg_write_debug_reg(unsigned long tmp,char reg){

	switch(reg){
			case REG_DR0:
				WRITE_DBGREG(dr0, tmp);
			
			break;
			case REG_DR1:
				WRITE_DBGREG(dr1, tmp);
			break;
			case REG_DR2:
				WRITE_DBGREG(dr2, tmp);

			break;
			case REG_DR3:
				WRITE_DBGREG(dr3, tmp);
			
			break;
			case REG_DR6:
				WRITE_DBGREG(dr6, tmp);
			
			break;
			case REG_DR7:
				WRITE_DBGREG(dr7, tmp);
			
			break;
	}
}

void dbg_disable_breakpoint(char bp_num, char islocal){
	dr7_t val;
	
	val.raw = dbg_read_debug_reg(REG_DR0);
	
	unsigned char en_bits = islocal?0b01:0b10;
	unsigned long tmp = (en_bits << (bp_num*2));
	tmp = !tmp;
	
	val.raw &= tmp;
	
	dbg_write_debug_reg(*(unsigned long*)&val, REG_DR7);
	
	
}

void dbg_enable_breakpoint(char bp_num, char islocal, char cond, void *addr, char len){
	dr7_t val;
	val.raw = 0;
	val.rsvd_set_1 = 1;
	
	unsigned char en_bits = islocal?0b01:0b10;
	val.raw |= (en_bits << (bp_num*2));
	
	
	
	switch(bp_num){
			case 0:
				dbg_write_debug_reg((unsigned long)addr,REG_DR0);
				val.bp0_cond = cond;
				val.bp0_len = len;
				
				break;
			case 1:
				dbg_write_debug_reg((unsigned long)addr, REG_DR1);
				val.bp1_cond = cond;
				val.bp1_len = len;
				
				break;
			case 2:
				dbg_write_debug_reg((unsigned long)addr, REG_DR2);
				val.bp2_cond = cond;
				val.bp2_len = len;
				
				break;
			case 3:
				dbg_write_debug_reg((unsigned long)addr, REG_DR3);
				val.bp3_cond = cond;
				val.bp3_len = len;
				
				break;
		
	} 
	
	dbg_write_debug_reg(*(unsigned long*)&val,REG_DR7);
	

}



void dbgconchar(char in){

    io_outb(0xe9, in);
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
