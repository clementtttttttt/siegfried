#include "syscallno.h"



//enum syscalls{
//	sys_exit = 0, sys_sleep, sys_print_w_sz, sys_disk_get_next, sys_disk_read, sys_disk_write, sys_read, sys_write, sys_open, sys_spawn, sys_disk_get_root, sys_get_tid, sys_stat, sys_getcwd
//};
void syscall_setup();

#define SYSCALL_REBOOT_MAGIC 0xC1EA1EBE1550CA55
#define SYSCALL_REBOOT_MAGIC2 0xCAFEBABE1E560001
