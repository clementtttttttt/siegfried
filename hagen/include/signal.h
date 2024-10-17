#include <sys/types.h>
#ifndef _STDC_SIGNAL_H
#define _STDC_SIGNAL_H


#define _NSIG             32
#define NSIG		_NSIG


typedef struct __stack_t_struct{
        void *ss_sp;
        size_t ss_size;
        int ss_flags;
} stack_t;

typedef int sig_atomic_t;

#define SIGHUP		 1
#define SIGINT		 2
#define SIGQUIT		 3
#define SIGILL		 4
#define SIGTRAP		 5
#define SIGABRT		 6
#define SIGIOT		 6
#define SIGUNUSED	 7
#define SIGFPE		 8
#define SIGKILL		 9
#define SIGUSR1		10
#define SIGSEGV		11
#define SIGUSR2		12
#define SIGPIPE		13
#define SIGALRM		14
#define SIGTERM		15
#define SIGSTKFLT	16
#define SIGCHLD		17
#define SIGCONT		18
#define SIGSTOP		19
#define SIGTSTP		20
#define SIGTTIN		21
#define SIGTTOU		22
#define SIGXCPU 	23
#define SIGXFSZ		24
       int setuid(uid_t uid);
       int setgid(gid_t gid);
int sigemptyset(sigset_t *set);
int sigprocmask(int how, const sigset_t *restrict set, sigset_t *restrict old);
int kill(pid_t pid, int sig);
       int sigsuspend(const sigset_t *mask);

void (*signal( int sig, void (*handler) (int))) (int);
int raise( int sig );

int    sigaddset(sigset_t *, int);

#define SIG_UNBLOCK 1
#define SIG_BLOCK 2

int    sigaltstack(const stack_t *restrict, stack_t *restrict);

#define SIG_SETMASK 42
#warning tmp sig_setmask

#define MINSIGSTKSZ 2048
#define SIGSTKSZ 8192

#define SIG_DFL (void*)2
#define SIG_ERR (void*)1
#define SIG_IGN (void*)0
#warning stub SIG_DFL, SIG_ERR, SIG_IGN

#define SA_ONSTACK 1
#define SA_SIGINFO 2



      struct sigaction {
               void     (*sa_handler)(int);
//               void     (*sa_sigaction)(int, siginfo_t *, void *);
               sigset_t   sa_mask;
               int        sa_flags;
               void     (*sa_restorer)(void);
           };
int sigaction(int sig, struct sigaction *act, struct sigaction *oldact);
int sigfillset(sigset_t *mask);

typedef struct __siginfo_struct{
	int si_code;
	int si_signo;
} siginfo_t;

#endif
