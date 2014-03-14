#include "opt-A2.h"
#include <file_descriptor.h>

// create a file descriptor
struct file_descriptor* file_dst_create(struct vnode* vn, int fl, off_t off){
	struct file_descriptor* fd;
	fd = kmalloc(sizeof(struct file_descriptor));
	
	//if kmalloc fails
	if(fd == NULL){return NULL;}
	
	fd->vnode = vn;
	fd->flag = fl;
	fd->offset = off;
	return fd;
}

// destroy(free memory) a file descriptor
void file_dst_destroy(struct file_descriptor* dst){
	/*
	if(dst != NULL){ // free the vnode pointer
		kfree(dst->vnode);
	}
	*/
	kfree(dst);
}
