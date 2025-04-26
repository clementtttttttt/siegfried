#include "syscallno.h"



//enum syscalls{
//	sys_exit = 0, sys_sleep, sys_print_w_sz, sys_disk_get_next, sys_disk_read, sys_disk_write, sys_read, sys_write, sys_open, sys_spawn, sys_disk_get_root, sys_get_tid, sys_stat, sys_getcwd
//};
void syscall_setup();
char *sys_basename(char *path);
char *sys_dirname(char *buf,char *path);
int syscall_open_dir(char* path, siegfried_dir *in);
pid_t syscall_get_tid(void);
void syscall_pmsg_1(pid_t dest, pid_t src, syscall_msg_type_t t);
void syscall_close_dir(siegfried_dir *in);
syscall_msg_t * syscall_gmsg(void);
