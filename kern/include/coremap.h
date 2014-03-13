#ifndef _COREMAP_H_
#define _COREMAP_H_


#include "opt-A3.h"

#include <types.h>
#include <addrspace.h>
#include <vm.h>
#include <spinlock.h>

enum p_state{
    FREE = 0,
    FIXED,
    DIRTY
};

struct page {
    paddr_t paddr;
    enum p_state state;
    int length;
    int order;
};

void cm_bootstrap(void);

paddr_t cm_getpages(unsigned long pages);

int cm_freepages(vaddr_t vaddr);


#endif /* _COREMAP_H_ */
