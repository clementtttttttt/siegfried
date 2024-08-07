#include <sys/types.h>
#ifndef _STDC_SIGNAL_H
#define _STDC_SIGNAL_H

typedef int sig_atomic_t;


int sigemptyset(sigset_t *set);
int sigprocmask(int how, const sigset_t *restrict set, sigset_t *restrict old);
int kill(pid_t pid, int sig);

void (*signal( int sig, void (*handler) (int))) (int);
int raise( int sig );

#define SIG_SETMASK 42
#warning tmp sig_setmask

enum signals{
	SIGINT
};

#define SIG_DFL 0
#warning stub SIG_DFL

#endif
