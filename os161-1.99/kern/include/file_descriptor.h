#if OPT_A2
#ifndef FILE_DESCRIPTOR_H
#define FILE_DESCRIPTOR_H

#include "opt-A2.h"
#include <types.h>
#include <lib.h>
#include <vnode.h>
#include <synch.h>

struct file_descriptor{
	struct vnode* vnode; // vnode pointer
	int flag; // permit flag for O_RDONLY, O_WRONLY
	off_t offset; // file offset

        struct lock *wlock; //write lock
        struct lock *rlock; //read lock
};

/*
 * file_dst_create: create a file descriptor
 * file_dst_destroy: destroy(free memory) a file descriptor
*/

struct file_descriptor* file_dst_create(struct vnode* vn, int fl, off_t off);
void file_dst_destroy(struct file_descriptor* dst);
#endif
#endif
