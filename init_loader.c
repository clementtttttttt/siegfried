#include "tasks.h"
#include "syscall.h"

//note: RING 3 CODE. DO NOT USE ANY KRNL FUNCS WITHOTU SYSCALL.

void init_loader_end();
__attribute__((noinline, section(".init_loader"))) void init_loader(){

        char buf[512];

        syscall4(2, 1, 1, 1, (unsigned long)buf);
   
	syscall2(3,(unsigned long)buf, 512);

        syscall_disk_ent information;

	information.diskman_ent = 0;

        syscall1(1, (unsigned long) &information);

        syscall2(3, (unsigned long)information.uuid, information.uuid_len);

        syscall2(3, (unsigned long)"\nREACHED END OF INIT\r", 21);

    //    __builtin_trap();

        while(1){
        }

	return;

}

