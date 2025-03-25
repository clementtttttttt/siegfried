#include "syscalls.h"
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
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
	
	//char *test = dlmalloc(16);
	
	char buffer[128];

	while(1){
		char cwd[256];
		getcwd(cwd, 256);
		puts(cwd);
		puts("# ");	
		
		unsigned char b_idx = 0;
		memset(buffer, 0, 128);

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
		
		
		char *saveptr = 0;
		
		char *next_tok(){
			return strtok_r (0, " ", &saveptr);
		}
		
		char *tok = strtok_r(buffer, " ", &saveptr);
		do{

			
			if(!strcmp(tok, "ls")){
				//if(
				char *dir = next_tok();
				
				if(dir == NULL){
						dir = ".";
				}
				
				syscall_siegfried_dir curr_dir;
				long ret = (long)syscall2(sys_open_dir, dir, &curr_dir);
				if(ret < 0){
					if(ret == -ENOTDIR){
						//it's a file
						puts(dir);
						puts("\n");
						break;
					}
					else{
						puts("Error opening directory\n");
						break;
					}
				}
				

				syscall_siegfried_dirnames_t d[curr_dir.num_files];
				memset(d, 0, sizeof(syscall_siegfried_dirnames_t[curr_dir.num_files]));
				
				syscall2(sys_read_dir, &curr_dir, &d);
				
				
				for(int i=0;i<curr_dir.num_files;++i){
					puts(d[i]);
					puts("\n");
				}
				//char test[][NAME_MAX] = k_obj_alloc(sizeof(char[256]) * count);
				
				
			}
			
		}
		while(tok=next_tok());
		
	}

	
	

	return 1;

}
