#ifndef __SF_SYSCALL_H
#define __SF_SYSCALL_H

#include <sys/types.h>

#include "syscallno.h"


#define NAME_MAX 256 //TODO: MERGE LIMITS TO DEDICATED HEADER
typedef struct syscall_siegfried_dir{

    unsigned long num_files;
	ino_t inode;
	char name[NAME_MAX];
	

}syscall_siegfried_dir;

typedef char syscall_siegfried_dirnames_t[NAME_MAX];


typedef struct syscall_siegfried_file syscall_siegfried_file;




void syscall_setup();
void _exit(int stat);
int execve(char *name, char **argv, char **env);
int fstat(int file, struct stat *st);
int read(int file, char *buf, int len);
pid_t spawn(char *path, char** argv, char** env, unsigned long attrs);
int opendir(char* path, syscall_siegfried_dir *in);
void syscall_pmsg_1(pid_t dest, pid_t src, syscall_msg_type_t t);
void gmsg(syscall_msg_t *m );
int getpid();

extern syscall_siegfried_file *fds[4096];


#endif
