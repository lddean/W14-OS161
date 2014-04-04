/*#include <types.h>
#include <kern/errno.h>
#include <lib.h>
#include <spl.h>
#include <vnode.h>
#include <spinlock.h>
#include <proc.h>
#include <current.h>
#include <mips/tlb.h>
#include <addrspace.h>
#include <pt.h>
#include <coremap.h>*/

/*
 * Dumb MIPS-only "VM system" that is intended to only be just barely
 * enough to struggle off the ground. You should replace all of this
 * code while doing the VM assignment. In fact, starting in that
 * assignment, this file is not included in your kernel!
 */

#include <kern/errno.h>
#include <vm.h>
#include <uw-vmstats.h>
/* under dumbvm, always have 48k of user stack */
#define DUMBVM_STACKPAGES    12

/*
 * Wrap rma_stealmem in a spinlock.
 */
static struct spinlock stealmem_lock = SPINLOCK_INITIALIZER;

int times;
bool boot=false;

void
vm_bootstrap(void)
{
//kprintf("swap init\n");
    vmstats_init();
	swap_init();
//kprintf("coremap init\n");
	coremap_init();
	boot = true;
	/* Do nothing. */
}

paddr_t
getppages(unsigned long npages)
{

//kprintf("in getppages\n");
	if(!boot){
	paddr_t addr;
    
	spinlock_acquire(&stealmem_lock);
    
	addr = ram_stealmem(npages);
//kprintf("after ram-stealmem\n");
	spinlock_release(&stealmem_lock);
	return addr;
	}else{
	//kprintf("here else\n");
		paddr_t addr;
	//kprintf("before the alloc\n");
		addr = coremap_alloc(npages);
	//kprintf("after the alloc\n");
		return addr;
	}
}

/* Allocate/free some kernel-space virtual pages */
vaddr_t
alloc_kpages(int npages)
{
	paddr_t pa;
	pa = getppages(npages);
	if (pa==0) {
		return 0;
	}
	return PADDR_TO_KVADDR(pa);
}

void
free_kpages(vaddr_t addr)
{
	/* nothing - leak the memory. */
	coremap_free(addr);
    
	(void)addr;
}

void
vm_tlbshootdown_all(void)
{
	panic("dumbvm tried to do tlb shootdown?!\n");
}

void
vm_tlbshootdown(const struct tlbshootdown *ts)
{
	(void)ts;
	panic("dumbvm tried to do tlb shootdown?!\n");
}

int tlb_get_rr_victim(){
	int victim;
	static unsigned int next_victim = 0;
	victim = next_victim;
	next_victim = (next_victim + 1) % NUM_TLB;
	return victim;
}

int loading_page(struct addrspace *as, vaddr_t vbase,struct vnode *v, off_t offset, vaddr_t vaddr, paddr_t paddr, size_t memsize, size_t filesize, int is_executable){
    
	// ununsed for NOW !!!!
    (void) as;
	(void) is_executable;
    
    struct iovec iov;
    struct uio u;
    int result;
    
    if (filesize > memsize){
        
        kprintf("ELF: warning: segment filesize > segment memsize\n");
		filesize = memsize;
        
    }
    
    DEBUG(DB_EXEC, "ELF: Loading %lu bytes to 0x%lx\n",
	      (unsigned long) filesize, (unsigned long) vaddr);
    
    iov.iov_ubase = (userptr_t)PADDR_TO_KVADDR(paddr);
 	iov.iov_len = PAGE_SIZE;		 // length of the memory space
	u.uio_iov = &iov;
	u.uio_iovcnt = 1;
	u.uio_resid = 0;          // amount to read from the file
	u.uio_offset = offset;
	u.uio_segflg = UIO_SYSSPACE;
	u.uio_rw = UIO_READ;
	u.uio_space = NULL;
    
    int check = 0;
    
    size_t need_resid = 0;
    
    if ((vaddr - vbase < filesize) && (vaddr + PAGE_SIZE > vbase + filesize)){
        
        
        need_resid = vbase - vaddr + filesize;
        
        check ++;
        
    }else if((vaddr - vbase < filesize) && (vaddr + PAGE_SIZE <= vbase + filesize)) {
        
        
        need_resid = PAGE_SIZE;
        
        check ++;
    }else{
        
        need_resid = 0;
    }
    
    if (need_resid !=0){
        
        u.uio_resid = need_resid;
        //kprintf("the vop_read in loading page\n");
        result = VOP_READ(v, &u);
        //kprintf("finished vop_read in loading page\n");
        check ++;
        
        if (result) {
            return result;
        }
        
        check ++;
        
        if (u.uio_resid != 0) {
            /* short read; problem with executable? */
            kprintf("ELF: short read on segment - file truncated?\n");
            return ENOEXEC;
        }
        
        vmstats_inc(VMSTAT_PAGE_FAULT_DISK);
        vmstats_inc(VMSTAT_ELF_FILE_READ);
    }else{
        vmstats_inc(VMSTAT_PAGE_FAULT_ZERO);
    }
    
    check ++;
    
    
    return 0;
    
    
}


int
vm_fault(int faulttype, vaddr_t faultaddress)
{
    vmstats_inc(VMSTAT_TLB_FAULT);
	//kprintf("Begin vm_fault\n");
	vaddr_t vbase1, vtop1, vbase2, vtop2, stackbase, stacktop;
	paddr_t paddr;
	int i;
	uint32_t ehi, elo;
	struct addrspace *as;
	int spl;
    
    int result;
    
    int segment = 0;
    
	faultaddress &= PAGE_FRAME;
    
	DEBUG(DB_VM, "dumbvm: fault: 0x%x\n", faultaddress);
    
	switch (faulttype) {
	    case VM_FAULT_READONLY:
            /* We always create pages read-write, so we can't get this */
            //panic("DUMBVM!!!!!!!!!!!!!!!!!!!!!!!!!!!!: got VM_FAULT_READONLY\n");
            kprintf("VM_FAULT_READONLY\n");
            return EFAULT;
	    case VM_FAULT_READ:
	    case VM_FAULT_WRITE:
            break;
	    default:
            return EINVAL;
	}

	if (curproc == NULL) {
		 kprintf("curproc is NULL\n");

		/*
		 * No process. This is probably a kernel fault early
		 * in boot. Return EFAULT so as to panic instead of
		 * getting into an infinite faulting loop.
		 */
		return EFAULT;
	}
	//kprintf("Begin2 vm_fault\n");
	
	as = curproc_getas();

	if (as == NULL) {
	  kprintf("as is NULL\n");

		/*
		 * No address space set up. This is probably also a
		 * kernel fault early in boot.
		 */
		return EFAULT;
	}

	/* Assert that the address space has been set up properly. */
	KASSERT(as->as_vbase1 != 0);
	//KASSERT(as->as_pbase1 != 0);
	KASSERT(as->as_npages1 != 0); 
	KASSERT(as->as_vbase2 != 0);
	KASSERT(as->as_npages2 != 0);
	KASSERT(as->as_stackpbase != 0);
	KASSERT((as->as_vbase1 & PAGE_FRAME) == as->as_vbase1);
	KASSERT((as->as_vbase2 & PAGE_FRAME) == as->as_vbase2);
	KASSERT((as->as_stackpbase & PAGE_FRAME) == as->as_stackpbase);
	vbase1 = as->as_vbase1;
	vtop1 = vbase1 + as->as_npages1 * PAGE_SIZE;
	vbase2 = as->as_vbase2;
	vtop2 = vbase2 + as->as_npages2 * PAGE_SIZE;
	stackbase = USERSTACK - DUMBVM_STACKPAGES * PAGE_SIZE;
	stacktop = USERSTACK;
    /*
     if (faultaddress >= vbase1 && faultaddress < vtop1) {
     paddr = (faultaddress - vbase1) + as->as_pbase1;
     }
     else if (faultaddress >= vbase2 && faultaddress < vtop2) {
     paddr = (faultaddress - vbase2) + as->as_pbase2;
     }
     else if (faultaddress >= stackbase && faultaddress < stacktop) {
     paddr = (faultaddress - stackbase) + as->as_stackpbase;
     }
     else {
     return EFAULT;
     }*/
    if ( faultaddress >= as -> as_vbase1 && faultaddress <= as -> as_vbase1 + as ->memsize1){
        segment = 1;
    }else{
        segment = 2;
    }
   
    if (!page_exist(as->page_table, faultaddress)){
        
        times = 0;
        off_t offset;
        
        paddr = getppages(1);
        
        if (segment == 1){
           // segment=1;
            times = times + 1;
            offset = faultaddress - as -> as_vbase1 + as -> offset1;
            
            result = loading_page(as, as->as_vbase1, as ->vnode, offset, faultaddress, paddr, as->memsize1, as -> filesize1, as -> executable);
            
            times = times + 1;
            if (result){
                
                times = times + 1;
                return ENOMEM;
            }
        }
        
        if(segment == 2){
            
            times = times + 1;
            offset = faultaddress - as -> as_vbase2 + as -> offset2;
            
            result = loading_page(as, as->as_vbase2 ,as ->vnode, offset, faultaddress, paddr, as->memsize2, as -> filesize2, as -> executable);
            
            times = times + 1;
            if (result){
                
                times = times + 1;
                return ENOMEM;
            }
            
        }
        
        page_table_add(as -> page_table, faultaddress, paddr);
        
        
    }else{
    //kprintf("Begin3 vm_fault\n");
        
        vmstats_inc(VMSTAT_TLB_RELOAD);
	
        paddr = get_paddr(as->page_table, faultaddress);
    
	}/* make sure it's page-aligned */
	KASSERT((paddr & PAGE_FRAME) == paddr);
    
	/* Disable interrupts on this CPU while frobbing the TLB. */
	spl = splhigh();
   	//kprintf("check TLB\n"); 
	for (i=0; i<NUM_TLB; i++) {
		tlb_read(&ehi, &elo, i);
		if (elo & TLBLO_VALID) {
			continue;
		}
		ehi = faultaddress;
		//elo = paddr | TLBLO_DIRTY | TLBLO_VALID;
		if (segment == 1){
			//kprintf("SEGMENT - TEXT1\n");
			elo = (paddr | TLBLO_VALID); //&(~TLBLO_DIRTY);
		}
		else{
		
			elo = paddr | TLBLO_DIRTY | TLBLO_VALID;
		}
        
        vmstats_inc(VMSTAT_TLB_FAULT_FREE);
        
		DEBUG(DB_VM, "dumbvm: 0x%x -> 0x%x\n", faultaddress, paddr);
		tlb_write(ehi, elo, i);
		//kprintf("11111\n");
		splx(spl);
		//kprintf("22222\n");
		return 0;
	}
    int victim_index = tlb_get_rr_victim();
    ehi = faultaddress;
    
    //change_page_valid(as->page_table, faultaddress, 0);
    //page_invalid(as->page_table, paddr, victim_index);
    
    //elo = paddr | TLBLO_DIRTY | TLBLO_VALID;
    if (segment == 1){
    	//kprintf("SEGMENT - TEXT2\n");
		elo = paddr;  //&(~TLBLO_DIRTY);
	}
	else{
		elo = paddr | TLBLO_DIRTY;
	}
    vmstats_inc(VMSTAT_TLB_FAULT_REPLACE);
    
    DEBUG(DB_VM, "dumbvm: 0x%x -> 0x%x\n", faultaddress, paddr);
    tlb_write(ehi, elo, victim_index);
    
    splx(spl);
    return 0;
	//kprintf("dumbvm: Ran out of TLB entries - cannot handle page fault\n");
	//splx(spl);
	//return EFAULT;
}


