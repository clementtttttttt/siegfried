#include "syscalls.h"


int main(){

//        char buf[512];

	
        
    //    __builtin_trap();

        
                syscall2(2, (unsigned long)"LOADED FROM DISKKKKKKKKKK", 34);

	syscall2(2, (unsigned long)"siegfried syscall testing\n", sizeof("siegfried syscall testing\n"));
	
	unsigned long disk_root = syscall0(sys_disk_get_root);
	

	return 1;

}
