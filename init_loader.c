#include "tasks.h"
#include "syscall.h"

//note: RING 3 CODE. DO NOT USE ANY KRNL FUNCS WITHOTU SYSCALL.

void init_loader_end();
__attribute__((noinline, section(".init_loader"))) void init_loader(){

//        char buf[512];



        syscall2(3, (unsigned long)"\nREACHED END OF INIT! reaching end of init! \r", 34);

    //    __builtin_trap();

        while(1){
        }

	return;

}

