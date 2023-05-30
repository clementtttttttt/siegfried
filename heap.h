
typedef struct _KHEAPBLOCKSS {
	struct _KHEAPBLOCKSS	*next;
	unsigned int					top;
	unsigned int					max;
	unsigned long					size;			/* total size in bytes including this header */
	unsigned int extended;
} KHEAPBLOCKSS;

typedef struct _KHEAPSS {
	KHEAPBLOCKSS			*fblock;
	unsigned int					bsize;
} KHEAPSS;

void k_heapSSInit(KHEAPSS *heap, unsigned long bsize);

void *k_heapSSAlloc(KHEAPSS *heap, unsigned long size);
int k_heapSSAddBlock(KHEAPSS *heap, unsigned long addr, unsigned long size);
