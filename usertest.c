#include "syscall.h"

int main(){

//        char buf[512];

	
        syscall2(3, (unsigned long)"\nLOADED FROM DISKKKKKKKKKK HELLO WORLD \r", 34);

    //    __builtin_trap();

        while(1){
        }

	return 1;

}
