#include "syscalls.h"
#include <stddef.h>
int chdir(const char *path);
char *getcwd(char *buf, size_t size);

int pipe(int fildes[2]);
 int close(int fildes);

 int dup(int fildes);
int dup2(int fildes, int fildes2);

 int lstat(const char *path, struct stat *buf);

#define X_OK 0
