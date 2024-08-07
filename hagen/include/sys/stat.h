struct stat{
	__uint128_t st_mode;	
};

#define S_ISDIR(s) 0
#warning stub S_ISDIR and stub S_ISREG
#define S_ISREG(s) 0
int stat(const char *restrict pathname, struct stat *restrict statbuf);
