#include "opt-A2.h"
#if OPT_A2

#include <types.h>
#include <kern/errno.h>
#include <kern/unistd.h>
#include <lib.h>
#include <syscall.h>
#include <current.h>
#include <proc.h>
#include <thread.h>
#include <addrspace.h>
#include <pid.h>
#include <limits.h>
#include <synch.h>
//global variable
static struct proc_table* pt;

struct proc_table* proc_table_create(void){
	//struct proc_table* pt;
	pt = kmalloc(sizeof(struct proc_table));
	if (pt==NULL){
		return NULL;
	}
	pt->procInfoLst = array_create();
	pt->size = 0;
	//pt->nullPids = array_create();
	return pt;
}


void proc_table_destroy(void/*struct proc_table *pt*/){
	pt->size = 0;

	struct array* pInfoLst = pt->procInfoLst;
		
	int size1 = array_num(pInfoLst);	
	struct procInfo* pInfo;
	for (int i=0; i<size1; i++){
		pInfo = array_get(pInfoLst,i);
		lock_destroy(pInfo->plock);
		cv_destroy(pInfo->pcv);
		kfree(array_get(pInfoLst,i));
	}
	//array_destroy(pInfoLst);
	/*
	struct array* nullPids = pt->nullPids;
	int size2 = array_num(nullPids);
	for (int i=0; i<size2; i++){
		kfree(array_get(nullPids,i));
	}
	//array_destroy(nullPids);
	*/
	kfree(pt);
}

struct procInfo* procInfo_get(pid_t pid){
	//struct procInfo* tmp = procInfo_create(1,1);
	//return tmp;
	if (pt ==NULL || pid > pt->size || pid < 1){
		return NULL;
	}
	else{
		return array_get(pt->procInfoLst,pid);
	}
}

struct procInfo* procInfo_create(pid_t curPid, pid_t parPid){
	struct procInfo* new = kmalloc(sizeof(struct procInfo));
	KASSERT(new!=NULL);
	
	new->active = 1;
	new->currentPid = curPid;
	new->parentPid = parPid;
	new->exitcode = -1;
	new->plock = lock_create("plock");
	new->pcv = cv_create("pcv");
	return new;
}

pid_t proc_table_add(void){
	if (pt!=NULL && pt->size == PID_MAX){
		//cerr << "Cannot fit more processes into proc_table" << endl;
		return 0;//error
	}

	if (pt==NULL){
		pt = proc_table_create();
	}
	pt->size+=1;

	struct procInfo *pInfo_new; 
	struct procInfo *pInfo_reuse; 
	
	if (pt->size==1){
		pInfo_new = procInfo_create(pt->size, -1); //no parent
		return pt->size;
	}
	//else (pt->size>1)
	for (int index=1; index <= pt->size; index++){
		struct procInfo *pInfo = array_get(pt->procInfoLst, index);
		int active = pInfo->active;
		if (active==0){
			pInfo_reuse = procInfo_create(index, curthread->pid);
			array_set(pt->procInfoLst, index, pInfo_reuse);	 	
			pInfo->active = 1;
			return index; //current Pid
		}
	}
	unsigned result;
	pInfo_new = procInfo_create(pt->size, curthread->pid);
	array_add(pt->procInfoLst, pInfo_new, &result);
	return pt->size; //current Pid

}
/*
pid_t* nullPid_create(pid_t pid){
	pid_t *new =  kmalloc(sizeof(pid_t));
	KASSERT(new!=NULL);
	*new = pid;
	return new;
}

void proc_table_remove(pid_t pid){
	//unsigned int index;
	//index = (unsigned int)pid;	
	if (array_get(pt->procInfoLst,pid)==NULL){//out of bound
		return;
	}
	struct procInfo* pInfo = array_get(pt->procInfoLst, pid);
	pInfo->flag = 0;
	//array_set(pt, index, NULL);//what to do when a proc gets removed? release resource?
	pid_t* nullPid = nullPid_create(pid);
	unsigned result;
	array_add(pt->nullPids, nullPid, &result); //add like a list (side by side)
}
*/

#else
#endif
