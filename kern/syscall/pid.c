#include "opt-A2.h"

#if OPT_A2
#include <types.h>
#include <kern/errno.h>
#include <lib.h>
#include <pid.h>
#include <synch.h>
#include <limits.h>
#include <proc.h>
#include <current.h>
#include <fd_table.h>
#include "opt-synchprobs.h"

//pid starts from 2 although processinfo array index starts from 0.
static int max_index = 128;
static struct processInfo **proc_info_array = NULL;

void create_proc_info_array(void){
	proc_info_array = kmalloc(sizeof (struct processInfo *) * max_index);
}

struct processInfo * procinfo_create(void){
	struct processInfo *pinfo = kmalloc(sizeof (struct processInfo));

	if (pinfo == NULL){
		return NULL;
	}

	pinfo->self = -1;
	pinfo->parent = -1;
	pinfo->num_children = 0;
	pinfo->children = NULL;
	pinfo->exited = 0;
	pinfo->exitcode = -1;
	pinfo->plock = lock_create("plock");
	pinfo->pcv = cv_create("pcv");

	return pinfo;
}

void procinfo_destroy(struct processInfo *pinfo){
	lock_destroy(pinfo->plock);
	cv_destroy(pinfo->pcv);
	kfree(pinfo->children);
	kfree(pinfo);
}

pid_t get_next_pid(void){
	for(int i = 0; i <= max_index; i++){
		if (i == max_index){
			if (i == PID_MAX){
				return -1;
			}
			max_index = max_index * 2;
			if (max_index > PID_MAX){
				max_index = PID_MAX;
			}
			struct processInfo ** temp = kmalloc(sizeof (struct processInfo *) * max_index);
			for (int j = 0; j < i; j++){
				temp[j] = proc_info_array[j];
			}
			kfree(proc_info_array);
			proc_info_array = temp;
			return i + PID_MIN;
		}
		if (proc_info_array[i] == NULL){
			return i + PID_MIN;
		}
	}
	return -1;
}

pid_t add_proc_info(void){
	bool first = false;
	if (proc_info_array == NULL){
		create_proc_info_array();
		first = true;
	}
	pid_t npid = get_next_pid();
	struct processInfo * npinfo = procinfo_create();
	npinfo->self = npid;
	if (!first){
		npinfo->parent = sys_getpid();
	}
	proc_info_array[npid - PID_MIN] = npinfo;
	return npid;
}

struct processInfo *get_proc_details(pid_t pid){
	return proc_info_array[pid - PID_MIN];
}


#endif
