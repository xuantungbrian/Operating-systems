#include <types.h>
#include <kern/errno.h>
#include <lib.h>
#include <uio.h>
#include <proc.h>
#include <current.h>
#include <addrspace.h>
#include <vnode.h>
#include <elf.h>
#include <limits.h>
#include <vfs.h>
#include <fs.h>
#include <test.h>
#include <copyinout.h>
#include <synch.h>
#include <filetable.h>
#include <proc.h>
#include <sys_function.h>


int
sys_open(userptr_t *filename, int flags, int32_t *retval)
{
    struct vnode* vn;
    int i;
    int err = 0;
    char buf[PATH_MAX];
    size_t *actual = NULL;
	//struct opentable* entry;
    err = copyinstr((const_userptr_t)filename, buf, sizeof(buf), actual ); //must change 4th argument
    if (err) {
        kfree(vn);
		return err;
	}
    err = vfs_open(buf, flags, 0, &vn); // need to add error codes
	if (err) { //if the file is opened, should we return err or add something pointing to the vnode?
        kfree(vn);
		return err;
	}
    lock_acquire(curproc->fd->fdlock);

    for(i = 0; i < __OPEN_MAX ; i++) { //confirm i = 0 or 3
		//entry = proc->fd->fd_entry[i];
        if(curproc->fd->fd_entry[i] == NULL){
            //lock_acquire(fd_lock);
            curproc->fd->fd_entry[i] = kmalloc(sizeof(struct opentable));
            if(curproc->fd->fd_entry[i] == NULL){
                lock_release(curproc->fd->fdlock);
                return 29; // return ENFILE
            }
            curproc->fd->fd_entry[i]->offset = 0;// confirm offset starts at 0
            curproc->fd->fd_entry[i]->vnode_ptr = vn;
            curproc->fd->fd_entry[i]->flags = flags;
            //lock_release(fd_lock);
            break;
        }
    }
    lock_release(curproc->fd->fdlock);
    if(i == __OPEN_MAX){
        return 28;// return EMFILE
    }
    *retval = i;
    kprintf("%d\n",i);
    return err;
}

int
sys_close(int fd)
{
	lock_acquire(curproc->fd->fdlock);
	if (curproc->fd->fd_entry[fd] == NULL) {
		kprintf("NO!!!!  %d\n", fd);
        lock_release(curproc->fd->fdlock);
		return 30; // Return EBDAF - refer to errno.h
	}
	vfs_close(curproc->fd->fd_entry[fd]->vnode_ptr);
	kfree(curproc->fd->fd_entry[fd]);
	curproc->fd->fd_entry[fd] = NULL;
	lock_release(curproc->fd->fdlock);
	kprintf("closeeeeeeeeeeeeeeeeeeee  %d\n", fd);
	return 0;
}

int
sys_dup2(int oldfd, int newfd, int32_t *retval)
{   

    lock_acquire(curproc->fd->fdlock);

    if(curproc->fd->fd_entry[oldfd] == NULL || oldfd < 0 || newfd < 0 || oldfd > __OPEN_MAX || newfd > __OPEN_MAX){
        lock_release(curproc->fd->fdlock);
        return 30; // return EBADF
    }
    if(oldfd == newfd) {
        lock_release(curproc->fd->fdlock);
        return 0;
    }
    if(curproc->fd->fd_entry[newfd] != NULL){
        sys_close(newfd);
        lock_release(curproc->fd->fdlock);
        return 0;
    }
    curproc->fd->fd_entry[newfd] = curproc->fd->fd_entry[oldfd];
    curproc->fd->fd_entry[newfd]->vnode_ptr->vn_refcount++;

    *retval = newfd;

    lock_release(curproc->fd->fdlock);
    kprintf("sys dup2  %d to %d\n", oldfd, newfd);
    return 0;
    
}

int
sys_chdir(const char *pathname){

    int err = 0;
    char buf[PATH_MAX];
    size_t *actual = NULL;
	//struct opentable* entry;
    err = copyinstr((const_userptr_t)pathname, buf, sizeof(buf), actual );
    if(err){
        return err;
    }

    err = vfs_chdir((char *) pathname);
    if(err){
        return err;
    }
    return 0;

}