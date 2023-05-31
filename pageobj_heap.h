
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

void k_pageobj_init_heap(KHEAPSS *heap, unsigned long bsize);

void *k_pageobj_alloc(KHEAPSS *heap, unsigned long size);
int k_pageobj_add_heapblk(KHEAPSS *heap, unsigned long addr, unsigned long size);
void k_pageobj_free(KHEAPSS *heap, void *ptr) ;

void k_pageobj_heap_setup();
