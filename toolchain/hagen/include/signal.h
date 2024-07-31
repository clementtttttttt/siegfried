#include <sys/types.h>
typedef int sig_atomic_t;


int sigemptyset(sigset_t *set);
int sigprocmask(int how, const sigset_t *restrict set, sigset_t *restrict old);
int kill(pid_t pid, int sig);
       
#define SIG_SETMASK 42
#warning tmp sig_setmask
