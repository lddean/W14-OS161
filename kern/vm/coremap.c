#include "opt-A3.h"
#include <coremap.h>
#include <types.h>
#include <queue.h>
#include <vm.h>
#include <spinlock.h>
#include <lib.h>

static int num_pages = 0;
static int order_count = 0;
static struct page *coremap = NULL;
static struct spinlock cm_lock = SPINLOCK_INITIALIZER;

void cm_bootstrap(){
	spinlock_acquire(&cm_lock);

	paddr_t first_addr, last_addr; 
	int range, first_free;
	ram_getsize(&first_addr, &last_addr);
	range = last_addr - first_addr;
	num_pages = ((PAGE_SIZE - range % PAGE_SIZE) + range) / PAGE_SIZE;
	coremap = (struct page *) PADDR_TO_KVADDR(first_addr);
	first_free = (first_addr + num_pages * sizeof(struct page)) / PAGE_SIZE;

	struct page *target_page;
	paddr_t pa = first_addr & PAGE_FRAME;
	for (int i = 0; i < first_free; i++){
		target_page = &coremap[i];
		target_page->state = FIXED;
		target_page->paddr = pa;
		target_page->length = 0;
		target_page->order = -1;
		pa += PAGE_SIZE;
	}
	coremap[0].length = first_free;
	for (int j = first_free; j < num_pages; j++){
		target_page = &coremap[j];
		target_page->state = FREE;
		target_page->paddr = pa;
		target_page->length = 0;
		target_page->order = -1;
		pa += PAGE_SIZE;
	}

	spinlock_release(&cm_lock);
}

paddr_t cm_getpages(unsigned long npages){
	int i = 0;
	uint32_t nfree = 0;
	int tightest_index = -1;
	uint32_t tightest_diff = num_pages;
	struct page *cur_page;

	spinlock_acquire(&cm_lock);
	while(i < num_pages){
		cur_page = &coremap[i];
		if (cur_page->state == FIXED || cur_page->state == DIRTY){
			i += cur_page->length;
			continue;
		}

		nfree = 0;
		while((cur_page->state == FREE) && ((i + (int)nfree) < num_pages)){
			nfree++;
			cur_page = &coremap[i+nfree];
		}

		if(nfree == npages){
			tightest_index = i;
			break;
		}
		else if ((nfree > npages) && ((nfree - npages) < tightest_diff)){
			tightest_index = i;
			tightest_diff = (nfree - npages);
		}
		i += nfree;
	}

	struct page *target_page;
	if (tightest_index >= 0){
		for(uint32_t x = 0; x < npages; x++){
			target_page = &coremap[tightest_index + x];
			target_page->state = DIRTY;
			if (npages > 1){
				target_page->state = FIXED;
			}
			target_page->order = order_count;
			order_count++;
		}
		coremap[tightest_index].length = npages;
	}
	else{} //cannot get npages from coremap
	spinlock_release(&cm_lock);
	return coremap[tightest_index].paddr;
}

int cm_freepages(vaddr_t vaddr){
	vaddr_t cur_vaddr;
	struct page *cur_page;
	struct page *target_page;
	int i = 0;

	spinlock_acquire(&cm_lock);
	while(i < num_pages){
		cur_page = &coremap[i];
		if (cur_page->state != DIRTY){
			i++;
			continue;
		}

		cur_vaddr = PADDR_TO_KVADDR(coremap[i].paddr);

		if (vaddr == cur_vaddr){		

			for(int j = 0; j < cur_page->length; j++){
				target_page = &coremap[i+j];
				target_page->state = FREE;
				target_page->length = 0;
				target_page->order = -1;
			}
			spinlock_release(&cm_lock);

			return 0;
		}
		i += cur_page->length;
	}

	spinlock_release(&cm_lock);
	return -1;
}
