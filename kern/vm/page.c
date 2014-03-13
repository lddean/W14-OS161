
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
#include <page.h>
#include <array.h>

static int max_index = 128;
	
struct pageInfo ** create_pagetable(void){
    struct pageInfo **page_table = NULL;
    page_table = kmalloc(sizeof (struct pageInfo *) * max_index);
    return page_table;
}

struct pageInfo * pageInfo_create(void){
    
    struct pageInfo * one_page = kmalloc(sizeof (struct pageInfo));
    
    if (one_page == NULL){
        return NULL;
    }
    
    one_page -> vaddr = 0;
    one_page -> paddr = 0;
    one_page -> segment = 4; // it is not belong to any segment 
    one_page -> dirty = 0; // not dirty;
    one_page -> valid = 0; // not valid
    
    return one_page;
    
}

bool page_exist(vaddr_t V, struct pageInfo ** page_table){
    
    for (int i = 0; i < max_index; i++){
        kprintf("vaddr = %d,  V= %d\n", page_table[i] -> vaddr,V);
        if (page_table[i] -> vaddr == V){
            return true;
        }else{
            continue;
        }
    }
    kprintf("fu\n");
    return false;
}

void add_vaddr_to_table(vaddr_t V, paddr_t P, struct pageInfo ** page_table){

    if (page_exist(V,page_table)){
        return;
    }

    int curindex = 0;
    
    for (int i = 0; i<=max_index; i++){
        if (i == max_index){
            max_index = max_index * 2;
            struct pageInfo ** temp = kmalloc(sizeof (struct pageInfo *) * max_index);
            for (int j = 0; j < i ; j ++){
                temp[j] = page_table[j];
            }
            kfree(page_table);
            page_table = temp;
            curindex = max_index;
        }
        if(page_table[i] == NULL){
            curindex = i;
        }
    }
    
    struct pageInfo * one_page = pageInfo_create();
    one_page -> vaddr = V;
    one_page -> paddr = P;
    
    page_table[curindex] = one_page;
    
}

paddr_t get_paddr(vaddr_t vaddr, struct pageInfo ** page_table){
    for (int i = 0; i < max_index; i ++){
        if (vaddr == page_table[i] -> vaddr){
            return page_table[i] -> paddr;
        }
    }
    
    return -1;
}

vaddr_t get_vaddr(paddr_t paddr , struct pageInfo ** page_table){
    
    for (int i = 0; i < max_index; i ++){
        if (paddr == page_table[i] -> paddr){
            return page_table[i] -> vaddr;
        }
    }
    
    return -1;
}

