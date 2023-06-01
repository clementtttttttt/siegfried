
#include "klib.h"
#include "debug.h"

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
    for(unsigned long i=0;i<sz;++i){
        d[i] = (unsigned char)val;
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
