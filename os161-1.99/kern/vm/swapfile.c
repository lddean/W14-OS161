#include <swapfile.h>

//struct vnode *swap_file;
//struct array* swap_record; // files descriptors
static bool swap_boot = false;
// this is 2304 in numerical
static unsigned page_limit = SWAP_SIZE / PAGE_SIZE;
static unsigned int free = 0; // the first free index
//struct swap* swap_record[2304];
struct swap* swap_record;
//paddr_t swap_record[2304];

// initialize the variable for swap operation
int swap_init(void){
	//swap_boot = true;
	//swap_record = array_create();
	swap_lock = lock_create("swap lock");
	//sm = sem_create("swap", 2);
	swap_wlock = lock_create("swap writelock");
	
	swap_record = kmalloc(sizeof(struct swap) * page_limit);
	/*for(int unsigned i=0; i<page_limit; i++){
		kprintf("array %p\n", &swap_record[i]);
	}*/
	
	// iterate 9MB record for later use
	/*for(unsigned int i=0; i<page_limit; i++){
	//for(unsigned int i=0; i<200; i++){
kprintf("kmalloc %d and page limit %d\n", i, page_limit);
        	struct swap* insert = kmalloc(sizeof(struct swap));
kprintf("done kmalloc\n");
       		insert->pa = 0;
        	insert->as = NULL;
		unsigned  ck;
		array_add(swap_record, &insert, &ck);
	}*/

       	char file[] = "SWAPFILE";
	int result = vfs_open(file, O_RDWR|O_CREAT|O_TRUNC, 0, &swap_file);
	if(result){
kprintf("open failsss\n");
		return result;
	}
	return 0;
}

// update the array and check whether it is hit the page size
int update_record(paddr_t pa, struct addrspace* as, int order){
	// first let us check whether it is already there
kprintf("oUPDATE pa %d order %d \n", pa, order);
	 for(unsigned int i=0; i<free; i++){
                struct swap check = swap_record[i];
                if(check.as == as && check.pa == pa && check.order == order){
kprintf("ALREADY there\n");
                        return -1;
                }
        }
	
	// find the free spot to record
	//struct swap* insert = array_get(swap_record, free);
kprintf("we add \n");
	struct swap insert = swap_record[free];
	insert.pa = pa;
	insert.as = as;
	insert.order = order;
	swap_record[free] = insert;
//kprintf("update record %d\n", free);
//printf("oUPDATE pa %d s_pa %d o_as %p & s_as %p \n", pa, insert.pa, as, insert.as);
	free++;
	//if(size == page_limit){
	if(free == page_limit){
		panic("Out of swap space");
	}

	return 0;
	//unsigned ck;
	//array_add(swap_record, &insertion, &ck);
}

// get offset from swap_record
int check_offset(paddr_t pa, struct addrspace* as, int order){
	//int size = array_num(swap_record);
	//int size = page_size;

        //for(int i=0; i<size; i++){
        for(unsigned int i=0; i<free; i++){
	 	//struct swap* check = array_get(swap_record, i);
//kprintf("check offset %d \n", i);
	 	struct swap check = swap_record[i];
//kprintf("orignal pa %d s_pa %d o_as %p & s_as %p order %d & s_order %d\n", pa, check.pa, as, check.as, order, check.order);
		if(check.as == as && check.pa == pa && check.order == order){
			return i;
		}
        }
	// if it goes here, it annot find offset, wrong
	KASSERT(1 == 2);
	return -1;
}

// read page from swap_file
int read_page(paddr_t pa, int offset){
  	struct iovec iov;
	struct uio u;

	uio_kinit(&iov, &u, (void*) PADDR_TO_KVADDR(pa), PAGE_SIZE, offset * PAGE_SIZE, UIO_READ);

	int result = VOP_READ(swap_file, &u);
	if(result){
		return -1;
	}
	
	return 0;
}

// swap in (read a page from SWAPFILE)
//void swap_in(paddr_t pa, struct addrspace* as){
void swap_in(paddr_t pa, int order){
	lock_acquire(swap_lock);
	//P(sm);
kprintf("SWAP in\n");
	
	struct addrspace* as = curproc_getas();

	coremap_swapin(pa, order);

	int offset = check_offset(pa, as, order);
//kprintf("swap_in offset %d\n", offset);
	read_page(pa, offset);
	//V(sm);
	lock_release(swap_lock);
}

// write page to swap_file
int write_page(paddr_t pa, int offset){ 
  	struct iovec iov;
	struct uio u;

	uio_kinit(&iov, &u, (void*) PADDR_TO_KVADDR(pa), PAGE_SIZE, offset * PAGE_SIZE, UIO_WRITE);

	int result = VOP_WRITE(swap_file, &u);
	if(result){
		return -1;
	}
	
	return 0;
}

// swap out (write a page to SWAPFILE)
void swap_out(paddr_t pa, struct addrspace* as, int order){
	lock_acquire(swap_wlock);
	// boots up at the first swap out
	if(!swap_boot){
		//swap_init();
		swap_boot = true;
       	char file[] = "SWAPFILE";
kprintf("OPEN VFS\n");
	int result = vfs_open(file, O_RDWR|O_CREAT|O_TRUNC, 0, &swap_file);
	if(result){
kprintf("open fails\n");
	}
	}
	
kprintf("SWAP OUT\n");
	//lock_acquire(swap_lock);
	/*struct swap* insert = kmalloc(sizeof(struct swap));
	insert->pa = pa;
	insert->as = as;*/
	// update the record as well
//kprintf("UPDATE record\n");
	update_record(pa, as, order);
//kprintf("done update && GOTO Page_INVAL \n");

	// tell page table to make them invalid
	page_invalid(as->page_table, pa, order);
	
	int offset = check_offset(pa, as, order);
//kprintf("offset %d\n", offset);
	write_page(pa, offset);
	//lock_release(swap_lock);
	lock_release(swap_wlock);
	//V(sm);
}
