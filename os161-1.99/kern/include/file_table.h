#if OPT_A2
#ifndef FILE_TABLE_H
#define FILE_TABLE_H

#include "opt-A2.h"
#include <types.h>
#include <lib.h>
#include <array.h>
#include <vnode.h>
#include <vfs.h>
#include <limits.h>
#include <kern/fcntl.h>
#include <kern/limits.h>
#include <file_descriptor.h>
#include <synch.h>

struct file_table{
	struct array* fds; // files descriptors
	int size; // size of array
};

/* 
 *	file_table_crete - create a file table
 *	file_table_destroy - destroy a file table
 *	file_table_add - add a file descriptor to file table
 *	file_table_remove - remove a file descriptor from file table
 *	file_table_get - get a file descriptor from file table
 *	file_table_duplicate - get a duplicate file table 
 *	file_table_initialize - add std in, std out, std err
*/

struct file_table* file_table_create(void);
void file_table_destroy(struct file_table* ft);

int file_table_add(struct file_table* ft, struct file_descriptor* fd);
int file_table_remove(struct file_table* ft, int index);
struct file_descriptor* file_table_get(struct file_table* ft, int index);
struct file_table* file_table_duplicate(struct file_table* src_ft);

void file_table_initialize(struct file_table* ft);
#endif
#endif
