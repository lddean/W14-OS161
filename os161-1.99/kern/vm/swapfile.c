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
//void update_record(struct swap* insertion){
void update_record(paddr_t pa, struct addrspace* as){
	// find the free spot to record
	//struct swap* insert = array_get(swap_record, free);
	struct swap insert = swap_record[free];
	insert.pa = pa;
	insert.as = as;
	free++;

	//if(size == page_limit){
	if(free == page_limit){
		panic("Out of swap space");
	}

	//unsigned ck;
	//array_add(swap_record, &insertion, &ck);
}

// get offset from swap_record
int check_offset(paddr_t pa, struct addrspace* as){
	//int size = array_num(swap_record);
	int size = page_size;

        for(int i=0; i<size; i++){
	 	//struct swap* check = array_get(swap_record, i);
	 	struct swap check = swap_record[i];
		if(check.as == as && check.pa == pa){
			return i;
		}
        }
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
void swap_in(paddr_t pa){
	lock_acquire(swap_lock);
	
	struct addrspace* as = curproc_getas();

	coremap_swapin(pa);

	int offset = check_offset(pa, as);
	read_page(pa, offset);
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
void swap_out(paddr_t pa, struct addrspace* as){
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
	
	lock_acquire(swap_lock);
	/*struct swap* insert = kmalloc(sizeof(struct swap));
	insert->pa = pa;
	insert->as = as;*/
	// update the record as well
	update_record(pa, as);

	// tell page table to make them invalid
	page_invalid(as->page_table, pa);
	
	int offset = check_offset(pa, as);
	write_page(pa, offset);
	lock_release(swap_lock);
}
