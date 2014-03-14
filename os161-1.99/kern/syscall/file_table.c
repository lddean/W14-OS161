#include "opt-A2.h"
#include <file_table.h>

// create a file table
struct file_table* file_table_create(void){
	struct file_table* ft;
	ft = kmalloc(sizeof(struct file_table));
	
	//if kmalloc fails
	if(ft == NULL){return NULL;}
	
	ft->fds = array_create();
	ft->size = 0;
	return ft;
}

// destroy a file table
void file_table_destroy(struct file_table* ft){

	struct array* fd = ft->fds;

	 //KASSERT(fd != NULL); // not null to free
	int size = array_num(fd);

	for(int i=0; i<size; i++){
		kfree(array_get(fd, i));
	}

	//kfree(fd);
	// destroy array and the pointer
	//array_destroy(fd);

	ft->size  = 0;
	kfree(ft);
}

// add  file descriptor to a file table
int file_table_add(struct file_table* ft, struct file_descriptor* fd){
	//kprintf("%p %p", ft, fd);

	// hit the max open files, cannot add new open file
	if(ft->size+1 >= OPEN_MAX){
		return -1;
	}

	// iterate to see whether there is a NULL to replace
	struct array* fds = ft->fds;
	for(int i=0; i<ft->size; i++){
		if(array_get(fds, i) == NULL){
			array_set(fds,i,fd);
			return i;
		}
	}
	unsigned index;
	array_add(ft->fds, fd, &index);
	ft->size++;
	return ft->size-1;
}


// remove file descriptor from a file table
int file_table_remove(struct file_table* ft, int index){
	//unsigned size = array_num(ft->fds);

	struct file_descriptor* rfd = array_get(ft->fds, index);

	if(rfd == NULL){ // the file has been removed(set to NULL)
		return -1;
	}	
	 KASSERT(rfd != NULL);

	struct vnode* rvnode = rfd->vnode;
	 KASSERT(rvnode != NULL);

	// make sure they are not the same
	//kprintf("index is %d +pointer address %p\n", index, rfd);
	vfs_close(rvnode); // close the node before exit

	// destroy the file descriptor and update member
	file_dst_destroy(rfd); // do not destroy file table
	//array_remove(ft->fds, index); //do not remove, but make it null to keep the size
        array_set(ft->fds, index, NULL);
	
	struct file_descriptor* test = array_get(ft->fds, index);
	KASSERT(test == NULL);
	
	//ft->size--;
	return 0;
}

// get a file descriptor from a file table
struct file_descriptor* file_table_get(struct file_table* ft, int index){
	return array_get(ft->fds, index);
}

// get a duplicate file table
struct file_table* file_table_duplicate(struct file_table* src_ft){

	struct file_table* dst_ft = file_table_create();

	// if create give NULL
	if(dst_ft == NULL){return NULL;}

	struct array* src_fds = src_ft->fds;
	struct array* dst_fds = dst_ft->fds;

	int size = src_ft->size;

	for(int i=0; i<size; i++){
		struct file_descriptor* dst_fd = kmalloc(sizeof(struct file_descriptor));
		struct file_descriptor* src_fd = array_get(src_fds, i);
		// copy all fields and increase vnode count
		dst_fd->vnode = src_fd->vnode;
		dst_fd->flag = src_fd->flag;
		dst_fd->offset = src_fd->offset;
		vnode_incref(dst_fd->vnode);
		
		unsigned result;
		array_add(dst_fds, dst_fd, &result);
	}
	return dst_ft;
}

// add std in, std out, std error to file table
void file_table_initialize(struct file_table* ft){
	unsigned result;

	// std in
	char* console1 = kstrdup("con:");
	struct vnode* v1;
	vfs_open(console1, O_RDONLY, 0, &v1);

	// create file descriptor and add to file table
	struct file_descriptor* fd1 = file_dst_create(v1, O_RDONLY, 0);
	kfree(console1);
	array_add(ft->fds, fd1, &result);
	ft->size++;

	// std out
	char* console2 = kstrdup("con:");
	struct vnode* v2;
	vfs_open(console2, O_WRONLY, 0, &v2);

	// create file descriptor and add to file table
	struct file_descriptor* fd2 = file_dst_create(v2, O_WRONLY, 0);
	kfree(console2);
	array_add(ft->fds, fd2, &result);
	ft->size++;

	// std out
	char* console3 = kstrdup("con:");
	struct vnode* v3;
	vfs_open(console3, O_WRONLY, 0, &v3);

	// create file descriptor and add to file table
	struct file_descriptor* fd3 = file_dst_create(v3, O_WRONLY, 0);
	kfree(console3);
	array_add(ft->fds, fd3, &result);
	ft->size++;
}
