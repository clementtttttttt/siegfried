
void mem_cpy(void *dest, void *src, unsigned long n)
{
// Typecast src and dest addresses to (char *)
char *csrc = (char *)src;
char *cdest = (char *)dest;

// Copy contents of src[] to dest[]
for (unsigned long i=0; i<n; i++)
    cdest[i] = csrc[i];
}
