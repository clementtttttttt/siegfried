#include "syscalls.h"


int main(){

//        char buf[512];

	
        
    //    __builtin_trap();

        
	syscall2(2, "LOADED FROM DISKKKKKKKKKK", (void*)20);

		
	execve("/sbin/sfinit", 0, 0);
	

	return 1;

}
