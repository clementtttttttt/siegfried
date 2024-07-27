#include "obj_heap.h"
#include "page.h"
#include "debug.h"
#include "draw.h"
#include "idt.h"

/**  Durand's Amazing Super Duper Memory functions.  */

#define VERSION 	"1.1"
#define ALIGNMENT	128ul//4ul				///< This is the byte alignment that memory must be allocated on. IMPORTANT for GTK and other stuff.

#define ALIGN_TYPE		unsigned char ///unsigned char[16] /// unsigned short
#define ALIGN_INFO		sizeof(ALIGN_TYPE)*16	///< Alignment information is stored right before the pointer. This is the number of bytes of information stored there.


#define USE_CASE1
#define USE_CASE2
#define USE_CASE3
#define USE_CASE4
#define USE_CASE5

//#define DEBUG

/** This macro will conveniently align our pointer upwards */
#define ALIGN( ptr )													\
		if ( ALIGNMENT > 1 )											\
		{																\
			unsigned long diff;												\
			ptr = (void*)((unsigned long)ptr + ALIGN_INFO);					\
			diff = (unsigned long)ptr & (ALIGNMENT-1);						\
			if ( diff != 0 )											\
			{															\
				diff = ALIGNMENT - diff;								\
				ptr = (void*)((unsigned long)ptr + diff);					\
			}															\
			*((ALIGN_TYPE*)((unsigned long)ptr - ALIGN_INFO)) = 			\
				diff + ALIGN_INFO;										\
		}


#define UNALIGN( ptr )													\
		if ( ALIGNMENT > 1 )											\
		{																\
			unsigned long diff = *((ALIGN_TYPE*)((unsigned long)ptr - ALIGN_INFO));	\
			if ( diff < (ALIGNMENT + ALIGN_INFO) )						\
			{															\
				ptr = (void*)((unsigned long)ptr - diff);					\
			}															\
		}



#define LIBALLOC_MAGIC	0xc001c0de
#define LIBALLOC_DEAD	0xdeaddead

#if defined DEBUG || defined INFO
#include <stdio.h>
#include <stdlib.h>

#define FLUSH()		fflush( stdout )

#endif

/** A structure found at the top of all system allocated
 * memory blocks. It details the usage of the memory block.
 */
struct liballoc_major
{
	struct liballoc_major *prev;		///< Linked list information.
	struct liballoc_major *next;		///< Linked list information.
	unsigned int pages;					///< The number of pages in the block.
	unsigned int size;					///< The number of pages in the block.
	unsigned int usage;					///< The number of bytes used in the block.
	struct liballoc_minor *first;		///< A pointer to the first allocated memory in the block.
}__attribute__((packed));


/** This is a structure found at the beginning of all
 * sections in a major block which were allocated by a
 * k_obj_alloc, k_obj_calloc, k_obj_realloc call.
 */
struct	liballoc_minor
{
	struct liballoc_minor *prev;		///< Linked list information.
	struct liballoc_minor *next;		///< Linked list information.
	struct liballoc_major *block;		///< The owning block. A pointer to the major structure.
	unsigned int magic;					///< A magic number to idenfity correctness.
	unsigned int size; 					///< The size of the memory allocated. Could be 1 byte or more.
	unsigned int req_size;				///< The size of memory requested.
}__attribute__((packed));


static struct liballoc_major *l_memRoot = 0;	///< The root memory block acquired from the system.
static struct liballoc_major *l_bestBet = 0; ///< The major with the most k_obj_free memory.

static unsigned int l_pageSize  = 2097152;			///< The size of an individual page. Set up in liballoc_init.
static unsigned int l_pageCount = 1;			///< The number of pages to request per chunk. Set up in liballoc_init.
static unsigned long long l_allocated = 0;		///< Running total of allocated memory.
static unsigned long long l_inuse	 = 0;		///< Running total of used memory.


static long long l_warningCount = 0;		///< Number of warnings encountered
static long long l_errorCount = 0;			///< Number of actual errors
static long long l_possibleOverruns = 0;	///< Number of possible overruns



int liballoc_unlock() {return 1;};
int liballoc_lock() {return 1;};

unsigned long blks;

unsigned long getblks(){return blks;}

void* liballoc_alloc(int sz){
	++blks;
	
	return page_find_and_alloc(sz);

}


int liballoc_free(void* in,int sz){
	--blks;
	page_free_found((unsigned long)in, sz);
	return 0;
}

// ***********   HELPER FUNCTIONS  *******************************

static void *liballoc_memset(void* s, int c, unsigned long n)
{
	unsigned int i;
	for ( i = 0; i < n ; i++)
		((char*)s)[i] = c;

	return s;
}
static void* liballoc_memcpy(void* s1, const void* s2, unsigned long n)
{
  char *cdest;
  char *csrc;
  unsigned int *ldest = (unsigned int*)s1;
  unsigned int *lsrc  = (unsigned int*)s2;

  while ( n >= sizeof(unsigned int) )
  {
      *ldest++ = *lsrc++;
	  n -= sizeof(unsigned int);
  }

  cdest = (char*)ldest;
  csrc  = (char*)lsrc;

  while ( n > 0 )
  {
      *cdest++ = *csrc++;
	  n -= 1;
  }

  return s1;
}


void liballoc_dump()
{

	struct liballoc_major *maj = l_memRoot;
	struct liballoc_minor *min = 0;


	draw_string( "liballoc: ------ Memory data ---------------\n");
	draw_string( "liballoc: System memory allocated in bytes: ");
	draw_hex(l_allocated);

	draw_string( "liballoc: Memory in used in BYTES(k_obj_alloc'ed): ");
	draw_hex(l_inuse);

	draw_string( "liballoc: Warning count: ");
	draw_hex(l_warningCount);
	draw_string( "liballoc: Error count: " );
	draw_hex(l_errorCount);

	draw_string( "liballoc: Possible overruns: ");
	draw_hex(l_possibleOverruns );


		while ( maj != 0 )
		{
			draw_string( "liballoc: ");
			draw_hex((unsigned long)maj);
			draw_string("size: ");
			draw_hex((unsigned long)maj->size);
			draw_string("usage: ");
			draw_hex((unsigned long)maj->usage );

			min = maj->first;
			while ( min != 0 )
			{
				draw_string( "liballoc min: ");
				draw_hex((unsigned long)min);
				draw_string("size: ");
				draw_hex((unsigned long)min->size );
				min = min->next;
			}

			maj = maj->next;
		}
}




// ***************************************************************

static struct liballoc_major *allocate_new_page( unsigned int size )
{
	unsigned int st;
	struct liballoc_major *maj;

		// This is how much space is required.
		st  = size + sizeof(struct liballoc_major);
		st += sizeof(struct liballoc_minor);

				// Perfect amount of space?
		if ( (st % l_pageSize) == 0 )
			st  = st / (l_pageSize);
		else
			st  = st / (l_pageSize) + 1;
							// No, add the buffer.


		// Make sure it's >= the minimum size.
		if ( st < l_pageCount ) st = l_pageCount;

		maj = (struct liballoc_major*)liballoc_alloc( st );

		if ( maj == 0 )
		{
			l_warningCount += 1;
			#if defined DEBUG || defined INFO
			printf( "liballoc: WARNING: liballoc_alloc( %i ) return 0\n", st );
			FLUSH();
			#endif
			return 0;	// uh oh, we ran out of memory.
		}

		maj->prev 	= 0;
		maj->next 	= 0;
		maj->pages 	= st;
		maj->size 	= st * l_pageSize;
		maj->usage 	= sizeof(struct liballoc_major);
		maj->first 	= 0;

		l_allocated += maj->size;

		#ifdef DEBUG
		printf( "liballoc: Resource allocated %x of %i pages (%i bytes) for %i size.\n", maj, st, maj->size, size );

		printf( "liballoc: Total memory usage = %i KB\n",  (int)((l_allocated / (1024))) );
		FLUSH();
		#endif



      return maj;
}



void *PREFIX(k_obj_alloc_2)(unsigned long req_size)
{
	
	pml4e *old = page_get_curr_tab();
	page_switch_krnl_tab();

	//dbgnumout_hex((unsigned long) __builtin_return_address(0));
	//while(1){}
	int startedBet = 0;
	unsigned long long bestSize = 0;
	void *p = 0;
	unsigned long diff;
	struct liballoc_major *maj;
	struct liballoc_minor *min;
	struct liballoc_minor *new_min;
	unsigned long size = req_size;

	// For alignment, we adjust size so there's enough space to align.
	if ( ALIGNMENT > 1 )
	{
		size += ALIGNMENT + ALIGN_INFO;
	}
				// So, ideally, we really want an alignment of 0 or 1 in order
				// to save space.

	liballoc_lock();

	if ( size == 0 )
	{
		l_warningCount += 1;
		#if defined DEBUG || defined INFO
		printf( "liballoc: WARNING: alloc( 0 ) called from %x\n",
							__builtin_return_address(0) );
		FLUSH();
		#endif
		liballoc_unlock();
			page_switch_tab(old);

		return PREFIX(k_obj_alloc)(1);
	}


	if ( l_memRoot == 0 )
	{
		#if defined DEBUG || defined INFO
		#ifdef DEBUG
		printf( "liballoc: initialization of liballoc " VERSION "\n" );
		#endif
		atexit( liballoc_dump );
		FLUSH();
		#endif

		// This is the first time we are being used.
		l_memRoot = allocate_new_page( size );
		if ( l_memRoot == 0 )
		{
		  liballoc_unlock();
		  #ifdef DEBUG
		  printf( "liballoc: initial l_memRoot initialization failed\n", p);
		  FLUSH();
		  #endif
		  return 0;
		}

		#ifdef DEBUG
		printf( "liballoc: set up first memory major %x\n", l_memRoot );
		FLUSH();
		#endif

	}


	#ifdef DEBUG
	printf( "liballoc: %x PREFIX(k_obj_alloc)( %i ): ",
					__builtin_return_address(0),
					size );
	FLUSH();
	#endif

	// Now we need to bounce through every major and find enough space....

	maj = l_memRoot;
	startedBet = 0;


	// test oom.

	struct liballoc_minor *oldtest = maj->first;
	maj->first = (struct liballoc_minor*)0xdeadbeef;

	if(maj->first == oldtest){
		draw_string("OOM DETECTED");
		while(1);
	}

	maj->first = oldtest;

	// Start at the best bet....
	if ( l_bestBet != 0 )
	{
		bestSize = l_bestBet->size - l_bestBet->usage;

		if ( bestSize > (size + sizeof(struct liballoc_minor)))
		{
			maj = l_bestBet;
			startedBet = 1;
		}
	}

	while ( maj != 0 )
	{
		diff  = maj->size - maj->usage;
										// k_obj_free memory in the block

		if ( bestSize < diff )
		{
			// Hmm.. this one has more memory then our bestBet. Remember!
			l_bestBet = maj;
			bestSize = diff;
		}


#ifdef USE_CASE1

		// CASE 1:  There is not enough space in this major block.
		if ( diff < (size + sizeof( struct liballoc_minor )) )
		{
			#ifdef DEBUG
			printf( "CASE 1: Insufficient space in block %x\n", maj);
			FLUSH();
			#endif

				// Another major block next to this one?
			if ( maj->next != 0 )
			{
				maj = maj->next;		// Hop to that one.
				continue;
			}

			if ( startedBet == 1 )		// If we started at the best bet,
			{							// let's start all over again.
				maj = l_memRoot;
				startedBet = 0;
				continue;
			}

			// Create a new major block next to this one and...
			maj->next = allocate_new_page( size );	// next one will be okay.

			if ( maj->next == 0 ) break;			// no more memory.
			maj->next->prev = maj;
			maj = maj->next;

			// .. fall through to CASE 2 ..
		}

#endif

#ifdef USE_CASE2


		// CASE 2: It's a brand new block.
		if ( maj->first == 0 )
		{
			maj->first = (struct liballoc_minor*)((unsigned long)maj + sizeof(struct liballoc_major) );


			maj->first->magic 		= LIBALLOC_MAGIC;
			maj->first->prev 		= 0;
			maj->first->next 		= 0;
			maj->first->block 		= maj;
			maj->first->size 		= size;
			maj->first->req_size 	= req_size;
			maj->usage 	+= size + sizeof( struct liballoc_minor );


			l_inuse += size;


			p = (void*)((unsigned long)(maj->first) + sizeof( struct liballoc_minor ));

			ALIGN( p );

			#ifdef DEBUG
			printf( "CASE 2: returning %x\n", p);
			FLUSH();
			#endif
			liballoc_unlock();		// release the lock
				page_switch_tab(old);

						return p;
		}

#endif

#ifdef USE_CASE3

		// CASE 3: Block in use and enough space at the start of the block.
		diff =  (unsigned long)(maj->first);
		diff -= (unsigned long)maj;
		diff -= sizeof(struct liballoc_major);

		if ( diff >= (size + sizeof(struct liballoc_minor)) )
		{
			// Yes, space in front. Squeeze in.
			maj->first->prev = (struct liballoc_minor*)((unsigned long)maj + sizeof(struct liballoc_major) );
			maj->first->prev->next = maj->first;
			maj->first = maj->first->prev;

			maj->first->magic 	= LIBALLOC_MAGIC;
			maj->first->prev 	= 0;
			maj->first->block 	= maj;
			maj->first->size 	= size;
			maj->first->req_size 	= req_size;
			maj->usage 			+= size + sizeof( struct liballoc_minor );

			l_inuse += size;

			p = (void*)((unsigned long)(maj->first) + sizeof( struct liballoc_minor ));
			ALIGN( p );

			#ifdef DEBUG
			printf( "CASE 3: returning %x\n", p);
			FLUSH();
			#endif
			liballoc_unlock();		// release the lock
				page_switch_tab(old);

			return p;
		}

#endif


#ifdef USE_CASE4

		// CASE 4: There is enough space in this block. But is it contiguous?
		min = maj->first;

			// Looping within the block now...
		while ( min != 0 )
		{
				// CASE 4.1: End of minors in a block. Space from last and end?
				if ( min->next == 0 )
				{
					// the rest of this block is k_obj_free...  is it big enough?
					diff = (unsigned long)(maj) + maj->size;
					diff -= (unsigned long)min;
					diff -= sizeof( struct liballoc_minor );
					diff -= min->size;
						// minus already existing usage..

					if ( diff >= (size + sizeof( struct liballoc_minor )) )
					{
						// yay....
						min->next = (struct liballoc_minor*)((unsigned long)min + sizeof( struct liballoc_minor ) + min->size);
						min->next->prev = min;
						min = min->next;
						min->next = 0;
						min->magic = LIBALLOC_MAGIC;
						min->block = maj;
						min->size = size;
						min->req_size = req_size;
						maj->usage += size + sizeof( struct liballoc_minor );

						l_inuse += size;

						p = (void*)((unsigned long)min + sizeof( struct liballoc_minor ));
						ALIGN( p );

						#ifdef DEBUG
						printf( "CASE 4.1: returning %x\n", p);
						FLUSH();
						#endif
						
						liballoc_unlock();		// release the lock
						
	page_switch_tab(old);

						return p;
					}
				}



				// CASE 4.2: Is there space between two minors?
				if ( min->next != 0 )
				{
					// is the difference between here and next big enough?
					diff  = (unsigned long)(min->next);
					diff -= (unsigned long)min;
					diff -= sizeof( struct liballoc_minor );
					diff -= min->size;
										// minus our existing usage.

					if ( diff >= (size + sizeof( struct liballoc_minor )) )
					{
						// yay......
						new_min = (struct liballoc_minor*)((unsigned long)min + sizeof( struct liballoc_minor ) + min->size);

						new_min->magic = LIBALLOC_MAGIC;
						new_min->next = min->next;
						new_min->prev = min;
						new_min->size = size;
						new_min->req_size = req_size;
						new_min->block = maj;
						min->next->prev = new_min;
						min->next = new_min;
						maj->usage += size + sizeof( struct liballoc_minor );

						l_inuse += size;

						p = (void*)((unsigned long)new_min + sizeof( struct liballoc_minor ));
						ALIGN( p );


						#ifdef DEBUG
						printf( "CASE 4.2: returning %x\n", p);
						FLUSH();
						#endif

						liballoc_unlock();		// release the lock
							page_switch_tab(old);

						return p;
					}
				}	// min->next != 0

				min = min->next;
		} // while min != 0 ...


#endif

#ifdef USE_CASE5

		// CASE 5: Block full! Ensure next block and loop.
		if ( maj->next == 0 )
		{
			#ifdef DEBUG
			printf( "CASE 5: block full\n");
			FLUSH();
			#endif

			if ( startedBet == 1 )
			{
				maj = l_memRoot;
				startedBet = 0;
				continue;
			}

			// we've run out. we need more...
			maj->next = allocate_new_page( size );		// next one guaranteed to be okay
			if ( maj->next == 0 ) break;			//  uh oh,  no more memory.....
			maj->next->prev = maj;

		}

#endif

		maj = maj->next;
	} // while (maj != 0)



	liballoc_unlock();		// release the lock

	draw_string("WARN: RETURNING 0 LIBALLOC");
	while(1){}

	#ifdef DEBUG
	printf( "All cases exhausted. No memory available.\n");
	FLUSH();
	#endif
	#if defined DEBUG || defined INFO
	printf( "liballoc: WARNING: PREFIX(k_obj_alloc)( %i ) returning 0.\n", size);
	liballoc_dump();
	FLUSH();
	#endif
	
	page_switch_tab(old);
	return 0;
}





void *PREFIX(k_obj_alloc)(unsigned long req_size){
        void *tmp = k_obj_alloc_2(req_size);
        
        return tmp;
}




void PREFIX(k_obj_free)(void *ptr)
{
	void *old_tab = page_get_curr_tab();
	
	page_switch_krnl_tab();
	struct liballoc_minor *min;
	struct liballoc_major *maj;

	if ( ptr == 0 )
	{
		l_warningCount += 1;
		#if defined DEBUG || defined INFO
		printf( "liballoc: WARNING: PREFIX(k_obj_free)( 0 ) called from %x\n",
							__builtin_return_address(0) );
		FLUSH();
		#endif
		return;
	}
	

	if(page_get_curr_tab() != page_get_krnl_tab()){
			draw_string("???");
			while(1){}
	}
	UNALIGN( ptr );

	liballoc_lock();		// lockit


	min = (struct liballoc_minor*)((unsigned long)ptr - sizeof( struct liballoc_minor ));


	if ( min->magic != LIBALLOC_MAGIC )
	{
		l_errorCount += 1;

		// Check for overrun errors. For all bytes of LIBALLOC_MAGIC
		if (
			((min->magic & 0xFFFFFF) == (LIBALLOC_MAGIC & 0xFFFFFF)) ||
			((min->magic & 0xFFFF) == (LIBALLOC_MAGIC & 0xFFFF)) ||
			((min->magic & 0xFF) == (LIBALLOC_MAGIC & 0xFF))
		   )
		{
			l_possibleOverruns += 1;
			draw_string( "liballoc: ERROR: Possible 1-3 byte overrun for magic %x != %x\n");
			draw_hex(min->magic);
			draw_hex(LIBALLOC_MAGIC);
		//	FLUSH();


		}


		if ( min->magic == LIBALLOC_DEAD )
		{
			draw_string( "liballoc: ERROR: multiple PREFIX(k_obj_free)() attempt on %x from (first address in stack trace).\n");
			draw_hex((unsigned long )ptr);
			//FLUSH();
		}
		else
		{
			draw_string( "liballoc: ERROR: Bad PREFIX(k_obj_free)( %x ) called from (first address in stack trace)\n");
								draw_hex((unsigned long)ptr);
	//		FLUSH();
		}

		draw_string("min->magic =");
		draw_hex(min->magic);
		draw_string("&min->magic = ");
		draw_hex((unsigned long) &min->magic);
		//liballoc_dump();
		idt_print_stacktrace(__builtin_frame_address(0));
		while(1){}
		// being lied to...
		liballoc_unlock();		// release the lock
		return;
	}

	#ifdef DEBUG
	printf( "liballoc: %x PREFIX(k_obj_free)( %x ): ",
				__builtin_return_address( 0 ),
				ptr );
	FLUSH();
	#endif


		maj = min->block;

		l_inuse -= min->size;

		maj->usage -= (min->size + sizeof( struct liballoc_minor ));
		min->magic  = LIBALLOC_DEAD;		// No mojo.

		if ( min->next != 0 ) min->next->prev = min->prev;
		if ( min->prev != 0 ) min->prev->next = min->next;

		if ( min->prev == 0 ) maj->first = min->next;
							// Might empty the block. This was the first
							// minor.


	// We need to clean up after the majors now....

	if ( maj->first == 0 )	// Block completely unused.
	{
		if ( l_memRoot == maj ) l_memRoot = maj->next;
		if ( l_bestBet == maj ) l_bestBet = 0;
		if ( maj->prev != 0 ) maj->prev->next = maj->next;
		if ( maj->next != 0 ) maj->next->prev = maj->prev;
		l_allocated -= maj->size;

		liballoc_free( maj, maj->pages );
	}
	else
	{
		if ( l_bestBet != 0 )
		{
			int bestSize = l_bestBet->size  - l_bestBet->usage;
			int majSize = maj->size - maj->usage;

			if ( majSize > bestSize ) l_bestBet = maj;
		}

	}


	#ifdef DEBUG
	printf( "OK\n");
	FLUSH();
	#endif

	liballoc_unlock();		// release the lock
	
	page_switch_tab(old_tab);
}





void* PREFIX(k_obj_calloc)(unsigned long nobj, unsigned long size)
{
       int real_size;
       void *p;

       real_size = nobj * size;

       p = PREFIX(k_obj_alloc)( real_size );

       liballoc_memset( p, 0, real_size );

       return p;
}




void*   PREFIX(k_obj_realloc)(void *p, unsigned long size)
{
	void *ptr;
	struct liballoc_minor *min;
	unsigned int real_size;

	// Honour the case of size == 0 => k_obj_free old and return 0
	if ( size == 0 )
	{
		PREFIX(k_obj_free)( p );
		return 0;
	}

	// In the case of a 0 pointer, return a simple k_obj_alloc.
	if ( p == 0 ) return PREFIX(k_obj_alloc)( size );

	// Unalign the pointer if required.
	ptr = p;
	UNALIGN(ptr);

	liballoc_lock();		// lockit

		min = (struct liballoc_minor*)((unsigned long)ptr - sizeof( struct liballoc_minor ));

		// Ensure it is a valid structure.
		if ( min->magic != LIBALLOC_MAGIC )
		{
			l_errorCount += 1;

			// Check for overrun errors. For all bytes of LIBALLOC_MAGIC
			if (
				((min->magic & 0xFFFFFF) == (LIBALLOC_MAGIC & 0xFFFFFF)) ||
				((min->magic & 0xFFFF) == (LIBALLOC_MAGIC & 0xFFFF)) ||
				((min->magic & 0xFF) == (LIBALLOC_MAGIC & 0xFF))
			   )
			{
				
				l_possibleOverruns += 1;
				#if defined DEBUG || defined INFO
				printf( "liballoc: ERROR: Possible 1-3 byte overrun for magic %x != %x\n",
									min->magic,
									LIBALLOC_MAGIC );
				FLUSH();
				#endif
			}


			if ( min->magic == LIBALLOC_DEAD )
			{
				#if defined DEBUG || defined INFO
				printf( "liballoc: ERROR: multiple PREFIX(k_obj_free)() attempt on %x from %x.\n",
										ptr,
										__builtin_return_address(0) );
				FLUSH();
				#endif
			}
			else
			{
				#if defined DEBUG || defined INFO
				printf( "liballoc: ERROR: Bad PREFIX(k_obj_free)( %x ) called from %x\n",
									ptr,
									__builtin_return_address(0) );
				FLUSH();
				#endif
			}

			// being lied to...
			liballoc_unlock();		// release the lock
			return 0;
		}

		// Definitely a memory block.

		real_size = min->req_size;

		if ( real_size >= size )
		{
			min->req_size = size;
			liballoc_unlock();
			return p;
		}

	liballoc_unlock();

	// If we got here then we're reallocating to a block bigger than us.
	ptr = PREFIX(k_obj_alloc)( size );					// We need to allocate new memory
	liballoc_memcpy( ptr, p, real_size );
	draw_string("RELA_SIZE:");
	draw_hex(real_size);
	PREFIX(k_obj_free)( p );

	return ptr;
}

