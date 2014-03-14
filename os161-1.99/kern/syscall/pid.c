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
	pt->nullPid = array_create();
	return pt;
}


void proc_table_destroy(void/*struct proc_table *pt*/){
	pt->size = 0;

	struct array* pInfoLst = pt->procInfoLst;
	
	int size = array_num(pInfoLst);
	for (int i=0; i<size; i++){
		lock_destroy((array_get(pInfoLst,i))->plock);
		cv_destroy((array_get(pInfoLst,i))->pcv);
		kfree(array_get(pInfoLst,i));
	}
	//array_destroy(pInfoLst);

	struct array* nullPids = pt->nullPids;
	int size = array_num(nullPids);
	for (int i=0; i<size; i++){
		kfree(array_get(nullPids,i));
	}
	//array_destroy(nullPids);

	kfree(pt);
}

struct procInfo* procInfo_get(pid_t pid){
	return array_get(pt->procInfoLst,pid);
}

struct procInfo* procInfo_create(int curPid, int parPid){
	struct procInfo* new = malloc(sizeof(struct procInfo));
	KASSERT(new!=NULL);
	
	new->flag = 1;
	new->currentPid = curPid;
	new->parentPid = parPid;
	//new->exit_status = -1;
	new->plock = lock_create("plock");
	new->pcv = cv_create("pcv");
	return new;
}

pid_t proc_table_add(void){
	if (pt->size == PID_MAX){
		cerr << "Cannot fit more processes into proc_table" << endl;
		return 0;//error
	}

	if (pt==NULL){
		pt = proc_table_create();
	}
	pt->size+=1;

	if (array_num(pt->nullPids)==0){
		if (pt->size==1){
			struct procInfo *pInfo = procInfo_create(pt->size, -1); //no parent
		}	
		else{	
			struct procInfo *pInfo = procInfo_create(pt->size, curthread->pid);
		}	
		array_set(pt->procInfoLst, pt->size, pInfo);
		return pt->size;
	}
	else{
		int size = array_num(pt->nullPids);
		int i, index;
		for(i=0; i<size; i++){
			index = *(array_get(pt->nullPids, i));//gives back index in procInfoLst
			int flag = (array_get(pt->procInfoLst, index))->flag;
			if (flag==0){//inactive/ is equal to null then reuse pid
				struct procInfo *pInfo = procInfo_create(index, curthread->pid);
				array_set(pt->procInfoLst, index, pInfo);
				(array_get(pt->procInfoLst, index))->flag = 1;
				array_remove(pt->nullPids, i);
				break;
			}
		}
		return index;
	}
}

pid_t* nullPid_create(pid_t pid){
	pid_t *new =  malloc(sizeof(pid_t));
	KASSERT(new!=NULL);
	*new = pid;
	return new;
}

void proc_table_remove(/*struct proc_table* pt,*/ pid_t pid){
	unsigned int index;
	index = (unsigned int)pid;	
	if (array_get(pt,index)==NULL){//out of bound
		return;
	}
	(array_get(pt->procInfoLst, index))->flag = 0;
	//array_set(pt, index, NULL);//what to do when a proc gets removed? release resource?
	pid_t* nullPid = nullPid_create(index);
	usigned result;
	array_add(pt->nullPids, nullPid, &result); //add like a list (side by side)
}


#else
#endif
