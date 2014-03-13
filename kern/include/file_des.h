#ifndef FILE_DES_H
#define FILE_DES_H

#include "opt-A2.h"
#include <types.h>
#include <vnode.h>
#include <array.h>
#include <lib.h>
#include <kern/fcntl.h>

// A file descriptor is created upon open a file.
// It indicates: the vnode pointer "vnode" is opened with mode "flag"
// with current offset "offset" in file.
struct file_des
{
        struct vnode *vnode;                // vnode pointer
        int flag;                                        // permission flag such as O_RDONLY, O_WRONLY, etc
        off_t offset;                                // offset of file
        // int opencount;
};

// Create a new file descriptor
struct file_des *fd_create(struct vnode *v, int flg, off_t os);

// Destroy a file descriptor
void fd_destroy(struct file_des *fd);

#endif
