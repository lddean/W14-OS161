/*
 * Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2008, 2009
 *	The President and Fellows of Harvard College.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE UNIVERSITY AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE UNIVERSITY OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <types.h>
#include <kern/errno.h>
#include <lib.h>
#include <spl.h>
#include <spinlock.h>
#include <proc.h>
#include <current.h>
#include <mips/tlb.h>
#include <addrspace.h>
#include <vm.h>
<<<<<<< HEAD
=======
#include <spl.h>
#include <mips/tlb.h>
#include <addrspace.h>
#include <current.h>
#include <spinlock.h>
#include <coremap.h>
#include "opt-A3.h"
#ifdef UW
#include <proc.h>
#endif
>>>>>>> fb5a0f064c54551b2a6f2628046c68de54373f23

/*
 * Dumb MIPS-only "VM system" that is intended to only be just barely
 * enough to struggle off the ground. You should replace all of this
 * code while doing the VM assignment. In fact, starting in that
 * assignment, this file is not included in your kernel!
 */

<<<<<<< HEAD
/* under dumbvm, always have 48k of user stack */
#define DUMBVM_STACKPAGES    12

/*
 * Wrap rma_stealmem in a spinlock.
 */
static struct spinlock stealmem_lock = SPINLOCK_INITIALIZER;



struct addrspace *
as_create(void)
{
    =======
#if OPT_A3
    /* under dumbvm, always have 48k of user stack */
#define DUMBVM_STACKPAGES    12
    
    /*
     * Wrap rma_stealmem in a spinlock.
     */
    static struct spinlock stealmem_lock = SPINLOCK_INITIALIZER;
    
    
    static
    paddr_t
    getppages(unsigned long npages)
    {
        paddr_t addr;
        
        spinlock_acquire(&stealmem_lock);
        
        addr = ram_stealmem(npages);
        
        spinlock_release(&stealmem_lock);
        return addr;
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
        
        /*for(int i=0; i<page_size; i++){
         struct page* current = pages+i;
         vaddr_t cva = current->va;
         //paddr_t cpa = current->pa;
         
         if(addr == cva){
         current->state = 1; // flag it to free
         // this is to free physical address
         //as_zero_region(cpa, 1);
         }
         }*/
        
    }
    
    
    struct addrspace *
    as_create(void)
    {
        >>>>>>> fb5a0f064c54551b2a6f2628046c68de54373f23
        struct addrspace *as = kmalloc(sizeof(struct addrspace));
        if (as==NULL) {
            return NULL;
        }
        <<<<<<< HEAD
        
        =======
        
        >>>>>>> fb5a0f064c54551b2a6f2628046c68de54373f23
        as->as_vbase1 = 0;
        as->as_pbase1 = 0;
        as->as_npages1 = 0;
        as->as_vbase2 = 0;
        as->as_pbase2 = 0;
        as->as_npages2 = 0;
        as->as_stackpbase = 0;
        <<<<<<< HEAD
        
        =======
        
        >>>>>>> fb5a0f064c54551b2a6f2628046c68de54373f23
        return as;
    }
    
    void
    as_destroy(struct addrspace *as)
    {
        kfree(as);
    }
    
    void
    as_activate(void)
    {
        int i, spl;
        struct addrspace *as;
        
        as = curproc_getas();
#ifdef UW
        <<<<<<< HEAD
        /* Kernel threads don't have an address spaces to activate */
        =======
        /* Kernel threads don't have an address spaces to activate */
        >>>>>>> fb5a0f064c54551b2a6f2628046c68de54373f23
#endif
        if (as == NULL) {
            return;
        }
        <<<<<<< HEAD
        
        /* Disable interrupts on this CPU while frobbing the TLB. */
        spl = splhigh();
        
        for (i=0; i<NUM_TLB; i++) {
            tlb_write(TLBHI_INVALID(i), TLBLO_INVALID(), i);
        }
        
        =======
        
        /* Disable interrupts on this CPU while frobbing the TLB. */
        spl = splhigh();
        
        for (i=0; i<NUM_TLB; i++) {
            tlb_write(TLBHI_INVALID(i), TLBLO_INVALID(), i);
        }
        
        >>>>>>> fb5a0f064c54551b2a6f2628046c68de54373f23
        splx(spl);
    }
    
    void
    as_deactivate(void)
    {
        /* nothing */
    }
    
    int
    as_define_region(struct addrspace *as, vaddr_t vaddr, size_t sz,off_t offset,unit32_t filesize,int readable, int writeable, int executable)
    {
        <<<<<<< HEAD
        size_t npages;
        
        /* Align the region. First, the base... */
        sz += vaddr & ~(vaddr_t)PAGE_FRAME;
        vaddr &= PAGE_FRAME;
        
        /* ...and now the length. */
        sz = (sz + PAGE_SIZE - 1) & PAGE_FRAME;
        
        npages = sz / PAGE_SIZE;
        
        =======
        size_t npages;
        
        /* Align the region. First, the base... */
        sz += vaddr & ~(vaddr_t)PAGE_FRAME;
        vaddr &= PAGE_FRAME;
        
        /* ...and now the length. */
        sz = (sz + PAGE_SIZE - 1) & PAGE_FRAME;
        
        npages = sz / PAGE_SIZE;
        
        >>>>>>> fb5a0f064c54551b2a6f2628046c68de54373f23
        /* We don't use these - all pages are read-write */
        (void)readable;
        (void)writeable;
        (void)executable;
        <<<<<<< HEAD
        
        if (as->as_vbase1 == 0) {
            as->as_vbase1 = vaddr;
            as->as_npages1 = npages;
            as->offset1 = offset;
            as->filesize1 = filesize;
            as->memsize1 = sz;
            return 0;
        }
        
        if (as->as_vbase2 == 0) {
            as->as_vbase2 = vaddr;
            as->as_npages2 = npages;
            as->offset2 = offset;
            as->filesize2 = filesize;
            as->memsize2 = sz;
            
            return 0;
        }
        
        =======
        
        if (as->as_vbase1 == 0) {
            as->as_vbase1 = vaddr;
            as->as_npages1 = npages;
            return 0;
        }
        
        if (as->as_vbase2 == 0) {
            as->as_vbase2 = vaddr;
            as->as_npages2 = npages;
            return 0;
        }
        
        >>>>>>> fb5a0f064c54551b2a6f2628046c68de54373f23
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
    {
        KASSERT(as->as_pbase1 == 0);
        KASSERT(as->as_pbase2 == 0);
        KASSERT(as->as_stackpbase == 0);
        <<<<<<< HEAD
        
        =======
        
        >>>>>>> fb5a0f064c54551b2a6f2628046c68de54373f23
        as->as_pbase1 = getppages(as->as_npages1);
        if (as->as_pbase1 == 0) {
            return ENOMEM;
        }
        <<<<<<< HEAD
        
        =======
        
        >>>>>>> fb5a0f064c54551b2a6f2628046c68de54373f23
        as->as_pbase2 = getppages(as->as_npages2);
        if (as->as_pbase2 == 0) {
            return ENOMEM;
        }
        <<<<<<< HEAD
        
        =======
        
        >>>>>>> fb5a0f064c54551b2a6f2628046c68de54373f23
        as->as_stackpbase = getppages(DUMBVM_STACKPAGES);
        if (as->as_stackpbase == 0) {
            return ENOMEM;
        }
        
        as_zero_region(as->as_pbase1, as->as_npages1);
        as_zero_region(as->as_pbase2, as->as_npages2);
        as_zero_region(as->as_stackpbase, DUMBVM_STACKPAGES);
        <<<<<<< HEAD
        
        =======
        
        >>>>>>> fb5a0f064c54551b2a6f2628046c68de54373f23
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
        <<<<<<< HEAD
        
        *stackptr = USERSTACK;
        =======
        
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
        
        /* (Mis)use as_prepare_load to allocate some physical memory. */
        if (as_prepare_load(new)) {
            as_destroy(new);
            return ENOMEM;
        }
        
        KASSERT(new->as_pbase1 != 0);
        KASSERT(new->as_pbase2 != 0);
        KASSERT(new->as_stackpbase != 0);
        
        memmove((void *)PADDR_TO_KVADDR(new->as_pbase1),
                (const void *)PADDR_TO_KVADDR(old->as_pbase1),
                old->as_npages1*PAGE_SIZE);
        
        memmove((void *)PADDR_TO_KVADDR(new->as_pbase2),
                (const void *)PADDR_TO_KVADDR(old->as_pbase2),
                old->as_npages2*PAGE_SIZE);
        
        memmove((void *)PADDR_TO_KVADDR(new->as_stackpbase),
                (const void *)PADDR_TO_KVADDR(old->as_stackpbase),
                DUMBVM_STACKPAGES*PAGE_SIZE);
        
        *ret = new;
        >>>>>>> fb5a0f064c54551b2a6f2628046c68de54373f23
        return 0;
    }
#endif
    
    
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
        
        /* (Mis)use as_prepare_load to allocate some physical memory. */
        if (as_prepare_load(new)) {
            as_destroy(new);
            return ENOMEM;
        }
        
        KASSERT(new->as_pbase1 != 0);
        KASSERT(new->as_pbase2 != 0);
        KASSERT(new->as_stackpbase != 0);
        
        memmove((void *)PADDR_TO_KVADDR(new->as_pbase1),
                (const void *)PADDR_TO_KVADDR(old->as_pbase1),
                old->as_npages1*PAGE_SIZE);
        
        memmove((void *)PADDR_TO_KVADDR(new->as_pbase2),
                (const void *)PADDR_TO_KVADDR(old->as_pbase2),
                old->as_npages2*PAGE_SIZE);
        
        memmove((void *)PADDR_TO_KVADDR(new->as_stackpbase),
                (const void *)PADDR_TO_KVADDR(old->as_stackpbase),
                DUMBVM_STACKPAGES*PAGE_SIZE);
        
        *ret = new;
        return 0;
    }
