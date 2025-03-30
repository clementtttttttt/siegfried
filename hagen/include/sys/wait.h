#define WCONTINUED 1
#define WNOHANG 2
#define WUNTRACED 4

#warning STUB WAIT MACROS

#define WIFEXITED(stat_val) 0
#define WIFSTATUS(stat_val) 0
#define WIFSIGNALED(stat_val) 0
#define WTERMSIG(stat_val) 0
#define WIFSTOPPED(stat_val) 0
#define WSTOPSIG(stat_val) 0
#define WIFCONTINUED(stat_val) 0
#define WEXITSTATUS(stat_val) 0

pid_t waitpid(pid_t pid, int *status, int options);
pid_t wait3 (int *stat_loc, int options, struct rusage *resource_usage);
