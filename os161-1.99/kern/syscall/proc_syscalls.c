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
#include <proc_syscalls.h>
#include <kern/wait.h>

#include <copyinout.h>

pid_t sys_getpid(void){
	return curthread->pid;
}
void sys_exit(int exitcode){
	struct procInfo* pInfo = procInfo_get(sys_getpid());
	
	if (pInfo!=NULL && pInfo->active!=0){
		pInfo->active = 0;
		pInfo->exitcode = exitcode;
		lock_acquire(pInfo->plock);
		cv_signal(pInfo->pcv, pInfo->plock);
		lock_release(pInfo->plock);
	}

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
	if (pInfo==NULL){ //|| pInfo->active==0){
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
		*status = _MKWAIT_EXIT(pInfo->exitcode);
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

int
sys_execv(const char *progname, char **args, int *rv)
{
    
    if (progname == NULL || args == NULL){
        
        *rv = EFAULT;
        return -1;
        
    }

    
    if ((unsigned int)args == 0x80000000 || (unsigned int)args == 0x40000000){
        *rv = EFAULT;
        return -1;
    }

    
	struct addrspace *as;
	struct vnode *v;
	vaddr_t entrypoint, stackptr;
	int result;
    
    int count_args;
    
    size_t *size_of_args;
    
    count_args = 0;
    
    if (args[0] == NULL){
        
        *rv = EFAULT;
        return -1;
    }
    while (args[count_args] != NULL){
        
        if ((unsigned int)args[count_args] == 0x80000000 || (unsigned int)args[count_args] == 0x40000000){
            *rv = EFAULT;
            return -1;
        }
        
        
        if (strlen(args[count_args]) > PATH_MAX){
            
            *rv = E2BIG;
            return -1;
            
        }
        
        count_args = count_args + 1;
    }
    
    
    char *name;
    
    name = (char *)kmalloc(sizeof PATH_MAX);
    
    size_t progname_name = 0;
    
    if (name == NULL){
        
        *rv = EFAULT;
        return -1;
    }
    
    /*if (strcmp(progname, "")){
	*rv = EINVAL;
	return -1;
    }*/

    result = copyinstr((userptr_t)progname, name, PATH_MAX,size_of_args);
    
    if (result){
        
        progname_name = progname_name + 1;
        *rv = EFAULT;
        return -1;
    }
    
    if (strlen(name) == 0){
        //*rv = EFAULT;
        *rv = EINVAL;
        return -1;
    }
    
    char **name_list;
    
    name_list = kmalloc(sizeof ARG_MAX);
    
    for (int i = 0; i < count_args; i++){
        
        name_list[i] = (char *)kmalloc(sizeof PATH_MAX);
        
        if(name_list[i] == NULL){
            *rv = ENOMEM;
            return -1;
        }
    }
    
    for (int i = 0; i < count_args; i++){
        
        size_t str_length;
        str_length = strlen(args[i]);
        result = copyinstr((userptr_t)args[i], name_list[i], str_length, size_of_args);
    }
    
    
    
    
	/* Open the file. */
	result = vfs_open(name, O_RDONLY, 0, &v);
	if (result) {
        *rv = EISDIR;
		return -1;
	}
    
    
	/* We should be a new process. */
	//KASSERT(curproc_getas() == NULL);
    
	/* Create a new address space. */
	as = as_create();
	if (as ==NULL) {
		vfs_close(v);
        *rv = ENOMEM;
		return ENOMEM;
	}
    
	/* Switch to it and activate it. */
	curproc_setas(as);
	as_activate();
    
	/* Load the executable. */
	result = load_elf(v, &entrypoint);
	if (result) {
		/* p_addrspace will go away when curproc is destroyed */
		vfs_close(v);
        *rv = EIO;
		return -1;
	}
    
	/* Done with the file now. */
	vfs_close(v);
    
	/* Define the user stack in the address space */
	result = as_define_stack(as, &stackptr);
	if (result) {
		/* p_addrspace will go away when curproc is destroyed */
        *rv = EFAULT;
		return -1;
	}
    
	int count;
	count = (int) count_args;
	vaddr_t address[count + 1];
    
	int i = 0;
    
	size_t j = 0;
    
    
	for (i = count - 1; i >= 0; i --){
		size_t size;
		size = strlen(name_list[i]) + 1;
		
		j = j + size;
        
        
		if (size % 4 != 0){
            
			size = size - (size % 4) + 4;
            
			stackptr = stackptr - size;
		}else{
			stackptr = stackptr - size;
		}
		
		//result = 0;
        
        
		result = copyoutstr(name_list[i],(userptr_t)stackptr,size, &size);
        
		if (result){
            
			*rv = EFAULT;
			return -1;
		}
        
		address[i] = stackptr;
	}
	//int count1 = count;
    
	address[count] = 0;
    
	for (i = count; i >= 0 ; i --){
		int stack = 4;
		
        stack = stack + 4;
        
		stackptr = stackptr - 4;
        
		result = copyout(&address[i],(userptr_t)stackptr,4);
        
		if(result){
			*rv = EFAULT;
			return -1;
		}
        
	}
    
	vaddr_t n;
    
	n = stackptr;
	
	int new_stack  = 8; //the stack must divide by 8;
	
	new_stack = (new_stack % 8 )* 8;
    
	if (stackptr % 8 != 0){
        
		stackptr = stackptr - stackptr % 8;
        
	}
    
	/* Warp to user mode. */
	enter_new_process(count /*argc*/,  (userptr_t)n/*userspace addr of argv*/,
                      stackptr, entrypoint);
	
	/* enter_new_process does not return. */
	panic("enter_new_process returned\n");
    *rv = EINVAL;
	return -1;
}

