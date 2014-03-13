#include "opt-A2.h"
#include <file_syscalls.h>
#include <vfs.h>
#include <proc.h>
#include <kern/fcntl.h>
#include <copyinout.h>

struct fd_table *curfdt;
int DEADBEEF = 0xdeadbeef;

// Open the file
int sys_open(const char *filename, int flags, int mode, int *return_val){
    struct vnode *v;
    int result;
    struct file_des *fd;
    // char *path = kstrdup(filename);

    if (filename == NULL) {
        if (flags != O_CREAT) {
            *return_val = ENODEV;
            return -1;
        }
    }
    // result = vfs_lookup(path,&v);
    // if (result){
    //     *return_val = result;
    //     return -1;
    // }
    if (flags == O_APPEND) {
        // Block user from open a file in O_APPEND mode
        *return_val = EAGAIN;
        return -1;
    }
    // Check for invalid user address space
    int * temp_pt = kmalloc(sizeof(*filename));
    result = copyin((userptr_t)filename, temp_pt, sizeof(*filename));
    kfree(temp_pt);
    if (result == EFAULT){
        *return_val=EFAULT;
        return -1;
    }

    // Open the file with name filename
    result = vfs_open((char*)filename, flags, mode, &v);

    // If failed, return errno
    if (result) {
        *return_val = result;
        return -1;
    }

    fd = fd_create(v, flags, 0);

    // Failure on creating fd
    if (fd == NULL){
        return -1;
    }

    curfdt = curthread->fdt;
    int new_fd = fd_table_add_fd(curfdt, fd);

    // Open too many files.
    if (new_fd == -1){
        *return_val = EMFILE;
        return -1;
    }

    // Store return value
    *return_val = new_fd;

    return 0;
}


// Read from the file
int
sys_read(int fd, void *buf, size_t buflen, int *return_val){

    // If at address deadbeef or fd out of range
    if (fd == DEADBEEF || fd < 0 || fd >= (int)array_num(curthread->fdt->fds)){
        *return_val = EBADF;
        return -1;
    }

    struct file_des *file_d;
    file_d = array_get(curthread->fdt->fds, fd);

    // If fild_d is NULL or is write only
    if ((file_d == NULL) || (file_d->flag == 1)){
        *return_val = EBADF;
        return -1;
        // DEBUG file is not opened or is write only
    }
    else {
        struct iovec iov;
        struct uio u;
        int result;

        // Keeping track of blocks of data for I/O
        iov.iov_ubase = buf;
        iov.iov_len = buflen;

        // Data region to store data read by VOP_READ below
        u.uio_iov = &iov;
        u.uio_iovcnt = 1;
        u.uio_offset = file_d->offset;
        u.uio_resid = buflen;
        u.uio_segflg = UIO_USERSPACE;
        u.uio_rw = UIO_READ;
        u.uio_space = curproc_getas();

        // Read from fd
        result = VOP_READ(file_d->vnode, &u);

        // If failed, return errno through return_val
        if (result){
            *return_val = result;
            return -1;
        }

        // The fd's offset now includes the amount of data successfully read
        file_d->offset = u.uio_offset;

        // If finish read, return value (number of bytes read) will equal to the buflen
        if (u.uio_resid == 0){
            *return_val = buflen;
        }
        // If stopped during reading, return value will equal to buflen subtract the
        // bytes left to read
        else {
            *return_val = buflen-u.uio_resid;
        }
    }
    return 0;
}

// Write to the file
int
sys_write(int fd, const void *buf, size_t nbytes, int *return_val){

    // If at address deadbeef or fd out of range
    if (fd == DEADBEEF || fd < 0 || fd >= (int)array_num(curthread->fdt->fds)){
        *return_val = EBADF;
        return -1;
    }

    curfdt = curthread->fdt;
    struct file_des *file_d;
    file_d = array_get(curfdt->fds,fd);

    // If fild_d is NULL or is read only
    if ((file_d == NULL) || (file_d->flag == 0)){
            *return_val = EBADF;
            return -1;
    }
    else {
        struct iovec iov;
        struct uio u;
        int result;

        // Keeping track of blocks of data for I/O
        iov.iov_ubase = (void*)buf;
        iov.iov_len = nbytes;

        // Data region to store data to be write to fd by VOP_WRITE below
        u.uio_iov = &iov;
        u.uio_iovcnt = 1;
        u.uio_offset = file_d->offset;
        u.uio_resid = nbytes;
        u.uio_segflg = UIO_USERSPACE;
        u.uio_rw = UIO_WRITE;
        u.uio_space = curproc_getas();

        // Write to fd
        result = VOP_WRITE(file_d->vnode, &u);

        // If failed, return errno through return_val
        if (result){
            *return_val = result;
            return -1;
        }

        // The fd's offset now includes the amount of data successfully write
        file_d->offset = u.uio_offset;

        // If finish write, return value (number of bytes wrote) will equal to the buflen
        if (u.uio_resid == 0){
            *return_val = nbytes;
        }
        // If stopped during writing, return value will equal to buflen subtract the
        // bytes left to write
        else {
            *return_val = nbytes-u.uio_resid;
        }
    }
    return 0;
}

// Close the file
int
sys_close(int fd, int *return_val){

    // If at address deadbeef or fd out of range
    if (fd == DEADBEEF || fd < 0 || fd >= (int)array_num(curthread->fdt->fds)){
        *return_val = EBADF;
        return -1;
    }

    int result;
    curfdt = curthread->fdt;
    if (fd > fd_table_fd_nums(curfdt)){
        *return_val = EBADF;
        return -1;
        // Fd number exceed maximum file nums
    }
    result = fd_table_rm_fd(curfdt,fd);

    // If failed, return errno through return_val
    if (result){
        // Change result to something else (DEBUG) since result is 1!!!
        *return_val = result;
        return -1;
    }

    return 0;
}
