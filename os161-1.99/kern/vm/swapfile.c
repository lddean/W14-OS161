#include <swapfile.h>

//struct vnode *swap_file;
//struct array* swap_record; // files descriptors
static bool swap_boot = false;
static unsigned page_limit = SWAP_SIZE / PAGE_SIZE;

// initialize the variable for swap operation
int swap_init(void){
	swap_boot = true;
	swap_record = array_create();
	swap_lock = lock_create("swap lock");
	
       	char file[] = "SWAPFILE";
	int result = vfs_open(file, 0, O_RDWR , &swap_file);
	if(result){
		return result;
	}
	return 0;
}

// update the array and check whether it is hit the page size
void update_record(struct swap* insertion){
	// get the size limit we panic
	unsigned size = array_num(swap_record);
	if(size == page_limit){
		panic("Out of swap space");
	}

	unsigned ck;
	array_add(swap_record, &insertion, &ck);
}

// get offset from swap_record
int check_offset(paddr_t pa, struct addrspace* as){
	int size = array_num(swap_record);

        for(int i=0; i<size; i++){
	 	struct swap* check = array_get(swap_record, i);
		if(check->as == as && check->pa == pa){
			return i;
		}
        }
	return -1;
}

// read page from swap_file
int read_page(paddr_t pa, int offset){
	if(!swap_boot){
		swap_init();
	}
	
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
void swap_in(paddr_t pa, struct addrspace* as){
	lock_acquire(swap_lock);
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
	lock_acquire(swap_lock);
	struct swap* insert = kmalloc(sizeof(struct swap));
	insert->pa = pa;
	insert->as = as;

	// update the record as well
	update_record(insert);

	// tell page table to make them invalid
	page_invalid(as->page_table, pa);
	
	int offset = check_offset(pa, as);
	write_page(pa, offset);
	lock_release(swap_lock);
}
