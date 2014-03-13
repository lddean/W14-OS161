#ifndef FD_TABLE_H
#define FD_TABLE_H

#include "opt-A2.h"
#include <file_des.h>

// A fd table stores an array (array structure defined by kernel)
// of file descriptors associated with a thread.
struct fd_table{
        struct array *fds;                        // An array of file descriptors
        int num_fd;
};

/*
 * fd table operations
 *
 * fd_table_create  - Create a fd table for a thread
 * fd_table_init    - Initialize the fd table by adding std in, std out and std err
                                       file descriptors to it
 * fd_table_destroy - Destroy a fd table including the fds in its fd array
 * fd_table_dup         - Duplicates a fd table, the vnode's refcounter of each fd increases by 1
 * fd_table_add_fd  - Add a fd to fd table and return its index as the fd's ID
                                           Return -1 when try to open too many files
 * fd_table_get_fd  - Get the fd in a fd table given the fd's integer ID
 * fd_table_rm_fd   - Remove a fd from the fd table, return 0 upon success and 1 upon failure
 * fd_table_fd_nums - Return the number of fds in the fd table
 */

struct fd_table *fd_table_create(void);
int fd_table_init(struct fd_table *fdt);
void fd_table_destroy(struct fd_table *fdt);
struct fd_table *fd_table_dup(struct fd_table *src_fdt);

int fd_table_add_fd(struct fd_table *fdt, struct file_des *fd);
struct file_des *fd_table_get_fd(struct fd_table *fdt, int fd);
int fd_table_rm_fd(struct fd_table *fdt, int fd);
int fd_table_fd_nums(struct fd_table *fdt);

#endif
