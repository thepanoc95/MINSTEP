/*
 * Developer/BSD/bsd_process.c
 *
 * Process management for the BSD server.
 * Maintains a process table mapping Mach tasks to POSIX processes.
 * Implements fork, exec, exit, wait, and process query operations.
 */

#include "bsd.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>

/* -----------------------------------------------------------------------
 *  Process table initialization
 * ----------------------------------------------------------------------- */

static int _bsd_process_count = 0;

kern_return_t bsd_process_init(void)
{
    memset(bsd_server.process_table, 0, sizeof(bsd_server.process_table));
    bsd_server.process_count = 0;
    _bsd_process_count = 0;

    /* Initialize the kernel process entry */
    bsd_process_t *proc = &bsd_server.process_table[0];
    proc->pid = getpid();
    proc->ppid = 0;
    proc->pgid = getpid();
    proc->sid = getsid(0);
    proc->in_use = YES;
    proc->running = YES;
    proc->zombie = NO;
    proc->stopped = NO;
    proc->uid = getuid();
    proc->euid = geteuid();
    proc->gid = getgid();
    proc->egid = getegid();
    proc->exit_status = 0;
    proc->exit_signal = 0;
    proc->task_port = MACH_PORT_NULL;
    proc->bootstrap_port = MACH_PORT_NULL;
    proc->fd_count = 0;
    proc->argc = 0;
    proc->envc = 0;
    proc->brk_addr = sbrk(0);
    proc->mmap_base = NULL;
    proc->mmap_size = 0;
    proc->signal_pending = 0;
    proc->signal_mask = 0;
    proc->signal_actions = 0;

    (void)getcwd(proc->cwd, BSD_MAX_CWD - 1);

    bsd_fd_init(proc);
    bsd_signal_init(proc);
    bsd_memory_init(proc);

    _bsd_process_count = 1;
    bsd_server.process_count = 1;

    fprintf(stderr, "[bsd] process table initialized (kernel pid %d)\n",
            proc->pid);

    return KERN_SUCCESS;
}

/* -----------------------------------------------------------------------
 *  Process allocation and lookup
 * ----------------------------------------------------------------------- */

kern_return_t bsd_process_alloc(pid_t pid, bsd_process_t **out)
{
    if (_bsd_process_count >= BSD_MAX_PROCESSES) return KERN_NO_SPACE;

    for (int i = 0; i < BSD_MAX_PROCESSES; i++) {
        if (!bsd_server.process_table[i].in_use) {
            bsd_process_t *proc = &bsd_server.process_table[i];
            memset(proc, 0, sizeof(bsd_process_t));
            proc->pid = pid;
            proc->ppid = getpid();
            proc->in_use = YES;
            proc->running = YES;
            proc->zombie = NO;
            proc->stopped = NO;
            proc->uid = getuid();
            proc->euid = geteuid();
            proc->gid = getgid();
            proc->egid = getegid();
            proc->fd_count = 0;
            proc->argc = 0;
            proc->envc = 0;
            proc->signal_pending = 0;
            proc->signal_mask = 0;
            proc->signal_actions = 0;

            (void)getcwd(proc->cwd, BSD_MAX_CWD - 1);

            proc->task_port = mach_port_allocate(MACH_PORT_RIGHT_RECEIVE);
            proc->bootstrap_port = mach_port_allocate(MACH_PORT_RIGHT_RECEIVE);

            bsd_fd_init(proc);
            bsd_signal_init(proc);
            bsd_memory_init(proc);

            _bsd_process_count++;
            bsd_server.process_count = _bsd_process_count;

            *out = proc;
            return KERN_SUCCESS;
        }
    }

    return KERN_NO_SPACE;
}

bsd_process_t *bsd_process_lookup(pid_t pid)
{
    for (int i = 0; i < BSD_MAX_PROCESSES; i++) {
        if (bsd_server.process_table[i].in_use &&
            bsd_server.process_table[i].pid == pid) {
            return &bsd_server.process_table[i];
        }
    }
    return NULL;
}

void bsd_process_free(bsd_process_t *proc)
{
    if (!proc || !proc->in_use) return;

    for (int i = 0; i < BSD_MAX_FD; i++) {
        if (proc->fd_table[i].in_use) {
            close(proc->fd_table[i].host_fd);
            proc->fd_table[i].in_use = NO;
        }
    }

    for (int i = 0; i < proc->argc; i++) {
        if (proc->argv[i]) free(proc->argv[i]);
    }
    proc->argc = 0;

    for (int i = 0; i < proc->envc; i++) {
        if (proc->envp[i]) free(proc->envp[i]);
    }
    proc->envc = 0;

    proc->in_use = NO;
    proc->running = NO;

    _bsd_process_count--;
    bsd_server.process_count = _bsd_process_count;
}

/* -----------------------------------------------------------------------
 *  Reap zombie child processes
 * ----------------------------------------------------------------------- */

kern_return_t bsd_process_reap(void)
{
    int status;
    pid_t pid;

    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        bsd_process_t *proc = bsd_process_lookup(pid);
        if (proc) {
            proc->exit_status = WEXITSTATUS(status);
            proc->running = NO;
            proc->zombie = YES;

            if (proc->ppid > 0) {
                bsd_signal_kill(proc->ppid, SIGCHLD);
            }
        }
    }

    return KERN_SUCCESS;
}

/* -----------------------------------------------------------------------
 *  System call implementations
 * ----------------------------------------------------------------------- */

kern_return_t bsd_syscall_exit(bsd_process_t *proc, int status)
{
    if (!proc || !proc->in_use) return KERN_INVALID_TASK;

    proc->exit_status = status;
    proc->running = NO;
    proc->zombie = YES;

    for (int i = 0; i < BSD_MAX_FD; i++) {
        if (proc->fd_table[i].in_use) {
            close(proc->fd_table[i].host_fd);
            proc->fd_table[i].in_use = NO;
        }
    }

    if (proc->ppid > 0) {
        bsd_signal_kill(proc->ppid, SIGCHLD);
    }

    fprintf(stderr, "[bsd] process %d exited with status %d\n",
            proc->pid, status);

    return KERN_SUCCESS;
}

kern_return_t bsd_syscall_fork(bsd_process_t *proc, bsd_syscall_reply_t *reply)
{
    if (!proc || !reply) return KERN_INVALID_ARGUMENT;

    pid_t child_pid = fork();
    if (child_pid < 0) {
        reply->ret_code = -1;
        reply->errno_val = errno;
        return KERN_FAILURE;
    }

    if (child_pid == 0) {
        /* Child process */
        reply->ret_code = 0;
        reply->out_arg1 = getpid();
        return KERN_SUCCESS;
    }

    /* Parent process - create process entry */
    bsd_process_t *child;
    kern_return_t kr = bsd_process_alloc(child_pid, &child);
    if (kr == KERN_SUCCESS) {
        child->ppid = proc->pid;
        child->pgid = proc->pgid;
        child->sid = proc->sid;
        child->uid = proc->uid;
        child->euid = proc->euid;
        child->gid = proc->gid;
        child->egid = proc->egid;
        strncpy(child->cwd, proc->cwd, BSD_MAX_CWD - 1);
    }

    reply->ret_code = child_pid;
    reply->out_arg1 = child_pid;
    return KERN_SUCCESS;
}

kern_return_t bsd_syscall_exec(bsd_process_t *proc, const char *path,
                               char *const argv[], char *const envp[])
{
    if (!proc || !path) return KERN_INVALID_ARGUMENT;

    for (int i = 0; i < BSD_MAX_FD; i++) {
        if (proc->fd_table[i].in_use && (proc->fd_table[i].flags & FD_CLOEXEC)) {
            close(proc->fd_table[i].host_fd);
            proc->fd_table[i].in_use = NO;
        }
    }

    for (int i = 0; i < proc->argc; i++) {
        if (proc->argv[i]) free(proc->argv[i]);
    }
    proc->argc = 0;

    if (argv) {
        int i = 0;
        while (argv[i] && i < BSD_MAX_ARGS - 1) {
            proc->argv[i] = strdup(argv[i]);
            i++;
        }
        proc->argc = i;
        proc->argv[i] = NULL;
    }

    for (int i = 0; i < proc->envc; i++) {
        if (proc->envp[i]) free(proc->envp[i]);
    }
    proc->envc = 0;

    if (envp) {
        int i = 0;
        while (envp[i] && i < BSD_MAX_ENV - 1) {
            proc->envp[i] = strdup(envp[i]);
            i++;
        }
        proc->envc = i;
        proc->envp[i] = NULL;
    }

    if (access(path, X_OK) != 0) {
        fprintf(stderr, "[bsd] exec: %s: %s\n", path, strerror(errno));
        return KERN_FAILURE;
    }

    fprintf(stderr, "[bsd] process %d exec: %s\n", proc->pid, path);

    return KERN_SUCCESS;
}

kern_return_t bsd_syscall_waitpid(bsd_process_t *proc, int wait_pid,
                                  int *status, int options,
                                  bsd_syscall_reply_t *reply)
{
    if (!proc || !reply) return KERN_INVALID_ARGUMENT;

    int found_pid = 0;
    int exit_status = 0;

    if (wait_pid == -1) {
        for (int i = 0; i < BSD_MAX_PROCESSES; i++) {
            bsd_process_t *child = &bsd_server.process_table[i];
            if (child->in_use && child->ppid == proc->pid && child->zombie) {
                found_pid = child->pid;
                exit_status = child->exit_status;
                bsd_process_free(child);
                break;
            }
        }
    } else {
        bsd_process_t *target = bsd_process_lookup(wait_pid);
        if (target && target->ppid == proc->pid && target->zombie) {
            found_pid = target->pid;
            exit_status = target->exit_status;
            bsd_process_free(target);
        }
    }

    if (found_pid == 0) {
        if (options & WNOHANG) {
            reply->ret_code = 0;
        } else {
            reply->ret_code = -1;
            reply->errno_val = ECHILD;
        }
    } else {
        if (status) *status = exit_status;
        reply->ret_code = found_pid;
        reply->out_arg1 = exit_status;
    }

    return KERN_SUCCESS;
}

kern_return_t bsd_syscall_getpid(bsd_process_t *proc, bsd_syscall_reply_t *reply)
{
    if (!proc || !reply) return KERN_INVALID_ARGUMENT;
    reply->ret_code = proc->pid;
    return KERN_SUCCESS;
}

kern_return_t bsd_syscall_getppid(bsd_process_t *proc, bsd_syscall_reply_t *reply)
{
    if (!proc || !reply) return KERN_INVALID_ARGUMENT;
    reply->ret_code = proc->ppid;
    return KERN_SUCCESS;
}

kern_return_t bsd_syscall_getuid(bsd_process_t *proc, bsd_syscall_reply_t *reply)
{
    if (!proc || !reply) return KERN_INVALID_ARGUMENT;
    reply->ret_code = proc->uid;
    return KERN_SUCCESS;
}

kern_return_t bsd_syscall_geteuid(bsd_process_t *proc, bsd_syscall_reply_t *reply)
{
    if (!proc || !reply) return KERN_INVALID_ARGUMENT;
    reply->ret_code = proc->euid;
    return KERN_SUCCESS;
}

kern_return_t bsd_syscall_getgid(bsd_process_t *proc, bsd_syscall_reply_t *reply)
{
    if (!proc || !reply) return KERN_INVALID_ARGUMENT;
    reply->ret_code = proc->gid;
    return KERN_SUCCESS;
}

kern_return_t bsd_syscall_getegid(bsd_process_t *proc, bsd_syscall_reply_t *reply)
{
    if (!proc || !reply) return KERN_INVALID_ARGUMENT;
    reply->ret_code = proc->egid;
    return KERN_SUCCESS;
}

kern_return_t bsd_syscall_setuid(bsd_process_t *proc, uid_t uid)
{
    if (!proc) return KERN_INVALID_TASK;
    proc->euid = uid;
    proc->uid = uid;
    return KERN_SUCCESS;
}

kern_return_t bsd_syscall_setgid(bsd_process_t *proc, gid_t gid)
{
    if (!proc) return KERN_INVALID_TASK;
    proc->egid = gid;
    proc->gid = gid;
    return KERN_SUCCESS;
}