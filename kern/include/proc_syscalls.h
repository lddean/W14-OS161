#ifndef _PROC_SYSCALLS_H_
#define _PROC_STSCALLS_H_

#include "opt-A2.h"

#if OPT_A2
#include <threadlist.h>
#include <thread.h>

//pid starts from 1 although procDetails array index starts from 0.

pid_t sys_getpid(void);
int sys_waitpid(pid_t pidid, int *status, int *options, int *ret_val);
void sys_exit (int exitcode);

#endif

#endif /* _PID_H_ */
