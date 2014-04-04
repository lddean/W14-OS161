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

// get the victim to swap for coremap
// we use FIFO to make it simple && notice our coremap occupies 2 pages, so always start with 2
int coremap_get_victim(void){
	int victim;
	// 12 for stack space????
	static unsigned int next_victim = 15;
	victim = next_victim;
	next_victim = (next_victim + 1) % page_size;
	if(next_victim < 15){
		next_victim = 15;
	}
	return victim;
}

// set up coremap
void coremap_init(void){
	corelock = lock_create("coremap");

   	ram_getsize(&firstPaddr, &lastPaddr);

        // get the corresponding virtual pages
        pages = (struct core_page*)PADDR_TO_KVADDR(firstPaddr);

        // get page size(get multiple of PAGE_SIZE)
        page_size = (lastPaddr - firstPaddr)/PAGE_SIZE;

        // the paddr & offset for end of coremap
        endCore = firstPaddr + page_size*(sizeof(struct core_page));
        // this is formula, but need round up  // core_size = (endCore - firstPaddr)/PAGE_SIZE;
        core_size = 1 + ((endCore - firstPaddr) - 1) / PAGE_SIZE;

//kprintf("first %d endcore %d and last %d\n page_sizes %d  core_size %d PAGE_SIZE %d\n", firstPaddr, endCore, lastPaddr, page_size, core_size, PAGE_SIZE);

//kprintf("pages %p endCore %d long %u\n", pages, endCore, PADDR_TO_KVADDR(firstPaddr));

	// this will never be touched(set it to fixed), but initialize anyway
        for(int i=0; i<core_size; i++){
                struct core_page* current = pages+i;

                paddr_t current_paddr = firstPaddr + i*PAGE_SIZE; // get current physical address
                current->pa = current_paddr;
                current->as = curproc_getas();
                //current->va = PADDR_TO_KVADDR(current_paddr); // get current virtual address

		current->order = 0; // 0 refer
                current->state = 0; // 0 is fixed
                current->length = 0; // 0 is fixed
        }

        for(int i=core_size; i<page_size; i++){
                struct core_page* current = pages+i;

                paddr_t current_paddr = firstPaddr + i*PAGE_SIZE; // get current physical address
                current->pa = current_paddr;
                current->as = curproc_getas();
                //current->va = PADDR_TO_KVADDR(current_paddr); // get current virtual address

		current->order = 0; // 0 refer
                current->state = 1; // 1 is free
                current->length = 0; // 1 is free
        }
}

// check whether it has n consecutive pages starting at index i
bool coremap_check_pages(int index, int npages){
	for(int i=index; i<index+npages; i++){
		struct core_page* current = pages+i;
		if(current->state != 1){ // it is not free, return false	
			return false;
		}
	}
	return true;
}

// get the next available free pages
int coremap_get_free(int index){
        for(int i=index; i<page_size; i++){
                struct core_page* current = pages+i;
                if(current->state == 1){ // it is free, return index
                        return i;
                }
        }
        return -1;
}

// mark n consecutive pages as occupied, zero out physical/virtual addresses & return the first virtual address
paddr_t coremap_occupy_pages(int index, int npages){
	for(int i=index; i<index+npages; i++){
		struct core_page* current = pages+i;

                current->as = curproc_getas();
		current->state = 2; // mark it dirty(occupied)
		//current->order += 1; // increase the order 
                current->length = npages;
	}
	struct core_page* valid = pages+index;

	// zero out virtual address when we allocate(need to consider place later)
        //bzero((void *)valid->pa, npages * PAGE_SIZE);
//kprintf("BZEROR IN OCCupy %p\n", (void *)PADDR_TO_KVADDR(valid->pa));
        //bzero((void *)PADDR_TO_KVADDR(valid->pa), npages * PAGE_SIZE);

	return valid->pa;
}

// consume a paddr_t and return the counter
int coremap_getorder(paddr_t pa){
        for(int i=0; i<page_size; i++){
                struct core_page* current = pages+i; 

                if(current->pa == pa){
                	//current->order += 1; // increase the order
			return current->order;
		}	

        }
kprintf("GET_order %d limit %d\n", pa, (pages+page_size-1)->pa);
	KASSERT( true == false);
	return -1;
}

// mark n consecutive pages & swap out as occupied, zero out physical/virtual addresses 
void coremap_swap_out(int index, int npages){
	for(int i=index; i<index+npages; i++){
kprintf("coremap occupy swap %d npage %d index %d\n", i, npages ,index);
		struct core_page* current = pages+i;

		// *********
		// this is swap out to SWAPFILE
		swap_out(current->pa, current->as, current->order);
                //current->as = curproc_getas();
		current->order += 1; // increase the order 
                current->length = 0;
		current->state = 1; // mark it free

	}
	// zero out virtual address when we allocate
	//struct core_page* valid = pages+index;
        //bzero((void *)PADDR_TO_KVADDR(valid->pa), npages * PAGE_SIZE);
}

// get npages by replacing n pages(i.e. swap out to make it free)
paddr_t coremap_occupy_victim(int npages){
	int start = coremap_get_victim();
	kprintf("now should be in victim with npage %d start %d\n", npages, start);
	int i=start;
	while(i < start+npages){
		kprintf("in while %d\n", i);
	//for(int i=start; i<start+npages; i++){
		struct core_page* current = pages+i;

		// it is diry(swpped before), so call helper then continue
		// notice i >>>> start+npages since we remove all length memory from record
		if(current->state == 2){
			// do not swap out length, but only npages
			int length = current->length;
			coremap_swap_out(i, length);
			//coremap_swap_out(i, npages);
			//i = i+npages;
		}else if(current->state == 1){
		    kprintf("occupy available as\n");
       	         	current->as = curproc_getas();
                	current->length = npages;
                	//current->order += 1;
			current->state = 2; // mark it dirty(occupied)
			i++;
		}else{
			KASSERT(true == false);
		}
	}
	struct core_page* valid = pages+start;
	kprintf("done occupy at %d with address %p while limit %p\n", start, (void*) valid->pa, (void*)(pages+page_size-1)->pa);

	// zero out virtual address when we allocate
        //bzero((void *)valid->pa, npages * PAGE_SIZE);
        //bzero((void *)PADDR_TO_KVADDR(valid->pa), npages * PAGE_SIZE);

	return valid->pa;
}

// allocate n pages in physical memory
paddr_t coremap_alloc(int n){
	//kprintf("CORE_ ALLOC\n");	

	lock_acquire(corelock);	
	//int spl = splhigh();

	int i=0;
	while(i < page_size){
		if(!coremap_check_pages(i, n)){
			// whether the coremap_get free actually increase
			int potential_next = coremap_get_free(i);
	//kprintf("i %d potential next %d\n", i, potential_next);
			if(potential_next == -1){ // there is no free page
				i = page_size;
			}else if(i == potential_next){
				i++;
			}else{
				i = potential_next;
			}
			//i++;
	//kprintf("you want %d and give %d\n", n, i);
		}else{
			paddr_t rt = coremap_occupy_pages(i,n);
	kprintf("you want %d and give %d &  page limit %d & give you %p\n", n, i, page_size, (void*)rt);
			lock_release(corelock);
			return rt;
			//return coremap_occupy_pages(i, n);
		}
	}

	// !! SWAP FILE since it is full now
	paddr_t rt = coremap_occupy_victim(n);
	kprintf("use victim to find %d and gie you %p\n", rt, (void*)rt);
	lock_release(corelock);
	return rt;
	//return coremap_occupy_victim(n);
	// if it reaches here, should be a problem
	//splx(spl);
	return -1;
}

// free coremap 
void coremap_free(vaddr_t addr){
	lock_acquire(corelock);
	//int spl = splhigh();
	int start=0; // start point to free

	struct core_page* current = pages+start;
	// loop to find starting point
	while(PADDR_TO_KVADDR(current->pa) != addr){
		start++;
                current = pages + start;
		if(start == page_size){
			kprintf("cannot find freed page\n");
		}
	}

 	//use START point to free LENGTH memories	
	int length = current->length;
//kprintf("!!!! free start %d & length %d\n", start, length);
	for(int i=start; i<start + length; i++){
		current = pages+i;
		current->as = NULL;
		current->state = 1; // flag it to free
		current->length = 0;
	}

	lock_release(corelock);
	//splx(spl);
	// this is for shoot-down? need modify later
	as_activate();
}

// swap in to core map
// 1. swap in the original physical address, 2.
void coremap_swapin(paddr_t pa, int order){
	lock_acquire(corelock);

	// loop to find starting point
	for(int i=0; i< page_size; i++){
		struct core_page* current = pages+i;
		if(current->pa == pa){
			// !!! Swap OUT the current one to make it available
			swap_out(current->pa, current->as, current->order);
			// we only swap in one page at one time
			current->order = order;
			current->length = 1; 
			current->state = 2; 
			break;
		}
	}
	lock_release(corelock);
}

