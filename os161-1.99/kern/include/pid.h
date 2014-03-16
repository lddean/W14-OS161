#ifndef PID_H
#define PID_H

#include "opt-A2.h"
#if OPT_A2

#include<array.h>
#include<proc.h>
#include<thread.h>
#include<lib.h>
#include<syscall.h>
#include<array.h>
#include<types.h>

// pid functions prototypes
struct proc_table{
	struct array *procInfoLst; //procInfo structures
	int size;
	struct array *nullPids; //pid_t
};

struct procInfo{
	int flag; //0-inactive 1-active
	pid_t currentPid;
	pid_t parentPid;
	//int exit_status;
	//struct array *childrenPid; //list of pid_t types 
	//int childrenSize;
	struct lock* plock;
	struct cv* pcv;
};

struct proc_table* proc_table_create(void);
void proc_table_destroy(void);
struct procInfo* procInfo_get(pid_t pid);
struct procInfo* procInfo_create(int curPid, int parPid);
// add process info and return current pid
pid_t proc_table_add(void);
pid_t *nullPid_create(pid_t pid);
// remove a process and add pid to nullPids
void proc_table_remove(pid_t pid);

#endif
#endif
