 void syscall2(unsigned long func,unsigned long in1, unsigned long in2){
    asm("int $0xf0"::"D"(func), "S"(in1), "d"(in2));
}


int main(){

//        char buf[512];

	while(1){}

        syscall2(3, (unsigned long)"\nREACHED END OF INIT! reaching end of init! \r", 34);

    //    __builtin_trap();

        while(1){
        }

	return 1;

}
