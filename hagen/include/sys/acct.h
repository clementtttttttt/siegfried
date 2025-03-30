#include <stdint.h>
#include <time.h>
#define ACCT_COMM 16
typedef uint16_t comp_t;

struct acct
{
	char ac_flag;
	uint16_t ac_uid;
	uint16_t ac_gid;
	uint16_t ac_tty;
	uint32_t ac_btime;
	comp_t ac_utime;
	comp_t ac_stime;
	comp_t ac_etime;
	comp_t ac_mem;
	comp_t ac_io;
	comp_t ac_rw;
	comp_t ac_minflt;
	comp_t ac_majflt;
	comp_t ac_swaps;
	uint32_t ac_exitcode;
	char ac_comm[ACCT_COMM+1];
	char ac_pad[10];
};

