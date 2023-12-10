#include "draw.h"
#include "klib.h"
#include "debug.h"

unsigned long str_len(char *in){
    char *old=in;
    while(*in){
        ++in;
    }

    return in - old;

}




#ifdef HAVE_CC_INHIBIT_LOOP_TO_LIBCALL
# define inhibit_loop_to_libcall \
    __attribute__ ((__optimize__ ("-fno-tree-loop-distribute-patterns")))
#else
# define inhibit_loop_to_libcall
#endif


void inhibit_loop_to_libcall mem_cpy(void *dest, void *src, unsigned long n)
{
// Typecast src and dest addresses to (char *)
        unsigned char *csrc = (unsigned char *)src;
        unsigned char *cdest = (unsigned char *)dest;

// Copy contents of src[] to dest[]
      	while(n--){
		*cdest++ = *csrc++;
		
	}
}

//make the compiler shut up
void *memcpy(void *dest, void *src, unsigned long n){
	mem_cpy(dest, src, n);
	return dest;
}

void inhibit_loop_to_libcall *mem_set(void *dest, unsigned int c, unsigned long n){
  unsigned char *s = dest;
	size_t k;

	/* Fill head and tail with minimal branching. Each
	 * conditional ensures that all the subsequently used
	 * offsets are well-defined and in the dest region. */

	if (!n) return dest;
	s[0] = c;
	s[n-1] = c;
	if (n <= 2) return dest;
	s[1] = c;
	s[2] = c;
	s[n-2] = c;
	s[n-3] = c;
	if (n <= 6) return dest;
	s[3] = c;
	s[n-4] = c;
	if (n <= 8) return dest;

	/* Advance pointer to align it at a 4-byte boundary,
	 * and truncate n to a multiple of 4. The previous code
	 * already took care of any head/tail that get cut off
	 * by the alignment. */

	k = -(unsigned long)s & 3;
	s += k;
	n -= k;
	n &= -4;
for (; n; n--, s++) *s = c;


	return dest;
}



void klib_clear_var_cache(void *v){

    asm("clflush %0"::"m"(v));
}
//return 0 when not same

int mem_cmp(char *l, void *r, unsigned long sz){
	unsigned char *ul = (unsigned char*)l;
	unsigned char *ur = r;
	for(unsigned long i=0;i < sz && ((*ul - *ur) == 0);++i);
	return !(*ul-*ur);
}

unsigned long atoi_w_sz(char *str, unsigned long sz){
    unsigned long tot = 0;

    for(unsigned long i=0;i<sz;++i){
        tot = tot * 10 + str[i] - '0';
    }

    return tot;
}

void str_tok(char *str, char delim, str_tok_result *off){

    if(off->off > str_len(str) || str_len(str) == 0){off->off = 0; off->sz = 0; return;}

    if(off->sz != 0){ //non-first search
        do{
            ++off->off;
        }
        while(str[off->off] && str[off->off]!= delim);
        ++off->off;

    }

    unsigned long i;

    if(off->off == 0 && (str[off->off] == delim || str[off->off] == '\0'))
    {

        off->off = 0;
        off->sz = 1;

        return;

    }

    for(i=off->off+1;i < str_len(str);++i){

        if(str[i] == delim)break;
        if(str[i] == 0) break;


    }
    if(i > str_len(str)){off->off = 0; off->sz = 0; return;}

    off->sz = i - off->off+1;




}


