void syscall_setup();

inline void syscall0(unsigned long func){
    asm("int $0xf0"::"D"(func));
}

inline void syscall1(unsigned long func,unsigned long in1){
    asm("int $0xf0"::"D"(func), "S"(in1));
}

inline void syscall2(unsigned long func,unsigned long in1, unsigned long in2){
    asm("int $0xf0"::"D"(func), "S"(in1), "d"(in2));
}

inline void syscall4(unsigned long func,unsigned long in1, unsigned long in2, unsigned long in3, unsigned long in4){
    register unsigned long in4_r8 __asm__("r8") = in4;
    asm("int $0xf0"::"D"(func), "S"(in1), "d"(in2), "c"(in3), "r"(in4_r8));
}
