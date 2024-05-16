#include "syscalls.h"


int main(){

//        char buf[512];

	
        
    //    __builtin_trap();

        
                syscall2(2, "LOADED FROM DISKKKKKKKKKK", (void*)34);

	syscall2(2, "siegfried syscall testing\n", (void*)sizeof("siegfried syscall testing\n"));
	
	execve("/sfinit", 0, 0);
	

	return 1;

}
