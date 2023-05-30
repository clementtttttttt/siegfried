#include "heap.h"
#include "page.h"
#include "debug.h"

void k_heapSSInit(KHEAPSS *heap, unsigned long bsize) {
	heap->fblock = 0;
	heap->bsize = bsize;
}

int k_heapSSAddBlock(KHEAPSS *heap, unsigned long addr, unsigned long size) {

	KHEAPBLOCKSS		*b;
	unsigned long				x;
	unsigned long				*stack;
	unsigned long				stacksz;
	unsigned long				slotres;

	b = (KHEAPBLOCKSS*)addr;
	b->next = heap->fblock;
	heap->fblock = b;

	b->size = size;

//	size = size - sizeof(KHEAPBLOCKSS);

	size = size - heap->bsize;

	b->max = size / (heap->bsize);

	stacksz = b->max * 8;
	slotres = (stacksz / heap->bsize) * heap->bsize < stacksz ? (stacksz / heap->bsize) + 1 : stacksz / heap->bsize;

	b->top = slotres;

	b->extended = 0;

	//assume aligned addr

	stack = (unsigned long*)&b[1]; //+ (heap->bsize - sizeof(KHEAPBLOCKSS));

	//stack = (unsigned long*)b + heap->bsize;


	for (x = slotres; x < b->max; ++x) {
		stack[x] = x * heap->bsize;
	}

	return 1;
}

void *k_heapSSAlloc(KHEAPSS *heap, unsigned long size) {
	KHEAPBLOCKSS		*b;
	unsigned long				ptr;
	unsigned long				*stack;

	/* too big */
	if (size > heap->bsize) {
		return 0;
	}


	for (b = heap->fblock; b; b = b->next) {
		if (b->top != b->max) {

			stack = (unsigned long*)&b[1];
		//	stack = (unsigned long*) (b + heap->bsize);
			ptr = stack[b->top++];

			ptr = (unsigned long)&b[0] +  heap->bsize + ptr;

			if((b->max - b->top) < 3 && b->extended == 0){
				dbgconout("EXTEND HEAP: ");
				dbgnumout_hex(size);
				b -> extended = 1;

				unsigned long addr = (unsigned long) page_find_and_alloc(16);

				page_flush();

				dbgconout("CALL ADDBLOCK: ");
				dbgnumout_hex(addr);

				k_heapSSAddBlock(heap, addr, 2097152*16);

				dbgconout("ADDBLOCK COMPLETE\r\n");

			}
			for(unsigned long i=0;i<size;++i){
				((unsigned char*)ptr)[i] = 0;
			}

			return (void*)ptr;
		}
	}

	/* no more space left */

	if(heap->fblock->size == 0){
			dbgconout("PANIK: SIZE IS ZERO FILLED ENTIRE RAM");
		dbgnumout_hex((unsigned long)heap->fblock->size);
		dbg_crash();
		while(1);
	}
	return (void*)694;
	//	return k_heapSSAlloc(heap, size);
}

void k_heapSSFree(KHEAPSS *heap, void *ptr) {
	KHEAPBLOCKSS		*b;
	unsigned long				_ptr;
	unsigned long				*stack;

	/* find block */
	_ptr = (unsigned long)ptr;
	for (b = heap->fblock; b; b = b->next) {
		if (_ptr > (unsigned long)b && _ptr < ((unsigned long)b + b->size))
			break;
	}

	/* might want to catch this somehow or somewhere.. kinda means corruption */
	if (!b)
		return;

	/* get stack */
	stack = (unsigned long*)&b[1];
	/* normalize offset to after block header */
	/* decrement top index */
	/* place entry back into stack */
	stack[--b->top] = _ptr - (unsigned long)&b[1];
	return;
}
