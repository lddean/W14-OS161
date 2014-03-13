#include "opt-A2.h"
#include <fd_table.h>
#include <limits.h>
#include <current.h>
#include <vfs.h>
#include <array.h>

// Create a fd table
struct fd_table *fd_table_create(void){

        struct fd_table *fdt;
        fdt = kmalloc(sizeof(struct fd_table));

        // If malloc failed
        if (fdt == NULL) {
                return NULL;
        }

        fdt->fds = array_create();
        fdt->num_fd = 0;
        return fdt;
}

// Initialize a fd table
int fd_table_init(struct fd_table *fdt) {
        int result;
        unsigned index;

        // STD_IN
        char *console1 = NULL;
        console1 = kstrdup("con:");
        struct vnode *v1;
        result = vfs_open((char*)console1, O_RDONLY, 0, &v1);
        struct file_des *fd1 = fd_create(v1,O_RDONLY,0);

        kfree(console1);
        array_add(fdt->fds, fd1, &index);
        fdt->num_fd ++;

        // STD_OUT
        char *console2 = NULL;
        console2 = kstrdup("con:");
        struct vnode *v2;
        result = vfs_open((char*)console2, O_WRONLY, 0, &v2);
        struct file_des *fd2 = fd_create(v2,O_WRONLY,0);

        kfree(console2);
        array_add(fdt->fds, fd2, &index);
        fdt->num_fd ++;

        // STD_ERR
        char *console3= NULL;
        console3 = kstrdup("con:");
        struct vnode *v3;
        result = vfs_open((char*)console3, O_WRONLY, 0, &v3);
        struct file_des *fd3 = fd_create(v3,O_WRONLY,0);

        kfree(console3);
        array_add(fdt->fds, fd3, &index);
        fdt->num_fd ++;

        return 0;
}

// Destroy a fd table
void fd_table_destroy(struct fd_table *fdt){
        // Free all fds in fd table
        int dec =0;
        for (int i=0; i<(int)array_num(fdt->fds); i++){
                struct file_des *fd = array_get(fdt->fds,i);
                kfree(fd);
                dec = i+1;
        }
        (fdt->fds)->num -= dec;
        array_destroy(fdt->fds);

        // Free fd table
        kfree(fdt);
}

// Duplicate a fd table
struct fd_table *fd_table_dup(struct fd_table *src_fdt){
        struct fd_table *dst_fdt = fd_table_create();

        // If malloc failed
        if (dst_fdt == NULL) {
                return NULL;
        }

        dst_fdt->num_fd = src_fdt->num_fd;

        int len = array_num(src_fdt->fds);
        for (int i=0; i<len; i++){
                struct file_des *dst_fd = kmalloc(sizeof(struct file_des));
                struct file_des *src_fd = array_get(src_fdt->fds,i);

                dst_fd->vnode = src_fd->vnode;
                dst_fd->flag = src_fd->flag;
                dst_fd->offset= src_fd->offset;
                vnode_incref(dst_fd->vnode); // Increase each vnode's refcounter by 1

                unsigned result_index;
                array_add(dst_fdt->fds, dst_fd, &result_index);
        }
        return dst_fdt;
}

// Add a fd to fd table
int fd_table_add_fd(struct fd_table *fdt, struct file_des *fd){
        unsigned *index_ret = NULL;
        unsigned fd_index;
        fd_index = array_num(fdt->fds);

        // When open too many files
        if (fdt->num_fd >= OPEN_MAX-1){
                return -1;
        }
        array_add(fdt->fds, fd, index_ret);
        fdt->num_fd ++;
        return (int)fd_index;
}

// Get a fd from fd table
struct file_des *fd_table_get_fd(struct fd_table *fdt, int fd){
        return array_get(fdt->fds, fd);
}

// Remove a fd from fd table
int fd_table_rm_fd(struct fd_table *fdt, int fd){
        struct file_des *rm_fd = array_get(fdt->fds,fd);
        if ((array_num(fdt->fds)!=0) && (rm_fd!=NULL)){

                // Remove fd by close its vnode and set it to NULL
                struct vnode *close_v = (fd_table_get_fd(fdt,fd))->vnode;
                vfs_close(close_v);
                array_set(fdt->fds, fd, NULL);
                fdt->num_fd --;
                return 0;
        }
        return 1;
}

// Return the number of fds in fd table
int fd_table_fd_nums(struct fd_table *fdt){
        if (fdt == NULL){
                return 0;
        }
        return fdt->fds->num;
}
