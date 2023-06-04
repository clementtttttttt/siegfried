void syscall_setup();

inline void syscall0(unsigned long func){
    asm("int $0xf0"::"D"(func));
}

inline void syscall1(unsigned long func,unsigned long in1){
    asm("int $0xf0"::"D"(func), "S"(in1));
}

