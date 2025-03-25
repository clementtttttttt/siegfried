#include "syscalls.h"
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
int main(){

//        char buf[512];

	
        
    //    __builtin_trap();

       	char msg1[] = "Piece-of-shell v1.0\nA badly written duct-taped shell for SIEGFRIED\n\n"; 

	puts(msg1);
	
	
	char input;

	syscall_siegfried_file *in_file = (syscall_siegfried_file*)syscall1(sys_open, "//1/ps2kb");
	
	
	if(in_file == -EINVAL){
		syscall2(2,"Failed to open stdin", (void*)20);
		while(1){}
	}
	
	char buffer[128];
	unsigned char b_idx;
	//memset(buffer, 0, 128);

	while(1){
		char cwd[256];
		getcwd(cwd, 256);
		puts(cwd);
		puts("# ");	
		
		b_idx = 0;
	
		while(1){
			syscall4(sys_read, in_file, &input,(void*)1,0);
			
			if(input == 0xa){
					break;
			}
			if(input == 8){
				if(b_idx>0){
					puts("\b\ \b");
					buffer[b_idx--] = 0;

				}
			}
			else{
				syscall2(sys_print_w_sz, &input, (void*) 1);
				
				buffer[b_idx++] = input;
			}
			
		}
		puts("\n");
		
		char *saveptr;
		strtok_r(buffer, " ", &saveptr);
		
		
	}

	
	

	return 1;

}
