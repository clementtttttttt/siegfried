#include "draw.h"
#include "klib.h"
#include "debug.h"
#include "kb.h"

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
	unsigned char *d = dest;
	const unsigned char *s = src;

#ifdef __GNUC__

#if __BYTE_ORDER == __LITTLE_ENDIAN
#define LS >>
#define RS <<
#else
#define LS <<
#define RS >>
#endif

	typedef unsigned int __attribute__((__may_alias__)) u32;
	unsigned int w, x;

	for (; (long unsigned int)s % 4 && n; n--) *d++ = *s++;

	if ((long unsigned int)d % 4 == 0) {
		for (; n>=16; s+=16, d+=16, n-=16) {
			*(u32 *)(d+0) = *(u32 *)(s+0);
			*(u32 *)(d+4) = *(u32 *)(s+4);
			*(u32 *)(d+8) = *(u32 *)(s+8);
			*(u32 *)(d+12) = *(u32 *)(s+12);
		}
		if (n&8) {
			*(u32 *)(d+0) = *(u32 *)(s+0);
			*(u32 *)(d+4) = *(u32 *)(s+4);
			d += 8; s += 8;
		}
		if (n&4) {
			*(u32 *)(d+0) = *(u32 *)(s+0);
			d += 4; s += 4;
		}
		if (n&2) {
			*d++ = *s++; *d++ = *s++;
		}
		if (n&1) {
			*d = *s;
		}
		return;
	}

	if (n >= 32) switch ((long unsigned int)d % 4) {
	case 1:
		w = *(u32 *)s;
		*d++ = *s++;
		*d++ = *s++;
		*d++ = *s++;
		n -= 3;
		for (; n>=17; s+=16, d+=16, n-=16) {
			x = *(u32 *)(s+1);
			*(u32 *)(d+0) = (w LS 24) | (x RS 8);
			w = *(u32 *)(s+5);
			*(u32 *)(d+4) = (x LS 24) | (w RS 8);
			x = *(u32 *)(s+9);
			*(u32 *)(d+8) = (w LS 24) | (x RS 8);
			w = *(u32 *)(s+13);
			*(u32 *)(d+12) = (x LS 24) | (w RS 8);
		}
		break;
	case 2:
		w = *(u32 *)s;
		*d++ = *s++;
		*d++ = *s++;
		n -= 2;
		for (; n>=18; s+=16, d+=16, n-=16) {
			x = *(u32 *)(s+2);
			*(u32 *)(d+0) = (w LS 16) | (x RS 16);
			w = *(u32 *)(s+6);
			*(u32 *)(d+4) = (x LS 16) | (w RS 16);
			x = *(u32 *)(s+10);
			*(u32 *)(d+8) = (w LS 16) | (x RS 16);
			w = *(u32 *)(s+14);
			*(u32 *)(d+12) = (x LS 16) | (w RS 16);
		}
		break;
	case 3:
		w = *(u32 *)s;
		*d++ = *s++;
		n -= 1;
		for (; n>=19; s+=16, d+=16, n-=16) {
			x = *(u32 *)(s+3);
			*(u32 *)(d+0) = (w LS 8) | (x RS 24);
			w = *(u32 *)(s+7);
			*(u32 *)(d+4) = (x LS 8) | (w RS 24);
			x = *(u32 *)(s+11);
			*(u32 *)(d+8) = (w LS 8) | (x RS 24);
			w = *(u32 *)(s+15);
			*(u32 *)(d+12) = (x LS 8) | (w RS 24);
		}
		break;
	}
	if (n&16) {
		*d++ = *s++; *d++ = *s++; *d++ = *s++; *d++ = *s++;
		*d++ = *s++; *d++ = *s++; *d++ = *s++; *d++ = *s++;
		*d++ = *s++; *d++ = *s++; *d++ = *s++; *d++ = *s++;
		*d++ = *s++; *d++ = *s++; *d++ = *s++; *d++ = *s++;
	}
	if (n&8) {
		*d++ = *s++; *d++ = *s++; *d++ = *s++; *d++ = *s++;
		*d++ = *s++; *d++ = *s++; *d++ = *s++; *d++ = *s++;
	}
	if (n&4) {
		*d++ = *s++; *d++ = *s++; *d++ = *s++; *d++ = *s++;
	}
	if (n&2) {
		*d++ = *s++; *d++ = *s++;
	}
	if (n&1) {
		*d = *s;
	}
	return ;
#endif

	for (; n; n--) *d++ = *s++;
	return ;
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

void halt_and_catch_fire(){
	draw_string("H&CF: press any key to reboot");
	kb_wait_and_pop_char();
	draw_string("H&CF: rebooting NOW");
	asm("xor %rax, %rax;mov %rax,%cr3");
}


void klib_clear_var_cache(void *v){

    asm("clflush %0"::"m"(v));
}

int mem_cmp(const void *vl, const void *vr, size_t n)
{
	const unsigned char *l=vl, *r=vr;
	for (; n && *l == *r; n--, l++, r++);
	return n ? *l-*r : 0;
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

    off->sz = i - off->off;




}


