#ifndef _PID_H_
#define _PID_H_

#include "opt-A2.h"

#if OPT_A2
#include <fd_table.h>
#include <array.h>
#include <spinlock.h>
#include <threadlist.h>
#include <thread.h>
#include <proc.h>
#include <limits.h>

struct processInfo{
	pid_t self;			//the pid corresponding to this processInfo
	int exited; 		//if the process have exited: active = 0, exited = 1, error = -1
	pid_t parent;		//the pid of its parent
	pid_t *children; 	//the array that contains its children’s pid
	int num_children;	//the number of children it has
	int exitcode;		//the exitcode of this process
	struct lock *plock;	//used with pcv
	struct cv *pcv;		//the parent will be sleeping on this cv if the process have not
						//finished yet. This process will wakeup its parent when it exits.
};


void create_proc_info_array(void);

struct processInfo * procinfo_create(void);

void procinfo_destroy(struct processInfo *pinfo);

pid_t get_next_pid(void);

pid_t add_proc_info(void);

struct processInfo *get_proc_details(pid_t pid);

#endif

#endif /* _PID_H_ */
