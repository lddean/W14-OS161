#ifndef FILE_DESCRIPTOR_H
#define FILE_DESCRIPTOR_H

#include "opt-A2.h"
#include <types.h>
#include <lib.h>
#include <vnode.h>

struct file_descriptor{
	struct vnode* vnode; // vnode pointer
	int flag; // permit flag for O_RDONLY, O_WRONLY
	off_t offset; // file offset
};

// create a file descriptor
struct file_descriptor* file_dst_create(struct vnode* vn, int fl, off_t off);

// destroy(free memory) a file descriptor
void file_dst_destroy(struct file_descriptor* dst);

#endif
