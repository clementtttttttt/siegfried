void dbgconout(char* str);
void dbgnumout_bin(unsigned long in);
void dbgnumout_hex(unsigned long in);
void dbg_crash();

typedef union dr7_struct{
	
	struct{
	unsigned long local_bp0_en : 1,
				global_bp0_en : 1,
				local_bp1_en : 1,
				global_bp1_en : 1,
				local_bp2_en : 1,
				global_bp2_en : 1,
				local_bp3_en : 1,
				global_bp3_en : 1,
				
				local_exact_en_386 : 1,
				global_exact_en_386 : 1,
				rsvd_set_1 : 1,
				rtm_debug_en_tsx : 1,
				enable_smm_break_386_486 : 1,
				
				general_detect_en : 1,
				
				rsvd_set_0_0 : 2,
				
				bp0_cond : 2,
				bp0_len : 2,
				bp1_cond : 2,
				bp1_len : 2,
				bp2_cond : 2,
				bp2_len : 2,
				bp3_cond : 2,
				bp3_len : 2,
				
				rsvd_set_0_1 : 32;
	};
	unsigned long raw;
	
	
	
}__attribute__((packed))dr7_t;

enum dbgregs{
	REG_DR0,
	REG_DR1,
	REG_DR2,
	REG_DR3,
	REG_DR6,
	REG_DR7
	
};

enum bpcond{
	BREAK_ON_EXEC=0,
	BREAK_ON_WRITE,
	BREAK_ON_IO_READ_WRITE,
	BREAK_ON_READ_WRITE
	
};

enum bplen{
	BP_LEN_1=0,
	BP_LEN_2,
	BP_LEN_8,
	BP_LEN_4
};

void dbg_enable_breakpoint(char bp_num, char islocal, char cond, void *addr, char len);

void dbg_disable_breakpoint(char bp_num, char islocal);
