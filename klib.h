
#ifndef KLIB_H
#define KLIB_H
#include "types.h"

void mem_cpy(void *dest, void *src, unsigned long n);
void *mem_set(void *dest, unsigned int val, unsigned long sz);
int mem_cmp(const void *vl, const void *vr, unsigned long n);
unsigned long str_len(char *in);
void halt_and_catch_fire();

typedef struct str_tok_result{
    unsigned long off;
    unsigned long sz;
}str_tok_result;

void str_tok(char *str, char delim, str_tok_result *off);
unsigned  long atoi_w_sz(char *str, size_t sz);
void itoa_w_sz(char *buf, unsigned long in, size_t sz);
void klib_clear_var_cache(void *v);
unsigned long find_num_len(char *str);



static inline void mmio_pokel(volatile void *addr, unsigned int value){
	asm volatile inline("movl %%edx, (%%rbx)"::"b"(addr), "d"(value));
}
#endif
