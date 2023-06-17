
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
char *csrc = (char *)src;
char *cdest = (char *)dest;

// Copy contents of src[] to dest[]
for (unsigned long i=0; i<n; i++)
    cdest[i] = csrc[i];
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
