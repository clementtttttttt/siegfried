void syscall_setup();

#define syscall0(func)\
    asm("int $0xf0"::"D"(func));


#define syscall1(func,in1)\
    asm("int $0xf0"::"D"(func), "S"(in1));


#define  syscall2(func,in1, in2)\
    asm("int $0xf0"::"D"(func), "S"(in1), "d"(in2));


#define syscall4(func,in1, in2,  in3, in4)\
    register unsigned long in4_r8 __asm__("r8") = in4;\
    asm("int $0xf0"::"D"(func), "S"(in1), "d"(in2), "c"(in3), "r"(in4_r8));


typedef struct syscall_disk_ent{
    char uuid[16];
    int uuid_len;
    unsigned long inode;
    void* diskman_ent;
}syscall_disk_ent;


