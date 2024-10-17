#ifndef __SF_SYSCALL_H
#define __SF_SYSCALL_H
inline static unsigned long syscall0(unsigned long func){  
    unsigned long retval;
    asm("int $0xf0":"=a"(retval):"D"(func));
    return retval;
}

inline static unsigned long syscall1(unsigned long func,void* in1){
    unsigned long retval;
    asm("int $0xf0":"=a"(retval):"D"(func), "S"(in1));
    return retval;
}


inline static unsigned long  syscall2(unsigned long func,void* in1,void* in2){
    unsigned long retval;
    asm("int $0xf0":"=a"(retval):"D"(func), "S"(in1), "d"(in2));
	return retval;
}


#define syscall5(func,in1, in2,  in3, in4, in5)\
    register unsigned long in4_r8 __asm__("r8") = in4;\
    register unsigned long in5_r9 __asm__("r9") = in5;\
    asm("int $0xf0"::"D"(func), "S"(in1), "d"(in2), "c"(in3), "r"(in4_r8), "r"(in5_r9));


inline static unsigned long syscall4(unsigned long func,void* in1, void* in2,void*  in3, void* in4){
    register unsigned long in4_r8 __asm__("r8") = (unsigned long)in4;
    unsigned long retval;
    asm("int $0xf0":"=a"(retval):"D"(func), "S"(in1), "d"(in2), "c"(in3), "r"(in4_r8));
	return retval;
}


inline unsigned long syscall6(unsigned long func,void* in1, void* in2, void* in3, void* in4, void* in5,void* in6){
    
    unsigned long retval;
    asm("mov %4, %%r8; mov %5, %%r9; push %6 ;int $0xf0":"=a"(retval):"D"(func), "S"(in1), "d"(in2), "c"(in3), "r"(in4), "r"(in5), "r"(in6));
    return retval;
}

typedef struct syscall_siegfried_file syscall_siegfried_file;


typedef struct syscall_disk_ent{
    char uuid[16];
    int uuid_len;
    unsigned long inode;
    void* diskman_ent;
}syscall_disk_ent;

typedef struct syscall_siegfried_stat{

	unsigned long	perms;
	unsigned long 	inode;
	unsigned long	disk_inode;
	unsigned long	links;
	unsigned long	uid;
	unsigned long	gid;
	unsigned long	size;
	unsigned long	atime_in_ms;
	unsigned long	mtime_in_ms;
	unsigned long	ctime_in_ms;
} syscall_siegfried_stat;

enum syscalls{
	sys_exit = 0, sys_sleep, sys_print_w_sz, sys_disk_get_next, sys_disk_read, sys_disk_write, sys_read, sys_write, sys_open, sys_spawn, sys_disk_get_root, sys_get_tid, sys_stat, sys_close, sys_open_dir
};
void syscall_setup();
void _exit(int stat);
int close_(int file);
int execve(char *name, char **argv, char **env);
int fstat(int file, struct stat *st);
int read(int file, char *buf, int len);

int getpid();

typedef struct syscall_siegfried_dir syscall_siegfried_dir;
#endif
