#ifndef _SWAPFILE_H
#define _SWAPFILE_H

#include "opt-A3.h"
#include <types.h>
#include <lib.h>
#include <array.h>
#include <vnode.h>
#include <vfs.h>
#include <limits.h>
#include <kern/fcntl.h>
#include <kern/limits.h>
#include <file_descriptor.h>
#include <synch.h>
#include <addrspace.h>


#define SWAP_SIZE 9 * 1024 * 1024
struct vnode *swap_file;
// duplicate build-int one for now 
//struct array* swap_record; 
struct lock *swap_lock; // lock for read/write


//static bool swap_boot = false;
struct swap{
	paddr_t pa;
	struct addrspace* as;
};

int swap_init(void);
int check_offset(paddr_t pa, struct addrspace* as);
//void update_record(struct swap* insertion);
void update_record(paddr_t pa, struct addrspace* as);
int read_page(paddr_t pa, int offset);

// this is previous code, but page table does not have as
//void swap_in(paddr_t pa, struct addrspace* as);
void swap_in(paddr_t pa);
int write_page(paddr_t pa, int offset);
void swap_out(paddr_t pa, struct addrspace* as);
#endif
