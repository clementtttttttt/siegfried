#ifndef HG_UNISTD_H
#define HG_UNISTD_H

#include "syscalls.h"
#include <sys/types.h>
#include <stddef.h>
#define SEEK_CUR 1
#define SEEK_SET 2
#define SEEK_END 4

#define _SC_CLK_TCK 1000000
#warning stub _SC_CLK_TCK
       long sysconf(int name);

int chdir(const char *path);
char *getcwd(char *buf, size_t size);

int pipe(int fildes[2]);
 int close(int fildes);

 int dup(int fildes);
int dup2(int fildes, int fildes2);

 int lstat(const char *path, struct stat *buf);
off_t lseek(int fd, off_t off, int whence);
int isatty(int fd);
       pid_t tcgetpgrp(int fd);
       int tcsetpgrp(int fd, pid_t pgrp);

       int setpgid(pid_t pid, pid_t pgid);
       pid_t getpgid(pid_t pid);

       pid_t getpgrp(void);                            /* POSIX.1 version */
 uid_t getuid(void);
       uid_t geteuid(void);

       gid_t getgid(void);
       gid_t getegid(void);
ssize_t write(int fildes, const void * buf, size_t count);

int brk(void* end_data_segment);
void *sbrk(intptr_t increment);


       //int getopt(int argc, char *argv[],
                  //const char *optstring);
		  
       int unlink(const char *pathname);
 unsigned long alarm(unsigned long seconds);
 int access(const char *path, int amode);
       pid_t getsid(pid_t pid);

int getopt(int argc, char * const argv[], const char *optstring);
extern char *optarg;
extern int optind, opterr, optopt;

#define X_OK 0
#endif
