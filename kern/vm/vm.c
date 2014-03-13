
#ifdef UW
/* This was added just to see if we can get things to compile properly and
 * to provide a bit of guidance for assignment 3 */

#include "opt-A3.h"
#include "opt-vm.h"
#if OPT_VM

#include <page.h>
#include <types.h>
#include <kern/errno.h>
#include <lib.h>
#include <addrspace.h>
#include <vm.h>
#include <coremap.h>
#include <spinlock.h>
#include <spl.h>
#include <proc.h>
#include <current.h>
#include <mips/tlb.h>
#include <kern/fcntl.h>
#include <vfs.h>
#include <syscall.h>
#include <test.h>

//dumbvm
static struct spinlock stealmem_lock = SPINLOCK_INITIALIZER;
static struct spinlock tlb_lock = SPINLOCK_INITIALIZER;
static bool vm_started = false;

void
vm_bootstrap(void)
{
	cm_bootstrap();
	vm_started = true;
	/* May need to add code. */
}

#if 0
/* You will need to call this at some point */
static
paddr_t
getppages(unsigned long npages)
{
   /* Adapt code form dumbvm or implement something new */
	 (void)npages;
	 panic("Not implemented yet.\n");
   return (paddr_t) NULL;
}
#endif

//dumbvm

paddr_t
getppages(unsigned long npages)
{
	paddr_t addr;

	if (!vm_started){
		spinlock_acquire(&stealmem_lock);
		addr = ram_stealmem(npages);
		spinlock_release(&stealmem_lock);
	}
	else{
		addr = cm_getpages(npages);
	}
	return addr;
}

//dumbvm
vaddr_t
alloc_kpages(int npages)
{
	paddr_t pa;
	pa = getppages(npages);
	if (pa==0) {
		return pa;
	}
	return PADDR_TO_KVADDR(pa);
}

void
free_kpages(vaddr_t addr)
{
	cm_freepages(addr);
}

void
vm_tlbshootdown_all(void)
{
	panic("3Not implemented yet.\n");
}

void
vm_tlbshootdown(const struct tlbshootdown *ts)
{
	(void)ts;
	panic("2Not implemented yet.\n");
}

// #if OPT-A3
int tlb_get_rr_victim()
{
	int victim;
	static unsigned int next_victim = 0;
	victim = next_victim;
	next_victim = (next_victim + 1) % NUM_TLB;
	return victim;
}
// #endif

int
vm_fault(int faulttype, vaddr_t faultaddress)
{
	// #if OPT-A3
	kprintf("faultaddress = %d\n", faultaddress);
	vaddr_t vbase1, vtop1, vbase2, vtop2, stackbase, stacktop;
	paddr_t paddr;
	int i;
	uint32_t ehi, elo;
	struct addrspace *as;

	faultaddress &= PAGE_FRAME;

	DEBUG(DB_VM, "dumbvm: fault: 0x%x\n", faultaddress);

	switch (faulttype) {
	    case VM_FAULT_READONLY:
			return EFAULT;
		case VM_FAULT_READ:
	    case VM_FAULT_WRITE:
		break;
	    default:
		return EINVAL;
	}

	if (curproc == NULL) {
		/*
		 * No process. This is pdebugPrintrobably a kernel fault early
		 * in boot. Return EFAULT so as to panic instead of
		 * getting into an infinite faulting loop.
		 */
		return EFAULT;
	}

	as = curproc_getas();
	if (as == NULL) {
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
	//KASSERT(as->as_pbase2 != 0);
	KASSERT(as->as_npages2 != 0);
	KASSERT((as->as_vbase1 & PAGE_FRAME) == as->as_vbase1);
	//KASSERT((as->as_pbase1 & PAGE_FRAME) == as->as_pbase1);
	KASSERT((as->as_vbase2 & PAGE_FRAME) == as->as_vbase2);
	//KASSERT((as->as_pbase2 & PAGE_FRAME) == as->as_pbase2);
	KASSERT((as->as_stackvbase & PAGE_FRAME) == as->as_stackvbase);

	vbase1 = as->as_vbase1;
	vtop1 = vbase1 + as->as_npages1 * PAGE_SIZE;
	vbase2 = as->as_vbase2;
	vtop2 = vbase2 + as->as_npages2 * PAGE_SIZE;
	stackbase = USERSTACK - DUMBVM_STACKPAGES * PAGE_SIZE;
	stacktop = USERSTACK;

	// if (faultaddress >= vbase1 && faultaddress < vtop1) {
	// 	paddr = (faultaddress - vbase1) + as->as_pbase1;
	// }
	// else if (faultaddress >= vbase2 && faultaddress < vtop2) {
	// 	paddr = (faultaddress - vbase2) + as->as_pbase2;
	// }
	// else if (faultaddress >= stackbase && faultaddress < stacktop) {
	// 	paddr = (faultaddress - stackbase) + as->as_stackpbase;
	// }
	// else {
	// 	return EFAULT;
	// }

	/* make sure it's page-aligned */
	KASSERT((paddr & PAGE_FRAME) == paddr);
	
    
    if (!page_exist(faultaddress, as->page_table)){

        paddr_t newpaddr = getppages(1);
        
        if (faultaddress <= as -> code_memsize + as -> as_vbase1 && faultaddress >= as -> as_vbase1){

        	off_t offset;
        	offset = faultaddress - as -> as_vbase1 + as ->code_offset;
        	int result;
        	result = load_segment(as, as -> vnode, as -> code_offset,faultaddress,
        		newpaddr, 
        		as -> code_memsize, as -> code_filesize,as -> executable);
        	if (result){
        		return 0;
        	}

        }else if(as -> as_vbase2 <= faultaddress && faultaddress <= as -> data_memsize + as -> as_vbase2){

        	off_t offset;
        	offset = faultaddress - as -> as_vbase2 + as ->data_offset;
        	int result;
        	result = load_segment(as, as -> vnode, as -> data_offset,faultaddress,
        		newpaddr, 
        		as -> data_memsize, as -> data_filesize,as -> executable);
        	if (result){
        		return 0;
        	}

        }

        add_vaddr_to_table(faultaddress, newpaddr, as->page_table);

    }else{
    	kprintf("fuckkkkk\n");
    }

	/* Disable interrupts on this CPU while frobbing the TLB. */
	spinlock_acquire(&tlb_lock);

	for (i=0; i<NUM_TLB; i++) {
		tlb_read(&ehi, &elo, i);
		if (elo & TLBLO_VALID) {
			continue;
		}
		ehi = faultaddress;
		elo = paddr | TLBLO_DIRTY | TLBLO_VALID;
		DEBUG(DB_VM, "dumbvm: 0x%x -> 0x%x\n", faultaddress, paddr);
		tlb_write(ehi, elo, i);
		spinlock_release(&tlb_lock);
		return 0;
	}

	ehi = faultaddress;
	elo = paddr | TLBLO_DIRTY | TLBLO_VALID;
	int victim = tlb_get_rr_victim();
	tlb_write(ehi, elo, victim);
	spinlock_release(&tlb_lock);
	return 0;

	// #else
 //  /* Adapt code form dumbvm or implement something new */
	// (void)faulttype;
	// (void)faultaddress;
	// panic("1Not implemented yet.\n");
 //  	return 0;

 //  	#endif
}

#endif /* OPT_VM */

#endif /* UW */

