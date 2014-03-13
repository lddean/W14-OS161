#include "opt-A2.h"

#if OPT_A2
#include <types.h>
#include <kern/errno.h>
#include <lib.h>
#include <pid.h>
#include <proc_syscalls.h>
#include <copyinout.h>
#include <array.h>
#include <thread.h>
#include <proc.h>
#include <current.h>
#include <synch.h>
#include <vnode.h>
#include <fd_table.h>
#include "opt-synchprobs.h"

struct cv *wpcv = NULL;
int loop =0;

pid_t sys_getpid(void){
	loop ++;
	return curthread->pid;
}

int sys_waitpid(pid_t pid, int *status, int *options, int *ret){
	if (options != 0){
		*ret = EINVAL; //invalid option
		return -1;
	}

	// int dst;
	if (status == NULL ||
		// copyin((const_userptr_t)status, &dst, 1) ||
		(uintptr_t)status % 4 != 0){
		*ret = EFAULT;	//invalid status
		return -1;
	}

	struct processInfo *pinfo = get_proc_details(pid);

	if (pinfo == NULL){
		*ret = ESRCH; //no such process
		return -1;
	}

	if (pinfo->parent != sys_getpid()){
		*ret = ECHILD; //not child process
		return -1;
	}

	lock_acquire(pinfo->plock);
	while (pinfo->exited == 0){ //still active
		cv_wait(pinfo->pcv, pinfo->plock);
	}

	if (pinfo->exited == 1){ //exited
		*status = pinfo->exitcode;
		lock_release(pinfo->plock);
	}
	else{
		*ret = ESRCH; //state == invalid
		lock_release(pinfo->plock);
		return -1;
	}

	*ret = pid;
	return 0;
}

void sys_exit (int exitcode){
	struct processInfo *pinfo = get_proc_details(sys_getpid());
	if (pinfo != NULL){
		pinfo->exited = 1;
		pinfo->exitcode = exitcode;
		lock_acquire(pinfo->plock);
		cv_signal(pinfo->pcv, pinfo->plock);
		lock_release(pinfo->plock);
		for (int i = 0; i < pinfo->num_children; i++){
			procinfo_destroy(get_proc_details(pinfo->children[i]));
		}
	}
	thread_exit();
}
#endif
