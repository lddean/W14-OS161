//#if OPT_A3
#ifndef PT_H
#define PT_H

#include "opt-A3.h"
#include <types.h>
#include <lib.h>
#include <array.h>
#include <synch.h>
#include <coremap.h>

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
	int valid; // 0 - invalid, 1 - valid
};

//structure for page table
struct page_table{
	struct array *pages; //array of page structures
};
	
struct page_table* page_table_create(void);
struct page* page_create(vaddr_t vaddr, paddr_t paddr);
int page_exist(struct page_table* pgtbl, vaddr_t vaddr);
void page_table_add(struct page_table* pgtbl, vaddr_t vaddr, paddr_t paddr);	
void change_page_valid(struct page_table* pgtbl, vaddr_t vaddr, int validBit);
paddr_t get_paddr(struct page_table* pgtbl, vaddr_t vaddr);

// TOM adds these functions
void page_invalid(struct page_table* pgtbl, paddr_t pa);
void page_table_destroy(struct page_table* pt);
#endif
//#endif
