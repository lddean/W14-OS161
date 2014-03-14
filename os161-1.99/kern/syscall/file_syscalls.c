#include <types.h>
#include <kern/errno.h>
#include <kern/unistd.h>
#include <lib.h>
#include <uio.h>
#include <syscall.h>
#include <vnode.h>
#include <vfs.h>
#include <current.h>
#include <proc.h>

#if OPT_A2
#include <copyinout.h>
#include <file_table.h>
#include <clock.h>
#endif
//#include <kern/fcntl.h>

/* handler for write() system call                  */
/*
 * n.b.
 * This implementation handles only writes to standard output 
 * and standard error, both of which go to the console.
 * Also, it does not provide any synchronization, so writes
 * are not atomic.
 *
 * You will need to improve this implementation
 */

struct file_table* curft;
int DEADBEEF = 0xdeadbeef; // the user space, give error

// file open
int sys_open(const char *filename, int flags, int mode, int *retval){
	//*retval = 1;
	if(flags & O_APPEND){
		*retval = EAGAIN;
		return -1;
	}

	struct vnode* vnode;
	struct file_descriptor* fd;
	int result;

	//int result; // debug for now
	//kprintf("test open is here %d %d %s\n", flags, mode,  filename);
	//kprintf("open fLAG TEST %d create %d RDWR %d O_TRUNC %d READ %d WRITE %d EXCEL %d APPEND %d\n", (flags == O_CREAT), O_CREAT, O_RDWR, O_TRUNC, O_RDONLY, O_WRONLY, O_EXCL, O_APPEND);

	// it points to an invalid user space
	char* tmp = kmalloc(sizeof(filename));
	result = copyin((const_userptr_t)filename, tmp, sizeof(filename));
	kfree(tmp);
	if(result == EFAULT){
		*retval = EFAULT;
		return -1;		
	}	

	// open the file
	result = vfs_open((char*)filename, flags, mode, &vnode);
//kprintf("vfs open fails %d\n", result);
	// failed(it is not a create , or DNE yet), even the limit?
	if(result){
		*retval = result;
		return -1;	
	}
	//vfs_open(fn, flags, mode, &vnode);

	// create file descriptor and add to file table
	fd = file_dst_create(vnode, flags, 0);
//test if it is null
	curft = curthread->ft;
	int add = file_table_add(curft, fd);
	
	*retval = add;
	return 0;
}

// file close
int sys_close(int index, int* retval){
	// now assume it is right
	*retval = 0;

	//index = 0;
	curft = curthread->ft;

//kprintf("inde is %d and size %d\n", index, curft->size);
	// index out of range or deadbeef
	if(index < 0 || curft->size <= index || index == DEADBEEF){
		*retval = EBADF;
		return -1;
	}

	int remove;
	remove = file_table_remove(curft, index);
	if(remove == -1){ // cannot remove the file(NULL)
		*retval = EBADF;
		return -1;
	}
	
	return 0;
}

int sys_write(int fdesc,userptr_t ubuf,unsigned int nbytes,int *retval){
//kprintf("write: %d, %x, %d\n", fdesc, (unsigned int)ubuf, nbytes);;
	curft = curthread->ft;
	// fdesc out of range or deadbeef
	if(fdesc < 0 || curft->size <= fdesc || fdesc == DEADBEEF){
		*retval = EBADF;
		return -1;
	}
	struct file_descriptor* fd = file_table_get(curft, fdesc);

	// file descriptor is NULL 
	if(fd == NULL || fd->flag == O_RDONLY){
		*retval = EBADF;
		return -1;
	}

  struct iovec iov;
  struct uio u;
  int res;

  //DEBUG(DB_SYSCALL,"Syscall: write(%d,%x,%d)\n",fdesc,(unsigned int)ubuf,nbytes);
  
  KASSERT(curproc != NULL);
  KASSERT(curproc->console != NULL);
  KASSERT(curproc->p_addrspace != NULL);

  /* set up a uio structure to refer to the user program's buffer (ubuf) */
  iov.iov_ubase = ubuf;
  iov.iov_len = nbytes;
  u.uio_iov = &iov;
  u.uio_iovcnt = 1;
  //u.uio_offset = 0;  /* not needed for the console */
  u.uio_offset = fd->offset;  /* not needed for the console */
  u.uio_resid = nbytes;
  u.uio_segflg = UIO_USERSPACE;
  u.uio_rw = UIO_WRITE;
  //u.uio_space = curproc->p_addrspace;
  u.uio_space = curproc_getas(); // have a spin lock attached

  //res = VOP_WRITE(curproc->console,&u);
  res = VOP_WRITE(fd->vnode,&u);

//kprintf("offset is %d\n", (int)u.uio_offset);
	// update the file descriptor offset
	fd->offset = u.uio_offset;

//kprintf("res valule %d\n", res);
  if (res) {
    return res;
  }

  /* pass back the number of bytes actually written */
  *retval = nbytes - u.uio_resid;
//kprintf("write retval %d\n", *retval);
  KASSERT(*retval >= 0);
  return 0;
}

// read file system
int sys_read(int fdesc,userptr_t ubuf,unsigned int nbytes,int *retval){
//kprintf("read: %d, %x, %d\n", fdesc, (unsigned int)ubuf, nbytes);;
	curft = curthread->ft;
	// fdesc out of range or deadbeef
	if(fdesc < 0 || curft->size <= fdesc || fdesc == DEADBEEF){
		*retval = EBADF;
		return -1;
	}

	struct file_descriptor* fd = file_table_get(curft, fdesc);

	// file descriptor is NULL 
	if(fd == NULL || fd->flag == O_WRONLY){
		*retval = EBADF;
		return -1;
	}

  struct iovec iov;
  struct uio u;
  int res;

  //DEBUG(DB_SYSCALL,"Syscall: write(%d,%x,%d)\n",fdesc,(unsigned int)ubuf,nbytes);
  
  KASSERT(curproc != NULL);
  KASSERT(curproc->console != NULL);
  KASSERT(curproc->p_addrspace != NULL);

  /* set up a uio structure to refer to the user program's buffer (ubuf) */
  iov.iov_ubase = ubuf;
  iov.iov_len = nbytes;
  u.uio_iov = &iov;
  u.uio_iovcnt = 1;
  //u.uio_offset = 0;  /* not needed for the console */
  u.uio_offset = fd->offset;  /* not needed for the console */
  u.uio_resid = nbytes;
  u.uio_segflg = UIO_USERSPACE;
  u.uio_rw = UIO_READ;
  //u.uio_space = curproc->p_addrspace;
  u.uio_space = curproc_getas(); // have a spin lock attached

  //res = VOP_WRITE(curproc->console,&u);
  res = VOP_READ(fd->vnode,&u);

//kprintf("offset is %d\n", (int)u.uio_offset);
	// update the file descriptor offset
	fd->offset = u.uio_offset;

  if (res) {
    return res;
  }

  /* pass back the number of bytes actually written */
  *retval = nbytes - u.uio_resid;
  KASSERT(*retval >= 0);
  return 0;
}
