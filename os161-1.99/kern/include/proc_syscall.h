#ifndef PROC_SYSCALL_H
#define PROC_SYSCALL_H

#if OPT_A2
#include "opt-A2.h"
#include <thread.h>
#include <threadlist.h>

pid_t sys_getpid(void);
void sys_exit(int existcode);
int sys_waitpid(pid_t pid, int* status, int options, int *retval);

#endif
#endif

