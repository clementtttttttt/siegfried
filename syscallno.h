#ifndef _SYSCALLNO_H_
#define _SYSCALLNO_H_
#define SYSCALL_REBOOT_MAGIC 0xC1EA1EBE1550CA55
#define SYSCALL_REBOOT_MAGIC2 0xCAFEBABE1E560001
typedef enum syscalls_t_enum{
	sys_exit = 0, sys_sleep, sys_print_w_sz, sys_disk_get_next, sys_disk_read, sys_disk_write, sys_read, sys_write, sys_open, sys_spawn, sys_disk_get_root, sys_get_tid, sys_stat, sys_close, sys_open_dir, sys_mmap, sys_getcwd, sys_read_dir, sys_close_dir, sys_chdir, sys_reboot, sys_gmsg
} syscalls_t;

inline  static  void* syscall0(syscalls_t func){  
    void* retval;
    asm volatile ("int $0xf0":"=a"(retval):"D"(func));
    return retval;
}

inline static  void* syscall1(syscalls_t func,void* in1){
    void* retval;
    asm volatile ("int $0xf0":"=a"(retval):"D"(func), "S"(in1));
    return retval;
}


inline  static void*  syscall2(syscalls_t func,void* in1,void* in2){
    void* retval;
    asm volatile ("int $0xf0":"=a"(retval):"D"(func), "S"(in1), "d"(in2));
	return (void*) retval;
}


inline static void* syscall3(syscalls_t func,void* in1, void* in2,void*  in3){
    unsigned long retval;
    asm volatile ("int $0xf0":"=a"(retval):"D"(func), "S"(in1), "d"(in2), "c"(in3));
	return (void*) retval;
}

#define syscall5(func,in1, in2,  in3, in4, in5)\
    register unsigned long in4_r8 __asm__("r8") = in4;\
    register unsigned long in5_r9 __asm__("r9") = in5;\
    asm volatile ("int $0xf0"::"D"(func), "S"(in1), "d"(in2), "c"(in3), "r"(in4_r8), "r"(in5_r9));


inline static void* syscall4(syscalls_t func,void* in1, void* in2,void*  in3, void* in4){
    register unsigned long in4_r8 __asm__("r8") = (unsigned long)in4;
    unsigned long retval;
    asm volatile ("int $0xf0":"=a"(retval):"D"(func), "S"(in1), "d"(in2), "c"(in3), "r"(in4_r8));
	return(void*) retval;
}


inline static void* syscall6(syscalls_t func,void* in1, void* in2, void* in3, void* in4, void* in5,void* in6){
    
    unsigned long retval;
    asm volatile ("mov %4, %%r8; mov %5, %%r9; push %6 ;int $0xf0":"=a"(retval):"D"(func), "S"(in1), "d"(in2), "c"(in3), "r"(in4), "r"(in5), "r"(in6));
    return (void*)retval;
}

typedef enum{
		MSG_NULL, MSG_TERM, MSG_CHILD_DIED
	
} syscall_msg_type_t;


typedef struct syscall_msg_t{
	pid_t src;
	pid_t dest;
	syscall_msg_type_t type;
	char spec_dat[0];
	
} syscall_msg_t;

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

typedef struct syscall_disk_ent{
    char uuid[16];
    int uuid_len;
    unsigned long inode;
    void* diskman_ent;
}syscall_disk_ent;
#endif
