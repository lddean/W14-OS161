#include <pt.h>

//global page table variable
//static struct page_table* pgtbl = NULL;

//create page table
struct page_table* page_table_create(void){
	
    struct page_table* pgtbl;
    pgtbl = (struct page_table *)kmalloc(sizeof(struct page_table));
	
    if (pgtbl==NULL){
		return NULL;
	}
	pgtbl -> pages = array_create(); //create array of page structures
	return pgtbl;
}

// create a single page
struct page* page_create(vaddr_t vaddr, paddr_t paddr){
	struct page* new = (struct page *)kmalloc(sizeof(struct page));
	
	if (new==NULL){return NULL;}
	
	new->va = vaddr;
	new->pa = paddr;
	new->segment = -1;
	new->valid = 0; //not valid -> ..
	return new;
}

// check if page with vaddr exists in page table
int page_exist(struct page_table* pgtbl, vaddr_t vaddr){
	struct page* pg;
	int size = array_num(pgtbl->pages);
	for (int i=0; i<size; i++){
		pg = array_get(pgtbl->pages, i);
		if (pg->va == vaddr){
			return 1;
		}
	}
	return 0;
}
		
void page_table_add(struct page_table* pgtbl, vaddr_t vaddr, paddr_t paddr){
	if (pgtbl==NULL){
		pgtbl = page_table_create();
	}
	
	if (page_exist(pgtbl,vaddr)){
		return;
	}
	
	unsigned result;
    
	struct page* new = page_create(vaddr, paddr);
    
	array_add(pgtbl->pages, new, &result);
}
	
paddr_t get_paddr(struct page_table* pgtbl, vaddr_t vaddr){
	struct page* pg;
	int size = array_num(pgtbl->pages);
	for (int i=0; i<size; i++){
		pg = array_get(pgtbl->pages, i);
		if (pg->va == vaddr){
			return pg->pa;
		}
	}
	return -1; //no page with vaddr exists
}

