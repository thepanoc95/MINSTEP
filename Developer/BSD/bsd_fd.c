/*
 * Developer/BSD/bsd_fd.c
 *
 * File descriptor table management for the BSD server.
 * Each process maintains its own file descriptor table mapping
 * BSD file descriptors to host OS file descriptors.
 */

#include "bsd.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

/* -----------------------------------------------------------------------
 *  FD table initialization
 * ----------------------------------------------------------------------- */

kern_return_t bsd_fd_init(bsd_process_t *proc)
{
    if (!proc) return KERN_INVALID_TASK;

    for (int i = 0; i < BSD_MAX_FD; i++) {
        proc->fd_table[i].in_use = NO;
        proc->fd_table[i].host_fd = -1;
    }

    proc->fd_table[0].host_fd = dup(STDIN_FILENO);
    proc->fd_table[0].in_use = YES;
    proc->fd_table[0].access_mode = O_RDONLY;
    proc->fd_table[0].flags = 0;

    proc->fd_table[1].host_fd = dup(STDOUT_FILENO);
    proc->fd_table[1].in_use = YES;
    proc->fd_table[1].access_mode = O_WRONLY;
    proc->fd_table[1].flags = 0;

    proc->fd_table[2].host_fd = dup(STDERR_FILENO);
    proc->fd_table[2].in_use = YES;
    proc->fd_table[2].access_mode = O_WRONLY;
    proc->fd_table[2].flags = 0;

    proc->fd_count = 3;
    return KERN_SUCCESS;
}

/* -----------------------------------------------------------------------
 *  Find free FD slot
 * ----------------------------------------------------------------------- */

static int bsd_fd_find_free(bsd_process_t *proc)
{
    for (int i = 0; i < BSD_MAX_FD; i++) {
        if (!proc->fd_table[i].in_use) return i;
    }
    return -1;
}

/* -----------------------------------------------------------------------
 *  File operations
 * ----------------------------------------------------------------------- */

int bsd_fd_open(bsd_process_t *proc, const char *path, int flags, int mode)
{
    if (!proc || !path) { errno = EINVAL; return -1; }

    int slot = bsd_fd_find_free(proc);
    if (slot < 0) { errno = EMFILE; return -1; }

    int host_fd = open(path, flags, mode);
    if (host_fd < 0) return -1;

    proc->fd_table[slot].host_fd = host_fd;
    proc->fd_table[slot].in_use = YES;
    proc->fd_table[slot].access_mode = flags & (O_RDONLY | O_WRONLY | O_RDWR);
    proc->fd_table[slot].flags = (flags & O_CLOEXEC) ? FD_CLOEXEC : 0;
    strncpy(proc->fd_table[slot].path, path,
            sizeof(proc->fd_table[slot].path) - 1);

    return slot;
}

int bsd_fd_close(bsd_process_t *proc, int fd)
{
    if (!proc || fd < 0 || fd >= BSD_MAX_FD) { errno = EBADF; return -1; }
    if (!proc->fd_table[fd].in_use) { errno = EBADF; return -1; }

    close(proc->fd_table[fd].host_fd);
    proc->fd_table[fd].in_use = NO;
    proc->fd_table[fd].host_fd = -1;
    proc->fd_count--;
    return 0;
}

int bsd_fd_read(bsd_process_t *proc, int fd, void *buf, size_t nbyte)
{
    if (!proc || fd < 0 || fd >= BSD_MAX_FD) { errno = EBADF; return -1; }
    if (!proc->fd_table[fd].in_use) { errno = EBADF; return -1; }

    return read(proc->fd_table[fd].host_fd, buf, nbyte);
}

int bsd_fd_write(bsd_process_t *proc, int fd, const void *buf, size_t nbyte)
{
    if (!proc || fd < 0 || fd >= BSD_MAX_FD) { errno = EBADF; return -1; }
    if (!proc->fd_table[fd].in_use) { errno = EBADF; return -1; }

    return write(proc->fd_table[fd].host_fd, buf, nbyte);
}

off_t bsd_fd_lseek(bsd_process_t *proc, int fd, off_t offset, int whence)
{
    if (!proc || fd < 0 || fd >= BSD_MAX_FD) { errno = EBADF; return -1; }
    if (!proc->fd_table[fd].in_use) { errno = EBADF; return -1; }

    return lseek(proc->fd_table[fd].host_fd, offset, whence);
}

int bsd_fd_dup(bsd_process_t *proc, int oldfd)
{
    if (!proc || oldfd < 0 || oldfd >= BSD_MAX_FD) { errno = EBADF; return -1; }
    if (!proc->fd_table[oldfd].in_use) { errno = EBADF; return -1; }

    int slot = bsd_fd_find_free(proc);
    if (slot < 0) { errno = EMFILE; return -1; }

    int host_fd = dup(proc->fd_table[oldfd].host_fd);
    if (host_fd < 0) return -1;

    proc->fd_table[slot].host_fd = host_fd;
    proc->fd_table[slot].in_use = YES;
    proc->fd_table[slot].access_mode = proc->fd_table[oldfd].access_mode;
    proc->fd_table[slot].flags = proc->fd_table[oldfd].flags;
    strncpy(proc->fd_table[slot].path, proc->fd_table[oldfd].path,
            sizeof(proc->fd_table[slot].path) - 1);

    return slot;
}

int bsd_fd_dup2(bsd_process_t *proc, int oldfd, int newfd)
{
    if (!proc || oldfd < 0 || oldfd >= BSD_MAX_FD) { errno = EBADF; return -1; }
    if (newfd < 0 || newfd >= BSD_MAX_FD) { errno = EBADF; return -1; }
    if (!proc->fd_table[oldfd].in_use) { errno = EBADF; return -1; }

    if (oldfd == newfd) return newfd;

    int host_fd = dup2(proc->fd_table[oldfd].host_fd,
                       proc->fd_table[newfd].host_fd >= 0 ?
                       proc->fd_table[newfd].host_fd : newfd);
    if (host_fd < 0) return -1;

    proc->fd_table[newfd].host_fd = host_fd;
    proc->fd_table[newfd].in_use = YES;
    proc->fd_table[newfd].access_mode = proc->fd_table[oldfd].access_mode;
    proc->fd_table[newfd].flags = proc->fd_table[oldfd].flags;
    strncpy(proc->fd_table[newfd].path, proc->fd_table[oldfd].path,
            sizeof(proc->fd_table[newfd].path) - 1);

    return newfd;
}

int bsd_fd_pipe(bsd_process_t *proc, int fds[2])
{
    if (!proc || !fds) { errno = EINVAL; return -1; }

    int slot_r = bsd_fd_find_free(proc);
    if (slot_r < 0) { errno = EMFILE; return -1; }

    int slot_w = bsd_fd_find_free(proc);
    if (slot_w < 0) { errno = EMFILE; return -1; }

    int host_fds[2];
    if (pipe(host_fds) < 0) return -1;

    proc->fd_table[slot_r].host_fd = host_fds[0];
    proc->fd_table[slot_r].in_use = YES;
    proc->fd_table[slot_r].access_mode = O_RDONLY;

    proc->fd_table[slot_w].host_fd = host_fds[1];
    proc->fd_table[slot_w].in_use = YES;
    proc->fd_table[slot_w].access_mode = O_WRONLY;

    fds[0] = slot_r;
    fds[1] = slot_w;
    return 0;
}

int bsd_fd_ioctl(bsd_process_t *proc, int fd, unsigned long request, void *arg)
{
    if (!proc || fd < 0 || fd >= BSD_MAX_FD) { errno = EBADF; return -1; }
    if (!proc->fd_table[fd].in_use) { errno = EBADF; return -1; }

    return ioctl(proc->fd_table[fd].host_fd, request, arg);
}

int bsd_fd_fcntl(bsd_process_t *proc, int fd, int cmd, int arg)
{
    if (!proc || fd < 0 || fd >= BSD_MAX_FD) { errno = EBADF; return -1; }
    if (!proc->fd_table[fd].in_use) { errno = EBADF; return -1; }

    return fcntl(proc->fd_table[fd].host_fd, cmd, arg);
}

/* -----------------------------------------------------------------------
 *  Path operations
 * ----------------------------------------------------------------------- */

int bsd_fd_stat(bsd_process_t *proc, const char *path, void *buf)
{
    (void)proc;
    if (!path || !buf) { errno = EINVAL; return -1; }
    return stat(path, (struct stat *)buf);
}

int bsd_fd_fstat(bsd_process_t *proc, int fd, void *buf)
{
    if (!proc || !buf) { errno = EINVAL; return -1; }
    if (fd < 0 || fd >= BSD_MAX_FD) { errno = EBADF; return -1; }
    if (!proc->fd_table[fd].in_use) { errno = EBADF; return -1; }

    return fstat(proc->fd_table[fd].host_fd, (struct stat *)buf);
}

int bsd_fd_access(bsd_process_t *proc, const char *path, int mode)
{
    (void)proc;
    if (!path) { errno = EINVAL; return -1; }
    return access(path, mode);
}

int bsd_fd_chdir(bsd_process_t *proc, const char *path)
{
    if (!proc || !path) { errno = EINVAL; return -1; }

    if (chdir(path) < 0) return -1;

    strncpy(proc->cwd, path, BSD_MAX_CWD - 1);
    proc->cwd[BSD_MAX_CWD - 1] = '\0';
    return 0;
}

int bsd_fd_getcwd(bsd_process_t *proc, char *buf, size_t size)
{
    if (!proc || !buf) { errno = EINVAL; return -1; }

    if (getcwd(buf, size) != NULL) {
        return 0;
    }
    return -1;
}

int bsd_fd_chmod(bsd_process_t *proc, const char *path, mode_t mode)
{
    (void)proc;
    if (!path) { errno = EINVAL; return -1; }
    return chmod(path, mode);
}

int bsd_fd_unlink(bsd_process_t *proc, const char *path)
{
    (void)proc;
    if (!path) { errno = EINVAL; return -1; }
    return unlink(path);
}

int bsd_fd_rename(bsd_process_t *proc, const char *old, const char *new_path)
{
    (void)proc;
    if (!old || !new_path) { errno = EINVAL; return -1; }
    return rename(old, new_path);
}

int bsd_fd_mkdir(bsd_process_t *proc, const char *path, mode_t mode)
{
    (void)proc;
    if (!path) { errno = EINVAL; return -1; }
    return mkdir(path, mode);
}

int bsd_fd_rmdir(bsd_process_t *proc, const char *path)
{
    (void)proc;
    if (!path) { errno = EINVAL; return -1; }
    return rmdir(path);
}

int bsd_fd_select(bsd_process_t *proc, int nfds, fd_set *readfds,
                  fd_set *writefds, fd_set *errorfds, struct timeval *timeout)
{
    if (!proc) { errno = EINVAL; return -1; }

    fd_set host_rfds, host_wfds, host_efds;
    fd_set *r_ptr = NULL, *w_ptr = NULL, *e_ptr = NULL;
    int max_fd = 0;

    if (readfds) {
        FD_ZERO(&host_rfds);
        for (int i = 0; i < nfds; i++) {
            if (FD_ISSET(i, readfds) && proc->fd_table[i].in_use) {
                FD_SET(proc->fd_table[i].host_fd, &host_rfds);
                if ((int)proc->fd_table[i].host_fd > max_fd)
                    max_fd = (int)proc->fd_table[i].host_fd;
            }
        }
        r_ptr = &host_rfds;
    }

    if (writefds) {
        FD_ZERO(&host_wfds);
        for (int i = 0; i < nfds; i++) {
            if (FD_ISSET(i, writefds) && proc->fd_table[i].in_use) {
                FD_SET(proc->fd_table[i].host_fd, &host_wfds);
                if ((int)proc->fd_table[i].host_fd > max_fd)
                    max_fd = (int)proc->fd_table[i].host_fd;
            }
        }
        w_ptr = &host_wfds;
    }

    if (errorfds) {
        FD_ZERO(&host_efds);
        for (int i = 0; i < nfds; i++) {
            if (FD_ISSET(i, errorfds) && proc->fd_table[i].in_use) {
                FD_SET(proc->fd_table[i].host_fd, &host_efds);
                if ((int)proc->fd_table[i].host_fd > max_fd)
                    max_fd = (int)proc->fd_table[i].host_fd;
            }
        }
        e_ptr = &host_efds;
    }

    int ret = select(max_fd + 1, r_ptr, w_ptr, e_ptr, timeout);
    if (ret < 0) return -1;

    if (readfds) {
        FD_ZERO(readfds);
        for (int i = 0; i < nfds; i++) {
            if (proc->fd_table[i].in_use &&
                FD_ISSET((int)proc->fd_table[i].host_fd, &host_rfds)) {
                FD_SET(i, readfds);
            }
        }
    }
    if (writefds) {
        FD_ZERO(writefds);
        for (int i = 0; i < nfds; i++) {
            if (proc->fd_table[i].in_use &&
                FD_ISSET((int)proc->fd_table[i].host_fd, &host_wfds)) {
                FD_SET(i, writefds);
            }
        }
    }
    if (errorfds) {
        FD_ZERO(errorfds);
        for (int i = 0; i < nfds; i++) {
            if (proc->fd_table[i].in_use &&
                FD_ISSET((int)proc->fd_table[i].host_fd, &host_efds)) {
                FD_SET(i, errorfds);
            }
        }
    }

    return ret;
}