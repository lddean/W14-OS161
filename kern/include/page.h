
#include <types.h>
#include <lib.h>
#include <mips/tlb.h>
#include <addrspace.h>
#include <vm.h>


struct pageInfo{
    vaddr_t vaddr;
    paddr_t paddr;
    int segment;
    int dirty;
    int valid;
};
struct pageInfo ** create_pagetable(void);

bool page_exist(vaddr_t V, struct pageInfo ** page_table);

struct pageInfo * pageInfo_create(void);

paddr_t get_paddr(vaddr_t vaddr, struct pageInfo ** page_table);

vaddr_t get_vaddr(paddr_t paddr, struct pageInfo ** page_table);

void add_vaddr_to_table(vaddr_t V, paddr_t P, struct pageInfo ** page_table);


