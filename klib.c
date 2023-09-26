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


void mem_cpy(void *dest, void *src, unsigned long n)
{
// Typecast src and dest addresses to (char *)
	unsigned char *csrc = (unsigned char *)src;
	unsigned char *cdest = (unsigned char *)dest;

// Copy contents of src[] to dest[]
	while(n){
		*cdest++ = *csrc++;
		--n;
	}
}



void mem_set(void *dest, unsigned int val, unsigned long sz){
    char* d = dest;

    void* finish = (void*)(((unsigned long)dest )+ sz);

    if(!(sz % 16)){

        __uint128_t *d_128 = dest;
        unsigned char val_ext[16] = {val,val,val,val,val,val,val,val,val,val,val,val,val,val,val,val};

        while(d_128 != finish){
            *d_128++ = *((__uint128_t*)val_ext);
        }

    }
    else
    if(!(sz % 8 )){

        unsigned long *d_64 = dest;
        unsigned char val_ext[8] = {val,val,val,val,val,val,val,val};

        while(d_64 != finish){
            *d_64++ = *((unsigned long*)&val_ext);
        }


    }
    else{

        while(d != finish){

            *d++ = (char) val;

        }
    }

}


void klib_clear_var_cache(void *v){

    asm("clflush %0"::"m"(v));
}
//return 0 when not same

int mem_cmp(char *l, char *r, unsigned long sz){

    dbgnumout_hex((unsigned long)l);
    dbgnumout_hex((unsigned long)r);
    while(sz){
        if(*l != *r){
            return 0;
        }
        --sz;
        ++l;
        ++r;
    }
    return 1;
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
