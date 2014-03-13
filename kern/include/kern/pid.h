#ifndef _PID_H_
#define _PID_H_

#include "opt-A2.h"

#if OPT_A2
#include <fd_table.h>
#include <array.h>
#include <spinlock.h>
#include <threadlist.h>
#include <thread.h>0
#include <proc.h>
#include <limits.h>

//pid starts from 2 although processinfo array index starts from 0.
int max_index = 128;
struct processInfo **proc_info_array = NULL;

struct processInfo{
	pid_t self;
	int exited; // 0 = active, 1 = exited, -1 = error
				// if exited, remove all children proc details.
	pid_t parent;	//maybe use *processinfo?
	pid_t *children; //possibily not necessary?
	int num_children;
	int *exitcode;
	struct lock *plock;
	struct cv *pcv;
};

struct processInfo *create_proc_info_array(void);

struct processInfo * procinfo_create(void);

void procinfo_destroy(struct processInfo *pinfo);

pid_t get_next_pid(void);

pid_t add_proc_info(void);

struct processinfo *get_proc_details(pid_t pid);

#endif

#endif /* _PID_H_ */
