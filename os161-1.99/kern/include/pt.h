//#if OPT_A3
#ifndef PT_H
#define PT_H

#include "opt-A3.h"
#include <types.h>
#include <lib.h>
#include <array.h>
#include <synch.h>

/*#include "opt-A3.h"
#include <kern/errno.h>
#include <array.h>
#include <lib.h>
#include <types.h>
#include <vm.h>*/

//structure for each page
struct page{
	vaddr_t va;
	paddr_t pa;
	int segment; //1 - code, 2 - data, 3 - stack
	int valid;
};

//structure for page table
struct page_table{
	struct array *pages; //array of page structures
};
	
struct page_table* page_table_create(void);
struct page* page_create(vaddr_t vaddr, paddr_t paddr);
int page_exist(struct page_table* pgtbl, vaddr_t vaddr);
void page_table_add(struct page_table* pgtbl, vaddr_t vaddr, paddr_t paddr);	
paddr_t get_paddr(struct page_table* pgtbl, vaddr_t vaddr);

#endif
//#endif