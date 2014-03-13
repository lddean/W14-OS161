#ifndef _FILE_SYSCALLS_H_
#define _FILE_SYSCALLS_H_
#include "opt-A2.h"
#include <fd_table.h>
#include <uio.h>
#include <current.h>
#include <kern/errno.h>

/*
 * System Calls
 *
 * sys_open  - Opens the file, device, or other kernel object named by the pathname filename.
                            The flags argument specifies how to open the file.
                            Return_val stores the function's return value.
 * sys_read  - Reads up to buflen bytes from the file with file descriptor ID fd, at the location
                            in the file specified by the current seek position (number of bytes read) of the
                            file, and stores them in the space pointed to by buf.
                            The file must be open for reading.
 * sys_write - Writes up to buflen bytes to the file with file descriptor ID fd, at the location
                            in the file specified by the current seek position (number of bytes written) of the
                            file, taking the data from the space pointed to by buf.
                            The file must be open for writing.
 * sys_close - Close the file with file descriptor ID fd.
 * sys__exit - Cause the current process to exit, return void.
 */

int sys_open(const char *filename, int flags, int mode, int *return_val);
int sys_read(int fd, void *buf, size_t buflen, int *return_val);
int sys_write(int fd, const void *buf, size_t nbytes, int *return_val);
int sys_close(int fd, int *return_val);
void sys__exit(int exitcode);

#endif
