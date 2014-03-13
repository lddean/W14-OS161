#include "opt-A2.h"
#include <file_des.h>

// File descriptor Operations

// Create a new file descriptor
struct file_des *fd_create(struct vnode *v, int flg, off_t os){
        struct file_des *fd;
        fd = kmalloc(sizeof(struct file_des));

    // If malloc failed
        if (fd == NULL){
        kfree(fd);
                return NULL;
        }
    else {
        fd->vnode = v;
        fd->flag = flg;
        fd->offset = os;
        return fd;
    }
}

// Destroy a file descriptor
void fd_destroy(struct file_des *fd){
    if (fd != NULL){
        kfree(fd->vnode);
    }
    kfree(fd);
}
