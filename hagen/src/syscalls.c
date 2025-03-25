#define NO_FORK
//userspace syscalls wrapper source

#include "syscalls.h"
#include <errno.h>
#include <unistd.h>
syscall_siegfried_file *fds[4096] = {0};
static unsigned long lowest_fd = 1;

int _sys_errno= 0;

int *__errno(){
	return &_sys_errno;
}


void _exit(int st){
	syscall1(sys_exit, (void*)(unsigned long)st);
	while(1){}
}

int close(int file){
	if(file == 0) return -EINVAL;

	if(file < lowest_fd) lowest_fd = file;
	syscall1(sys_close, fds[lowest_fd]);

	return 0; 	//silently exit
}

char **environ; /* pointer to array of char * strings that define the current environment variables */
int execve(char *name, char **argv, char **env){
	long retval = (long)syscall4(sys_spawn, name, argv , env, 0);
	if(!retval){ //error
		return -errno;
	}

	syscall1(sys_exit, 0);
}


int fork(){ //we havent fork
	return -ENOSYS;
}

ssize_t write(int fd, const void *buf, size_t count){
	if(fd == 0) return -EINVAL;
	
	return syscall4(sys_write, fds[fd], buf, count, 0); //0 attr
	

}

/*
int fstat(int file, struct stat *st){
	syscall_siegfried_stat statstruct;
	int retval = syscall2(sys_stat, fds[lowest_fd], &statstruct);	
 	
 	st->st_dev = statstruct.disk_inode;
 	st->st_ino = statstruct.inode;
 	
 	return retval;
}
*/

uid_t getuid(){
	//return syscall0(sys_get_uid);
	return 0;
	#warning stub getuid

}

int getpid(){
	return syscall0(sys_get_tid);
}

int opendir(){
	
}
void *mmap(void *addr, size_t len, int prot, int flags, int fd, off_t offset){
	syscall6(sys_mmap,addr, len, prot, flags, fd, offset);
}

int isatty(int file);
int kill(int pid, int sig){
	#warning stub kill
	return 0;
}
int link(char *old, char *new);
int open(const char *name, int flags, ...){
	
	return -EINVAL; 
	#warning stub open

}

unsigned long alarm(unsigned long sec){

	return 0;
}

mode_t umask(mode_t cmask){
#warning stub umask
	return 0;

}

int read(int file, char *buf, int len){
	syscall_siegfried_file *f = fds[file];
	syscall4(sys_read, f, buf, len, 0);
	
	#warning TODO proper errcode return
	return 0; 
}
/*
caddr_t sbrk(int incr);
int stat(const char *file, struct stat *st);
clock_t times(struct tms *buf);
*/
//int unlink(char *name);
int wait(int *status);
//int write(int file, char *ptr, int len);
int gettimeofday(struct timeval  *__restrict p, void *__restrict z);
