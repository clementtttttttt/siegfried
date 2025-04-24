#include "syscallno.h"



//enum syscalls{
//	sys_exit = 0, sys_sleep, sys_print_w_sz, sys_disk_get_next, sys_disk_read, sys_disk_write, sys_read, sys_write, sys_open, sys_spawn, sys_disk_get_root, sys_get_tid, sys_stat, sys_getcwd
//};
void syscall_setup();

pid_t syscall_get_tid(void);
