/*
 * Helper for use with the PSP Software Development Kit - http://www.pspdev.org
 * -----------------------------------------------------------------------
 * Licensed as 'free to use and modify as long as credited appropriately'
 *
 * valloc.c - Standard C VRAM allocation routines.
 *
 * Copyright (c) 2006 Alexander Berl 'Raphael' <raphael@fx-world.org>
 * http://wordpress.fx-world.org
 *
 */
//#include <psptypes.h>
//#include <pspkernel.h>
#ifdef DEBUG
#include <pspdebug.h>
#endif
#include <pspgu.h>
#include <malloc.h>
#include "valloc.h"

#ifdef DEBUG
#define printf  pspDebugScreenPrintf
#endif

/* Use this to set the default valloc() alignment. */
#define DEFAULT_VALIGNMENT	16

#ifndef ALIGN
#define ALIGN(x, align) (((x)+((align)-1))&~((align)-1))
#endif


#define VRAM_SIZE 0x00200000
#define VRAM_BASE 0x04000000
#define UNCACHED_POINTER 0×40000000

static size_t __vram_free = VRAM_SIZE;
static size_t __vram_largest_block = VRAM_SIZE;


static unsigned int __attribute__((aligned(16))) list[16];


/* _vram_mem_block_header structure. */
typedef struct _vram_mem_header {
	void *	ptr;
	size_t	size;
	struct _vram_mem_header * prev;
	struct _vram_mem_header * next;
} vram_mem_header_t;



void * __valloc_vram_base = (void*)0;
vram_mem_header_t *__valloc_vram_head = NULL;
vram_mem_header_t *__valloc_vram_tail = NULL;



size_t vgetMemorySize(unsigned int width, unsigned int height, unsigned int psm)
{
	switch (psm)
	{
		case GU_PSM_T4:
			return (width * height) >> 1;

		case GU_PSM_T8:
			return width * height;

		case GU_PSM_5650:
		case GU_PSM_5551:
		case GU_PSM_4444:
		case GU_PSM_T16:
			return 2 * width * height;

		case GU_PSM_8888:
		case GU_PSM_T32:
			return 4 * width * height;

		default:
			return 0;
	}
}

inline void* vGuPointer( void *ptr )
{
	return (void*)((u32)ptr & ~VRAM_BASE);
}

inline void* vCPUPointer( void *ptr )
{
	return (void*)((u32)ptr | VRAM_BASE);
}


/* Find the best size for a texture to fit into size bytes at 2bytes per pixel */
void _vram_fit_size( size_t size, int* width, int* height, int* stride )
{
	if (size<1024) {
		*width = size/2;
		*height = 1;
		*stride = 512;
	} else {
		*width = 512;
		*height = size/1024;
		*stride = 512;
	}
}



/* Find the smallest block that we can allocate AFTER, returning NULL if there
   are none.  */
vram_mem_header_t * _vram_mem_fit(vram_mem_header_t *head, size_t size)
{
	vram_mem_header_t *prev_mem = head, *best_fit = NULL;
	u32 prev_top, next_bot;
	size_t best_size = 0;

	if (((u32)head->ptr+head->size+size)<=VRAM_SIZE) {
		best_fit = head;
		best_size = VRAM_SIZE - ((u32)head->ptr+head->size);
	}

	while (prev_mem != NULL) {
		if (prev_mem->next != NULL) {
			prev_top = (u32)prev_mem->ptr;
			next_bot = prev_top - ((u32)prev_mem->next->ptr + prev_mem->next->size);
			if (next_bot >= size) {
				if (best_fit==NULL || next_bot<best_size) {
					best_fit = prev_mem->next;
					best_size = next_bot;
				}
			}
		}

		prev_mem = prev_mem->next;
	}

	return best_fit;
}


void _vram_find_largest_block()
{
	vram_mem_header_t *cur;
	__vram_largest_block = VRAM_SIZE - ((u32)__valloc_vram_head->ptr + __valloc_vram_head->size);
	size_t new_size;

	cur = __valloc_vram_head;
	while (cur->next!=NULL)  {
		new_size = (u32)cur->ptr - ((u32)cur->next->ptr + cur->next->size);
		if (new_size>__vram_largest_block) __vram_largest_block = new_size;
		cur = cur->next;
	}
}


void * valloc(size_t size)
{
	void *ptr = NULL;
	vram_mem_header_t *new_mem, *prev_mem;
	size_t mem_sz;

	mem_sz = size;

	if ((mem_sz & (DEFAULT_VALIGNMENT - 1)) != 0)
		mem_sz = ALIGN(mem_sz, DEFAULT_VALIGNMENT);


	if (mem_sz>__vram_largest_block)
		return ptr;

	/* If we don't have any allocated blocks, reserve the first block
	   and initialize __valloc_vram_tail.  */
	if (__valloc_vram_head == NULL) {
		if (size>VRAM_SIZE)
			return ptr;

		__valloc_vram_head = (vram_mem_header_t *)malloc( sizeof(vram_mem_header_t) );
		if (__valloc_vram_head == NULL)
			return ptr;

		ptr = (void *)__valloc_vram_base;


		__valloc_vram_head->ptr  = ptr;
		__valloc_vram_head->size = mem_sz;
		__valloc_vram_head->prev = NULL;
		__valloc_vram_head->next = NULL;

		__valloc_vram_tail = __valloc_vram_head;
		
		__vram_free -= mem_sz;
		__vram_largest_block -= mem_sz;
		
		return vabsptr(ptr);
	}

	/* Check to see if there's free space at the bottom of the heap.
	   NOTE: This case is now already handled in _vram_mem_fit */
	/*if (((u32)__valloc_vram_head->ptr + __valloc_vram_head->size + mem_sz) < VRAM_SIZE) {
		new_mem = (vram_mem_header_t *)malloc( sizeof(vram_mem_header_t) );
		if (new_mem == NULL)
			return ptr;
		ptr     = (void *)((u32)__valloc_vram_head->ptr + __valloc_vram_head->size);

		new_mem->ptr  = ptr;
		new_mem->size = mem_sz;
		new_mem->prev = NULL;
		new_mem->next = __valloc_vram_head;
		new_mem->next->prev = new_mem;
		__valloc_vram_head = new_mem;
		
		return ptr;
	}*/

	/* See if we can allocate the block anywhere. */
	prev_mem = _vram_mem_fit(__valloc_vram_head, mem_sz);
	if (prev_mem != NULL) {
		new_mem = (vram_mem_header_t *)malloc( sizeof(vram_mem_header_t) );
		if (new_mem == NULL)
			return ptr;
		ptr     = (void *)((u32)prev_mem->ptr + prev_mem->size);

		new_mem->ptr  = ptr;
		new_mem->size = mem_sz;
		new_mem->prev = prev_mem->prev;
		if (new_mem->prev!=NULL)
		  new_mem->prev->next = new_mem;
		new_mem->next = prev_mem;
		prev_mem->prev = new_mem;
		if (prev_mem == __valloc_vram_head)
		  __valloc_vram_head = new_mem;
		__vram_free -= mem_sz;
		
		_vram_find_largest_block();

		return vabsptr(ptr);
	}

	/* Now we have a problem: There's no room at the bottom and also no room in between.
	   So either we do compact the memory (not easily possible because allocated pointers get invalidated),
	   or we just return NULL so the application has to handle this case itself.
	   For now we'll just return NULL
	*/

	return ptr;
}


void * vrealloc(void *ptr, size_t size)
{
	vram_mem_header_t *prev_mem;
	void *new_ptr = NULL;
	
	if (!size && ptr != NULL) {
		vfree(ptr);
		return new_ptr;
	}

	if (ptr == NULL)
		return valloc(size);
	
	if ((size & (DEFAULT_VALIGNMENT - 1)) != 0)
		size = ALIGN(size, DEFAULT_VALIGNMENT);

	if (!__valloc_vram_head)
		return new_ptr;
		
	prev_mem = __valloc_vram_head;
	while (vrelptr(ptr) != prev_mem->ptr)  {
		/* ptr isn't in our list */
		if (prev_mem->next == NULL) {
			return ptr;
		}
		prev_mem = prev_mem->next;
	}
	
	
	/* Don't do anything if asked for same sized block. */
	/* If the new size is shorter, let's just shorten the block. */
	if (prev_mem->size >= size) {
		__vram_free += (prev_mem->size-size);
		prev_mem->size = size;
		_vram_find_largest_block();
		return ptr;
	}

	/* We are asked for a larger block of memory. */
	/* Are we the last memory block ? */	
	size_t next_offs = VRAM_SIZE;
	if (prev_mem->prev)
		/* Is the next block far enough so we can extend the current block ? */
		next_offs = (u32)prev_mem->prev->ptr;

	if ((u32)prev_mem->ptr+size<=next_offs) {
		__vram_free -= (size-prev_mem->size);
		prev_mem->size = size;
		_vram_find_largest_block();
		return ptr;
	}
	
	/* Not enough room at the head, so check for more space before the block */
	if (prev_mem->next) {
		if (((u32)prev_mem->next->ptr+prev_mem->next->size+size)<=next_offs) {
			/* block fits directly after the previous block, so move the data */
			int blockwidth, blockheight, blockstride;
			_vram_fit_size( prev_mem->size, &blockwidth, &blockheight, &blockstride );
			//printf("sceGuCopyImage: width=%i height=%i stride=%i\n", blockwidth, blockheight, blockstride );
			sceGuStart(GU_DIRECT,list);
			sceGuCopyImage(GU_PSM_5650,0,0,blockwidth,blockheight,blockstride,vabsptr(prev_mem->ptr),0,0,blockstride,vabsptr((void*)((u32)prev_mem->next->ptr + prev_mem->next->size)));
			sceGuFinish();
			prev_mem->ptr = (void*)((u32)prev_mem->next->ptr + prev_mem->next->size);
			__vram_free -= (size-prev_mem->size);
			prev_mem->size = size;
			_vram_find_largest_block();
			return vabsptr(prev_mem->ptr);
		}
	}
	
	/* We got out of luck, let's allocate a new block of memory. */
	if ((new_ptr = valloc(size)) == NULL)
		return new_ptr;

    /* New block is larger, we only copy the old data. */
	int blockwidth, blockheight, blockstride;
	_vram_fit_size( prev_mem->size, &blockwidth, &blockheight, &blockstride );
	sceGuStart(GU_DIRECT,list);
	sceGuCopyImage(GU_PSM_5650,0,0,blockwidth,blockheight,blockstride,vabsptr(prev_mem->ptr),0,0,blockstride,new_ptr);
	sceGuFinish();
	free(ptr);

	return new_ptr;
}


void vfree(void *ptr)
{
	vram_mem_header_t *cur;

	if (!ptr)
		return;
	
	if (!__valloc_vram_head)
		return;

	ptr = vrelptr(ptr);

	/* Freeing the head pointer is a special case.  */
	if (ptr == __valloc_vram_head->ptr) {
		__vram_free += __valloc_vram_head->size;
		
		cur = __valloc_vram_head->next;
		free(__valloc_vram_head);

		__valloc_vram_head = cur;

		if (__valloc_vram_head != NULL) {
			__valloc_vram_head->prev = NULL;
			__vram_largest_block = VRAM_SIZE - __valloc_vram_head->size;
		} else {
			__valloc_vram_tail = NULL;
			__vram_largest_block = VRAM_SIZE;
		}

		return;
	}

	cur = __valloc_vram_head;
	while (ptr != cur->ptr)  {
		/* ptr isn't in our list */
		if (cur->next == NULL) {
			return;
		}
		cur = cur->next;
	}

	/* Deallocate the block.  */
	if (cur->next != NULL) {
		cur->next->prev = cur->prev;
		size_t new_size = (u32)cur->prev->ptr - ((u32)cur->next->ptr + cur->next->size);
		if (new_size>__vram_largest_block) __vram_largest_block = new_size;
	} else {
		__valloc_vram_tail = cur->prev;
		size_t new_size = VRAM_SIZE - ((u32)cur->prev->ptr + cur->prev->size);
		if (new_size>__vram_largest_block) __vram_largest_block = new_size;
	}

	cur->prev->next = cur->next;
	
	__vram_free += cur->size;
	free( cur );
}


size_t vmemavail()
{
/*	if (__valloc_vram_head==NULL)
		return VRAM_SIZE;
	
	vram_mem_header_t *cur;
	size_t size = VRAM_SIZE - ((u32)__valloc_vram_head->ptr + __valloc_vram_head->size);

	cur = __valloc_vram_head;
	while (cur->next!=NULL)  {
		size += (u32)cur->ptr - ((u32)cur->next->ptr + cur->next->size);
		cur = cur->next;
	}

	return size;
*/
	return __vram_free;		// Quick solution. Idea by Scienthsine
}


size_t vlargestblock()
{
	/*if (__valloc_vram_head==NULL)
		return VRAM_SIZE;
	
	vram_mem_header_t *cur;
	size_t size = VRAM_SIZE - ((u32)__valloc_vram_head->ptr + __valloc_vram_head->size);
	size_t new_size;

	cur = __valloc_vram_head;
	while (cur->next!=NULL)  {
		new_size = (u32)cur->ptr - ((u32)cur->next->ptr + cur->next->size);
		if (new_size>size) size = new_size;
		cur = cur->next;
	}

	return size;*/
	return __vram_largest_block;
}

