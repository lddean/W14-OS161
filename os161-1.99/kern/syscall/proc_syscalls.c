#include "opt-A2.h"
#if OPT_A2

#include <types.h>
#include <kern/errno.h>
#include <kern/unistd.h>
#include <lib.h>
#include <syscall.h>
#include <proc.h>
#include <pid.h>
#include <thread.h>
#include <current.h>
#include <addrspace.h>
#include <limits.h>
#include <proc_syscall.h>

pid_t sys_getpid(void){
	return curthread->pid;
}
void sys_exit(int exitcode){
	struct procInfo* pInfo = procInfo_get(sys_getpid());
	
	if (pInfo!=NULL || pInfo->active!=0){
		pInfo->active = 0;
		pInfo->exitcode = exitcode;
		lock_acquire(pInfo->plock);
		cv_signal(pInfo->pcv, pInfo->plock);
		lock_release(pInfo->plock);
	}
	thread_exit();
}
int sys_waitpid(pid_t pid, int* status, int options, int *retval){
	//Error checking
	//invalid option
	if (options!=0){
		*retval = EINVAL;
		return -1;
	}	

	struct procInfo* pInfo = procInfo_get(pid); //update pid's exitcode into status

	//non-existent process -> out of pt bound or inactive
	if (pInfo==NULL || pInfo->active==0){
		*retval = ESRCH;
		return -1;
	}	
	//not child process -> current process must be parent process
	if (pInfo->parentPid != sys_getpid()){
		*retval = ECHILD;
		return -1;
	}
	//error with status ptr 
	if (status==NULL || 
		(int)status%4!=0) {
		*retval = EFAULT;
		return -1;
	}

	lock_acquire(pInfo->plock);
	while (pInfo->active){
		cv_wait(pInfo->pcv,pInfo->plock);
	}
	if (pInfo->active==0){
		*status = pInfo->exitcode;
		lock_release(pInfo->plock);
	}	 	
	*retval = pid;
	return 0;
	 

}
void sys__exit(int exitcode) {

  struct addrspace *as;
  struct proc *p = curproc;
  /* for now, just include this to keep the compiler from complaining about
     an unused variable */
  (void)exitcode;

  DEBUG(DB_SYSCALL,"Syscall: _exit(%d)\n",exitcode);

  KASSERT(curproc->p_addrspace != NULL);
  as_deactivate();
  /*
   * clear p_addrspace before calling as_destroy. Otherwise if
   * as_destroy sleeps (which is quite possible) when we
   * come back we'll be calling as_activate on a
   * half-destroyed address space. This tends to be
   * messily fatal.
   */
  as = curproc_setas(NULL);
  as_destroy(as);

  /* detach this thread from its process */
  /* note: curproc cannot be used after this call */
  proc_remthread(curthread);

  /* if this is the last user process in the system, proc_destroy()
     will wake up the kernel menu thread */
  proc_destroy(p);

  thread_exit();
  /* thread_exit() does not return, so we should never get here */
  panic("return from thread_exit in sys_exit\n");
}
#else
void sys__exit(int exitcode) {

  struct addrspace *as;
  struct proc *p = curproc;
  /* for now, just include this to keep the compiler from complaining about
     an unused variable */
  (void)exitcode;

  DEBUG(DB_SYSCALL,"Syscall: _exit(%d)\n",exitcode);

  KASSERT(curproc->p_addrspace != NULL);
  as_deactivate();
  /*
   * clear p_addrspace before calling as_destroy. Otherwise if
   * as_destroy sleeps (which is quite possible) when we
   * come back we'll be calling as_activate on a
   * half-destroyed address space. This tends to be
   * messily fatal.
   */
  as = curproc_setas(NULL);
  as_destroy(as);

  /* detach this thread from its process */
  /* note: curproc cannot be used after this call */
  proc_remthread(curthread);

  /* if this is the last user process in the system, proc_destroy()
     will wake up the kernel menu thread */
  proc_destroy(p);

  thread_exit();
  /* thread_exit() does not return, so we should never get here */
  panic("return from thread_exit in sys_exit\n");
}
#endif
