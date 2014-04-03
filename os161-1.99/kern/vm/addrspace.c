/*#include <types.h>
#include <kern/errno.h>
#include <lib.h>
#include <spl.h>
#include <spinlock.h>
#include <proc.h>
#include <current.h>
#include <mips/tlb.h>
#include <vm.h>
#include <pt.h>*/

/*
 * Dumb MIPS-only "VM system" that is intended to only be just barely
 * enough to struggle off the ground. You should replace all of this
 * code while doing the VM assignment. In fact, starting in that
 * assignment, this file is not included in your kernel!
 */
#include <addrspace.h>

/* under dumbvm, always have 48k of user stack */
#define DUMBVM_STACKPAGES    12

/*
 * Wrap rma_stealmem in a spinlock.
 */
//static struct spinlock stealmem_lock = SPINLOCK_INITIALIZER;


struct addrspace *
as_create(void)
{
	struct addrspace *as = kmalloc(sizeof(struct addrspace));
	if (as==NULL) {
		return NULL;
	}
    
	as->as_vbase1 = 0;
	//as->as_pbase1 = 0;
	as->as_npages1 = 0;
	as->as_vbase2 = 0;
	//as->as_pbase2 = 0;
	as->as_npages2 = 0;
	as->as_stackpbase = 0;
    
    as->offset1 = 0;
    as->filesize1 = 0;
    as->memsize1 = 0;
    
    as->offset2 = 0;
    as->filesize2 = 0;
    as->memsize2 = 0;
    
    as->executable = 0;
    as->page_table = page_table_create();
    as->vnode = NULL;
    
    
	return as;
}

void
as_destroy(struct addrspace *as)
{
	//kfree((void*)as->as_stackpbase);
	// free the coremap & destroy page table(they both use getppages to get physical memory)
	coremap_free(PADDR_TO_KVADDR(as->as_stackpbase));
	page_table_destroy(as->page_table);
	kfree(as);
}

void
as_activate(void)
{
	int i, spl;
	struct addrspace *as;
    
	as = curproc_getas();
#ifdef UW
    /* Kernel threads don't have an address spaces to activate */
#endif
    //kprintf("clean the TLB!!!!!!!!!!\n");
	if (as == NULL) {
		return;
	}
    
	/* Disable interrupts on this CPU while frobbing the TLB. */
	spl = splhigh();
    
	for (i=0; i<NUM_TLB; i++) {
		tlb_write(TLBHI_INVALID(i), TLBLO_INVALID(), i);
	}
    
    //kprintf("cleaned the TLB!!!!!!!!!!!\n");
    
	splx(spl);
}

void
as_deactivate(void)
{
	/* nothing */
}

int
as_define_region(struct addrspace *as, vaddr_t vaddr, size_t sz, off_t offset,
                 size_t filesize,
                 int readable, int writeable, int executable)
{
kprintf("begin define region\n");
	size_t npages;
    
	/* Align the region. First, the base... */
	sz += vaddr & ~(vaddr_t)PAGE_FRAME;
	vaddr &= PAGE_FRAME;
    
	/* ...and now the length. */
	sz = (sz + PAGE_SIZE - 1) & PAGE_FRAME;
    
	npages = sz / PAGE_SIZE;
    
	/* We don't use these - all pages are read-write */
	(void)readable;
	(void)writeable;
	(void)executable;
    
	if (as->as_vbase1 == 0) {
		as->as_vbase1 = vaddr;
		as->as_npages1 = npages;
        as->offset1 = offset;
        as->filesize1 = filesize;
        as->memsize1 = sz;
        kprintf("end defien region\n");
		return 0;
	}
    
	if (as->as_vbase2 == 0) {
		as->as_vbase2 = vaddr;
		as->as_npages2 = npages;
        as->offset2 = offset;
        as->filesize2 = filesize;
        as->memsize2 = sz;
kprintf("end defien region\n");
		return 0;
	}
    
	/*
	 * Support for more than two regions is not available.
	 */
	kprintf("dumbvm: Warning: too many regions\n");

	return EUNIMP;
}

static
void
as_zero_region(paddr_t paddr, unsigned npages)
{
	bzero((void *)PADDR_TO_KVADDR(paddr), npages * PAGE_SIZE);
}

int
as_prepare_load(struct addrspace *as)
{/*
	KASSERT(as->as_pbase1 == 0);
	KASSERT(as->as_pbase2 == 0);
	KASSERT(as->as_stackpbase == 0);
    
	as->as_pbase1 = getppages(as->as_npages1);
	if (as->as_pbase1 == 0) {
		return ENOMEM;
	}
    
	as->as_pbase2 = getppages(as->as_npages2);
	if (as->as_pbase2 == 0) {
		return ENOMEM;
	}
    */
kprintf("here 11\n");
	as->as_stackpbase = getppages(DUMBVM_STACKPAGES);
kprintf("here 22\n");
	if (as->as_stackpbase == 0) {
		return ENOMEM;
	}

//	as_zero_region(as->as_pbase1, as->as_npages1);
//	as_zero_region(as->as_pbase2, as->as_npages2);
	as_zero_region(as->as_stackpbase, DUMBVM_STACKPAGES);
    
    
    (void)as;
	return 0;
}

int
as_complete_load(struct addrspace *as)
{
	(void)as;
	return 0;
}

int
as_define_stack(struct addrspace *as, vaddr_t *stackptr)
{
	KASSERT(as->as_stackpbase != 0);
    
	*stackptr = USERSTACK;
	return 0;
}

int
as_copy(struct addrspace *old, struct addrspace **ret)
{ 
	struct addrspace *new;
    
	new = as_create();
	if (new==NULL) {
		return ENOMEM;
	}
    
	new->as_vbase1 = old->as_vbase1;
	new->as_npages1 = old->as_npages1;
	new->as_vbase2 = old->as_vbase2;
	new->as_npages2 = old->as_npages2;
    new->offset1 = old->offset1;
    new->filesize1 = old->filesize1;
    new->memsize1 = old->memsize1;
    new->offset2 = old->offset2;
    new->filesize2 = old->filesize2;
    new->memsize2 = old->memsize2;
    
    
	/* (Mis)use as_prepare_load to allocate some physical memory. */
	if (as_prepare_load(new)) {
		as_destroy(new);
		return ENOMEM;
	}
    
//	KASSERT(new->as_pbase1 != 0);
//	KASSERT(new->as_pbase2 != 0);
	KASSERT(new->as_stackpbase != 0);
    
//	memmove((void *)PADDR_TO_KVADDR(new->as_pbase1),
//            (const void *)PADDR_TO_KVADDR(old->as_pbase1),
//            old->as_npages1*PAGE_SIZE);
    
//	memmove((void *)PADDR_TO_KVADDR(new->as_pbase2),
//            (const void *)PADDR_TO_KVADDR(old->as_pbase2),
//            old->as_npages2*PAGE_SIZE);
    
//	memmove((void *)PADDR_TO_KVADDR(new->as_stackpbase),
//            (const void *)PADDR_TO_KVADDR(old->as_stackpbase),
//            DUMBVM_STACKPAGES*PAGE_SIZE);
	
	*ret = new;
	return 0;
}
