/*
 * Developer/BSD/bsd_syscall.c
 *
 * System call dispatch for the BSD server.
 * Routes incoming syscall requests to the appropriate handler
 * based on the syscall number.
 */

#include "bsd.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/resource.h>

/* -----------------------------------------------------------------------
 *  Main dispatch
 * ----------------------------------------------------------------------- */

kern_return_t bsd_syscall_dispatch(bsd_syscall_request_t *req,
                                   bsd_syscall_reply_t *reply)
{
    if (!req || !reply) return KERN_INVALID_ARGUMENT;

    memset(reply, 0, sizeof(bsd_syscall_reply_t));
    reply->header.msgh_size = sizeof(bsd_syscall_reply_t);
    reply->header.msgh_id = BSD_MSG_ID_SYSCALL_RET;

    bsd_process_t *proc = bsd_process_lookup(req->caller_pid);
    if (!proc) {
        reply->ret_code = -1;
        reply->errno_val = ESRCH;
        return KERN_INVALID_TASK;
    }

    switch (req->syscall_num) {
    /* -- Process ---------------------------------------- */

    case BSD_SYSCALL_EXIT:
        bsd_syscall_exit(proc, req->arg1);
        reply->ret_code = 0;
        break;

    case BSD_SYSCALL_FORK:
        return bsd_syscall_fork(proc, reply);

    case BSD_SYSCALL_EXEC: {
        const char *path = (const char *)(uintptr_t)req->arg1;
        kern_return_t kr = bsd_syscall_exec(proc, path, NULL, NULL);
        reply->ret_code = (kr == KERN_SUCCESS) ? 0 : -1;
        reply->errno_val = errno;
        break;
    }

    case BSD_SYSCALL_WAITPID:
        return bsd_syscall_waitpid(proc, req->arg1,
                                   (int *)(uintptr_t)req->arg2,
                                   req->arg3, reply);

    case BSD_SYSCALL_GETPID:
        return bsd_syscall_getpid(proc, reply);

    case BSD_SYSCALL_GETPPID:
        return bsd_syscall_getppid(proc, reply);

    case BSD_SYSCALL_GETUID:
        return bsd_syscall_getuid(proc, reply);

    case BSD_SYSCALL_GETEUID:
        return bsd_syscall_geteuid(proc, reply);

    case BSD_SYSCALL_GETGID:
        return bsd_syscall_getgid(proc, reply);

    case BSD_SYSCALL_GETEGID:
        return bsd_syscall_getegid(proc, reply);

    case BSD_SYSCALL_SETUID: {
        kern_return_t kr = bsd_syscall_setuid(proc, (uid_t)req->arg1);
        reply->ret_code = (kr == KERN_SUCCESS) ? 0 : -1;
        reply->errno_val = errno;
        break;
    }

    case BSD_SYSCALL_SETGID: {
        kern_return_t kr = bsd_syscall_setgid(proc, (gid_t)req->arg1);
        reply->ret_code = (kr == KERN_SUCCESS) ? 0 : -1;
        reply->errno_val = errno;
        break;
    }

    /* -- File I/O --------------------------------------- */

    case BSD_SYSCALL_OPEN: {
        const char *path = (const char *)(uintptr_t)req->arg1;
        int fd = bsd_fd_open(proc, path, req->arg2, req->arg3);
        reply->ret_code = fd;
        reply->errno_val = (fd < 0) ? errno : 0;
        break;
    }

    case BSD_SYSCALL_CLOSE: {
        int ret = bsd_fd_close(proc, req->arg1);
        reply->ret_code = ret;
        reply->errno_val = (ret < 0) ? errno : 0;
        break;
    }

    case BSD_SYSCALL_READ: {
        void *buf = (void *)(uintptr_t)req->arg2;
        size_t nbyte = (size_t)req->arg3;
        int ret = bsd_fd_read(proc, req->arg1, buf, nbyte);
        reply->ret_code = ret;
        reply->errno_val = (ret < 0) ? errno : 0;
        break;
    }

    case BSD_SYSCALL_WRITE: {
        const void *buf = (const void *)(uintptr_t)req->arg2;
        size_t nbyte = (size_t)req->arg3;
        int ret = bsd_fd_write(proc, req->arg1, buf, nbyte);
        reply->ret_code = ret;
        reply->errno_val = (ret < 0) ? errno : 0;
        break;
    }

    case BSD_SYSCALL_LSEEK: {
        off_t ret = bsd_fd_lseek(proc, req->arg1, (off_t)req->arg2, req->arg3);
        reply->ret_code = (int32_t)ret;
        reply->errno_val = (ret < 0) ? errno : 0;
        break;
    }

    case BSD_SYSCALL_DUP: {
        int ret = bsd_fd_dup(proc, req->arg1);
        reply->ret_code = ret;
        reply->errno_val = (ret < 0) ? errno : 0;
        break;
    }

    case BSD_SYSCALL_DUP2: {
        int ret = bsd_fd_dup2(proc, req->arg1, req->arg2);
        reply->ret_code = ret;
        reply->errno_val = (ret < 0) ? errno : 0;
        break;
    }

    case BSD_SYSCALL_PIPE: {
        int fds[2];
        int ret = bsd_fd_pipe(proc, fds);
        reply->ret_code = ret;
        reply->out_arg1 = fds[0];
        reply->out_arg2 = fds[1];
        reply->errno_val = (ret < 0) ? errno : 0;
        break;
    }

    case BSD_SYSCALL_IOCTL: {
        int ret = bsd_fd_ioctl(proc, req->arg1, (unsigned long)req->arg2,
                               (void *)(uintptr_t)req->arg3);
        reply->ret_code = ret;
        reply->errno_val = (ret < 0) ? errno : 0;
        break;
    }

    case BSD_SYSCALL_FCNTL: {
        int ret = bsd_fd_fcntl(proc, req->arg1, req->arg2, req->arg3);
        reply->ret_code = ret;
        reply->errno_val = (ret < 0) ? errno : 0;
        break;
    }

    case BSD_SYSCALL_STAT: {
        const char *path = (const char *)(uintptr_t)req->arg1;
        int ret = bsd_fd_stat(proc, path, (void *)(uintptr_t)req->arg2);
        reply->ret_code = ret;
        reply->errno_val = (ret < 0) ? errno : 0;
        break;
    }

    case BSD_SYSCALL_FSTAT: {
        int ret = bsd_fd_fstat(proc, req->arg1, (void *)(uintptr_t)req->arg2);
        reply->ret_code = ret;
        reply->errno_val = (ret < 0) ? errno : 0;
        break;
    }

    case BSD_SYSCALL_ACCESS: {
        const char *path = (const char *)(uintptr_t)req->arg1;
        int ret = bsd_fd_access(proc, path, req->arg2);
        reply->ret_code = ret;
        reply->errno_val = (ret < 0) ? errno : 0;
        break;
    }

    case BSD_SYSCALL_CHDIR: {
        const char *path = (const char *)(uintptr_t)req->arg1;
        int ret = bsd_fd_chdir(proc, path);
        reply->ret_code = ret;
        reply->errno_val = (ret < 0) ? errno : 0;
        break;
    }

    case BSD_SYSCALL_GETCWD: {
        char *buf = (char *)(uintptr_t)req->arg1;
        size_t size = (size_t)req->arg2;
        int ret = bsd_fd_getcwd(proc, buf, size);
        reply->ret_code = ret;
        reply->errno_val = (ret < 0) ? errno : 0;
        break;
    }

    case BSD_SYSCALL_CHMOD: {
        const char *path = (const char *)(uintptr_t)req->arg1;
        int ret = bsd_fd_chmod(proc, path, (mode_t)req->arg2);
        reply->ret_code = ret;
        reply->errno_val = (ret < 0) ? errno : 0;
        break;
    }

    case BSD_SYSCALL_UNLINK: {
        const char *path = (const char *)(uintptr_t)req->arg1;
        int ret = bsd_fd_unlink(proc, path);
        reply->ret_code = ret;
        reply->errno_val = (ret < 0) ? errno : 0;
        break;
    }

    case BSD_SYSCALL_RENAME: {
        const char *old = (const char *)(uintptr_t)req->arg1;
        const char *new_path = (const char *)(uintptr_t)req->arg2;
        int ret = bsd_fd_rename(proc, old, new_path);
        reply->ret_code = ret;
        reply->errno_val = (ret < 0) ? errno : 0;
        break;
    }

    case BSD_SYSCALL_MKDIR: {
        const char *path = (const char *)(uintptr_t)req->arg1;
        int ret = bsd_fd_mkdir(proc, path, (mode_t)req->arg2);
        reply->ret_code = ret;
        reply->errno_val = (ret < 0) ? errno : 0;
        break;
    }

    case BSD_SYSCALL_RMDIR: {
        const char *path = (const char *)(uintptr_t)req->arg1;
        int ret = bsd_fd_rmdir(proc, path);
        reply->ret_code = ret;
        reply->errno_val = (ret < 0) ? errno : 0;
        break;
    }

    case BSD_SYSCALL_SELECT: {
        int ret = bsd_fd_select(proc, req->arg1, (fd_set *)(uintptr_t)req->arg2,
                                (fd_set *)(uintptr_t)req->arg3,
                                (fd_set *)(uintptr_t)req->arg4,
                                (struct timeval *)(uintptr_t)req->arg5);
        reply->ret_code = ret;
        reply->errno_val = (ret < 0) ? errno : 0;
        break;
    }

    /* -- Signals ---------------------------------------- */

    case BSD_SYSCALL_KILL: {
        int ret = bsd_signal_kill((pid_t)req->arg1, (int)req->arg2);
        reply->ret_code = ret;
        reply->errno_val = (ret < 0) ? errno : 0;
        break;
    }

    /* -- Memory ----------------------------------------- */

    case BSD_SYSCALL_BRK: {
        void *result = bsd_memory_brk(proc, (void *)(uintptr_t)req->arg1);
        reply->ret_code = (int32_t)(uintptr_t)result;
        break;
    }

    case BSD_SYSCALL_SBRK: {
        void *result = bsd_memory_sbrk(proc, req->arg1);
        reply->ret_code = (int32_t)(uintptr_t)result;
        break;
    }

    case BSD_SYSCALL_MMAP: {
        void *result = bsd_memory_mmap(proc, (void *)(uintptr_t)req->arg1,
                                        (size_t)req->arg2, (int)req->arg3,
                                        (int)req->arg4, req->arg5,
                                        (off_t)req->arg6);
        reply->ret_code = (int32_t)(uintptr_t)result;
        reply->errno_val = (result == MAP_FAILED) ? errno : 0;
        break;
    }

    case BSD_SYSCALL_MUNMAP: {
        int ret = bsd_memory_munmap(proc, (void *)(uintptr_t)req->arg1,
                                     (size_t)req->arg2);
        reply->ret_code = ret;
        reply->errno_val = (ret < 0) ? errno : 0;
        break;
    }

    /* -- Time ------------------------------------------- */

    case BSD_SYSCALL_GETTIMEOFDAY: {
        struct timeval *tv = (struct timeval *)(uintptr_t)req->arg1;
        if (tv) {
            reply->ret_code = gettimeofday(tv, NULL);
            reply->errno_val = (reply->ret_code < 0) ? errno : 0;
        } else {
            reply->ret_code = -1;
            reply->errno_val = EINVAL;
        }
        break;
    }

    case BSD_SYSCALL_GETRUSAGE: {
        struct rusage *rusage = (struct rusage *)(uintptr_t)req->arg2;
        if (rusage) {
            reply->ret_code = getrusage((int)req->arg1, rusage);
            reply->errno_val = (reply->ret_code < 0) ? errno : 0;
        } else {
            reply->ret_code = -1;
            reply->errno_val = EINVAL;
        }
        break;
    }

    /* -- Unknown ---------------------------------------- */

    default:
        fprintf(stderr, "[bsd] unknown syscall %d from pid %d\n",
                req->syscall_num, req->caller_pid);
        reply->ret_code = -1;
        reply->errno_val = ENOSYS;
        return KERN_INVALID_ARGUMENT;
    }

    return KERN_SUCCESS;
}