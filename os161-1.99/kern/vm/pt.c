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
kprintf("I doubt this\n");
	struct page* new = (struct page *)kmalloc(sizeof(struct page));
kprintf("if this print, i am wrong. I doubt this\n");
	
	if (new==NULL){return NULL;}
	
	new->va = vaddr;
	new->pa = paddr;
	new->order = coremap_getorder(paddr); // this is risky(not right)
	new->segment = -1;
	new->valid = 1; //valid
	return new;
}

// check if page with vaddr exists in page table
int page_exist(struct page_table* pgtbl, vaddr_t vaddr){
	struct page* pg;
	int size = array_num(pgtbl->pages);
	for (int i=0; i<size; i++){
		pg = array_get(pgtbl->pages, i);
	int order = coremap_getorder(pg->pa);
		if (pg->va == vaddr && (pg->valid == 1) && (pg->order == order)){
			return 1;
		}else if(pg->va == vaddr && (pg->valid == 0) && (pg->order == order)){
			swap_in(pg->va, pg->order);
			pg->valid = 1;
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
    
	int size = array_num(pgtbl->pages);
kprintf("TABLE size %d\n", size);
kprintf("ADD PAGE\n");
	struct page* new = page_create(vaddr, paddr);
kprintf("done ADD PAGE\n");
    
	array_add(pgtbl->pages, new, &result);
}

void change_page_valid(struct page_table* pgtbl, vaddr_t vaddr, int validBit){
	struct page* pg;
	int size = array_num(pgtbl->pages);
	for (int i=0; i<size; i++){
		pg = array_get(pgtbl->pages, i);
		if (pg->va == vaddr){
			pg->valid = validBit;
		}
	}
	//return -1; //no page with vaddr exists
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

// ********************************** Tom adds the following
// make a page invalid given its physical address

void page_invalid(struct page_table* pgtbl, paddr_t pa, int order){
        struct page* pg;
        int size = array_num(pgtbl->pages);
        for (int i=0; i<size; i++){
                pg = array_get(pgtbl->pages, i);
                if (pg->pa == pa && pg->order == order){
//kprintf("page invalid with pa %d\n", pa);
			pg->valid = 0;
			//pg->order = order;
			// invalidate the tlb as well
			int i = tlb_probe(pg->va, pg->pa);
			tlb_write(TLBHI_INVALID(i), TLBLO_INVALID(), i);
                }
        }
}

// destroy a file table
void page_table_destroy(struct page_table* pt){

        struct array* pages = pt->pages;

         //KASSERT(fd != NULL); // not null to free
        int size = array_num(pages);

//kprintf("PT Destroy\n");
        for(int i=0; i<size; i++){
//kprintf("PT Destroy %d\n", i);
		struct page* page = array_get(pages, i);
		paddr_t pa = page->pa;
		coremap_free((PADDR_TO_KVADDR(pa)));
                //file_dst_destroy(array_get(fd, i)); // prevent files2 at loop 6
                kfree(array_get(pages, i));
        }

        // destroy array and the pointer
        //array_destroy(fd);
        kfree(pt);
}

