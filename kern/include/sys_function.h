

#ifndef _SYSFUNCTION_H_
#define _SYSFUNCTION_H_
int sys_open(userptr_t *filename, int flags, int32_t *retval);
int sys_close(int fd);
int sys_dup2(int oldfd, int newfd, int32_t *retval);
int sys_chdir(const char *pathname);
int sys_read(int fd, void *buf, size_t buflen, int* retval);
int sys_write(int fd, const void *buf, size_t nbytes, int* retval);
int sys_lseek(int fd, off_t pos, int whence, int* retval);
int sys__getcwd(char *buf, size_t buflen, int32_t *retval);
#endif
