#include "syscalls.h"
#include <stddef.h>
char *getcwd(char *buf, size_t size){
	
	return syscall2(sys_getcwd, buf, size);

}

int chdir(const char *path){
	return syscall1(sys_chdir, path);
}
