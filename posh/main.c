#include <syscalls.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
int main(){

//        char buf[512];

	
        
    //    __builtin_trap();
    /*if(getpid() != 1){
		while(1){
			
			puts("HEY!");
			syscall1(sys_sleep, (void*)100);
			
		}
	}*/

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
		char cwd[PATH_MAX] = {0};
		getcwd(cwd, PATH_MAX);
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
							if(b_idx == 0){
						continue; //skip, empty buffer
					}
				
		
		char *saveptr = 0;
		
		char *next_tok(){
			return strtok_r (0, " ", &saveptr);
		}
		
		char *tok = strtok_r(buffer, " ", &saveptr);
		do{
				char *dir = next_tok();
				
				if(dir == NULL){
						dir = "./";
						
				}
			if(tok[0] == '.' || tok[0] == '/'){
				pid_t ret = spawn(tok, 0,0,0);
				puts("PID=");
				if(ret<=0){
					puts(strerror(-ret));
					puts("\n");
					break;
				}
				syscall_msg_t msg; 
				gmsg(&msg);
				syscall_child_died_type_t code = *((syscall_child_died_type_t*)&msg.spec_dat);
				if(code){
					puts("Program crashed\n");
				}
			}
			else
			if(!strcmp(tok, "exit")){
				asm("syscall");
				//exit(0);
			}
			else
			if(!strcmp(tok, "reboot")){
				
				syscall3(sys_reboot, 0, (void*)SYSCALL_REBOOT_MAGIC, (void*)SYSCALL_REBOOT_MAGIC2);
			}
			else if(!strcmp(tok, "cd")){
			//	char buf[PATH_MAX] = {0};
			//	memset(buf, 0, PATH_MAX);
			//	if(dir[0] != '/'){ //absolute
				//	getcwd(buf, PATH_MAX);
				//}
				//strcat(buf, dir);
	
				int ret = chdir(dir);
				if(ret!=0){
					char *test = strerror(-ret);
					puts(test);
					puts("\n");
				}
			}
				else
			if(!strcmp(tok, "ls")){
				//if(

				
				syscall_siegfried_dir curr_dir;
				int ret = opendir(dir, &curr_dir);
				if(ret != 0){
					if(ret == -ENOTDIR){
						//it's a file
						puts(dir);
						puts("\n");
						break;
					}
					else{
						char *test = strerror(-ret);
							puts(test);

						puts("\n");
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
				
				syscall1(sys_close_dir, &curr_dir);
				
				//char test[][NAME_MAX] = k_obj_alloc(sizeof(char[256]) * count);
				
				
			}
			else{
			puts(tok);
			puts(": Unknown command");
			puts("\n");
		}
		}
		while(tok=next_tok());
		
	}

	
	

	return 1;

}
