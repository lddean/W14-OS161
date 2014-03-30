#if OPT_A2
#ifndef _COREMAP_H
#define _COREMAP_H

#include "opt-A2.h"
#include <types.h>
#include <lib.h>
#include <array.h>
#include <vnode.h>
#include <vfs.h>
#include <limits.h>
#include <kern/fcntl.h>
#include <kern/limits.h>
#include <file_descriptor.h>
#include <synch.h>
#include <addrspace.h>

struct page{
        vaddr_t va; // virtual address
        vaddr_t pa; // physical address

        int state; // 0->fixed; 1->free; 2->dirty(occupied)
	
	int length; // length of pages(npages in alloc)
};

// this is coremap, record pages information
struct page* pages;
int core_size, page_size;
paddr_t firstPaddr, lastPaddr, endCore;

struct lock *corelock; // lock for free/alloc

/* 
 *	coremap_init - initilize all members
 *	coremap_check_pages - check whether it can contain npages starting index 
 *	coremap_occupy_pages - occupy npages starting index
 *	coremap_alloc - alloc n pages into coremap
 *	coremap_free - free addr and related addresses(from length)
*/

void coremap_init(void);
bool coremap_check_pages(int index, int pages);
vaddr_t coremap_occupy_pages(int index, int pages);
vaddr_t coremap_alloc(int n);
void coremap_free(vaddr_t addr);

#endif
#endif