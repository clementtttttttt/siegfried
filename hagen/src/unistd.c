#include "syscalls.h"
#include <stddef.h>
char *getcwd(char *buf, size_t size){
	
	return syscall2(sys_getcwd, buf,(void*) size);

}

int chdir(const char *path){
	return (int) syscall1(sys_chdir, (void*)path);
}
