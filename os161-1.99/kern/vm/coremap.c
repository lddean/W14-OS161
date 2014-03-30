#include <types.h>
#include <kern/errno.h>
#include <lib.h>
#include <spl.h>
#include <spinlock.h>
#include <proc.h>
#include <current.h>
#include <mips/tlb.h>
#include <addrspace.h>
#include <coremap.h>

// set up coremap
void coremap_init(void){
	corelock = lock_create("coremap");

   	ram_getsize(&firstPaddr, &lastPaddr);

        // get the corresponding virtual pages
        pages = (struct page*)PADDR_TO_KVADDR(firstPaddr);

        // get page size(get multiple of PAGE_SIZE)
        page_size = (lastPaddr - firstPaddr)/PAGE_SIZE;

        // the paddr & offset for end of coremap
        endCore = firstPaddr + page_size*(sizeof(struct page));
        // this is formula, but need round up  // core_size = (endCore - firstPaddr)/PAGE_SIZE;
        core_size = 1 + ((endCore - firstPaddr) - 1) / PAGE_SIZE;

//kprintf("first %d endcore %d and last %d\n page_sizes %d  core_size %d PAGE_SIZE %d\n", firstPaddr, endCore, lastPaddr, page_size, core_size, PAGE_SIZE);

//kprintf("pages %p endCore %d long %u\n", pages, endCore, PADDR_TO_KVADDR(firstPaddr));

	// this will never be touched(set it to fixed), but initialize anyway
        for(int i=0; i<core_size; i++){
                struct page* current = pages+i;

                paddr_t current_paddr = firstPaddr + i*PAGE_SIZE; // get current physical address
                current->pa = current_paddr;
                current->va = PADDR_TO_KVADDR(current_paddr); // get current virtual address

                current->state = 0; // 0 is fixed
                current->length = 0; // 0 is fixed
        }

        for(int i=core_size; i<page_size; i++){
                struct page* current = pages+i;

                paddr_t current_paddr = firstPaddr + i*PAGE_SIZE; // get current physical address
                current->pa = current_paddr;
                current->va = PADDR_TO_KVADDR(current_paddr); // get current virtual address

                current->state = 1; // 1 is free
                current->length = 0; // 1 is free
        }
}

// check whether it has n consecutive pages starting at index i
bool coremap_check_pages(int index, int npages){
	for(int i=index; i<index+npages; i++){
		struct page* current = pages+i;
		if(current->state != 1){ // it is not free, return false	
			return false;
		}
	}
	return true;
}
	
// mark n consecutive pages as occupied, zero out physical/virtual addresses & return the first virtual address
vaddr_t coremap_occupy_pages(int index, int npages){
	for(int i=index; i<index+npages; i++){
		struct page* current = pages+i;
		current->state = 2; // mark it dirty(occupied)
	}
	struct page* valid = pages+index;

	// zero out physical/virtual address when we allocate
        bzero((void *)valid->pa, npages * PAGE_SIZE);
        bzero((void *)PADDR_TO_KVADDR(valid->pa), npages * PAGE_SIZE);

	return valid->va;
}

// allocate n pages in physical memory
vaddr_t coremap_alloc(int n){
	lock_acquire(corelock);
	int i=0;
	while(i < page_size){
		if(!coremap_check_pages(i, n)){
			i = i+n;
		}else{
			return coremap_occupy_pages(i, n);
		}
	}
	// if it reaches here, should be a problem
	lock_release(corelock);
	return -1;
}

// free coremap 
void coremap_free(vaddr_t addr){
	lock_acquire(corelock);
	int start=0; // start point to free

	struct page* current = pages+start;
	// loop to find starting point
	while(current->va != addr){
		start++;
                current = pages + start;
		if(start == page_size){
			kprintf("cannot find freed page\n");
		}
	}

 	//use START point to free LENGTH memories	
	int length = current->length;
	for(int i=start; i<length; i++){
		current = pages+i;
		current->state = 1; // flag it to free
	}

	lock_release(corelock);
	// this is for shoot-down?
	as_activate();
}

                                                   