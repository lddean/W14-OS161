#include <swapfile.h>
#include <uw-vmstats.h>

static bool swap_boot = false;
// this is 2304 in numerical
static unsigned page_limit = SWAP_SIZE / PAGE_SIZE;
static unsigned int free = 0; // the first free index
struct swap* swap_record;

// initialize the variable for swap operation
int swap_init(void){
	swap_lock = lock_create("swap lock");
	swap_wlock = lock_create("swap writelock");
	
	swap_record = kmalloc(sizeof(struct swap) * page_limit);

       	char file[] = "SWAPFILE";
	int result = vfs_open(file, O_RDWR|O_CREAT|O_TRUNC, 0, &swap_file);
	if(result){
		return result;
	}
	return 0;
}

// update the array and check whether it is hit the page size
int update_record(paddr_t pa, struct addrspace* as, int order){
	// first let us check whether it is already there
	 for(unsigned int i=0; i<free; i++){
                struct swap check = swap_record[i];
                if(check.as == as && check.pa == pa && check.order == order){
                        return -1;
                }
        }
	
	// find the free spot to record
	struct swap insert = swap_record[free];
	insert.pa = pa;
	insert.as = as;
	insert.order = order;
	swap_record[free] = insert;
	free++;
	if(free == page_limit){
		panic("Out of swap space");
	}

	return 0;
}

// get offset from swap_record
int check_offset(paddr_t pa, struct addrspace* as, int order){
        for(unsigned int i=0; i<free; i++){
	 	struct swap check = swap_record[i];
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
void swap_in(paddr_t pa, int order){
	lock_acquire(swap_lock);

        vmstats_inc(VMSTAT_PAGE_FAULT_DISK);
        vmstats_inc(VMSTAT_SWAP_FILE_READ);
	
	struct addrspace* as = curproc_getas();

	coremap_swapin(pa, order);

	int offset = check_offset(pa, as, order);
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
void swap_out(paddr_t pa, struct addrspace* as, int order){
	lock_acquire(swap_wlock);
        vmstats_inc(VMSTAT_SWAP_FILE_WRITE);
	// boots up at the first swap out
	if(!swap_boot){
		swap_boot = true;
       		char file[] = "SWAPFILE";

		int result = vfs_open(file, O_RDWR|O_CREAT|O_TRUNC, 0, &swap_file);
		if(result){
			kprintf("open fails\n");
		}
	}
	
	// update the record as well
	update_record(pa, as, order);

	// tell page table to make them invalid
	page_invalid(as->page_table, pa, order);
	
	int offset = check_offset(pa, as, order);
	write_page(pa, offset);
	lock_release(swap_wlock);
}
