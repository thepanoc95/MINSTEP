/*
 * Developer/BSD/bsd.h
 *
 * BSD subsystem for the Mango nanokernel.
 *
 * Provides POSIX/BSD system call services to Mach tasks via IPC.
 * The BSD server runs as a privileged user-space server that
 * translates Mach trap messages into host POSIX operations.
 *
 * Copyright (c) 2026 MinSTEP Project
 * Licensed under the MIT License.
 */

#ifndef _BSD_BSD_H_
#define _BSD_BSD_H_

#include <objc/objc.h>

#include "../../Kernel/mxwl/mach/mach_types.h"
#include "../../Kernel/mxwl/mach/mach_msg.h"
#include "../../Kernel/mxwl/mach/mach_port.h"
#include "../../Kernel/mxwl/ipc/ipc.h"

#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/mman.h>

/* ========================================================================
 *  FD flags (for compatibility)
 * ======================================================================== */

#ifndef FD_CLOEXEC
#define FD_CLOEXEC 1
#endif

/* ========================================================================
 *  BSD Server Version
 * ======================================================================== */

#define BSD_SERVER_VERSION  "0.1.0"
#define BSD_SERVER_NAME     "MinSTEP BSD"

/* ========================================================================
 *  Limits
 * ======================================================================== */

#define BSD_MAX_PROCESSES   128
#define BSD_MAX_THREADS     256
#define BSD_MAX_FD          256
#define BSD_MAX_SIGNALS     32
#define BSD_MAX_CWD         1024
#define BSD_MAX_ENV         256
#define BSD_MAX_ARGS        64

/* ========================================================================
 *  System Call Numbers
 * ======================================================================== */

#define BSD_SYSCALL_EXIT            1
#define BSD_SYSCALL_FORK            2
#define BSD_SYSCALL_EXEC            3
#define BSD_SYSCALL_WAIT            4
#define BSD_SYSCALL_WAITPID         5
#define BSD_SYSCALL_READ            6
#define BSD_SYSCALL_WRITE           7
#define BSD_SYSCALL_OPEN            8
#define BSD_SYSCALL_CLOSE           9
#define BSD_SYSCALL_LSEEK           10
#define BSD_SYSCALL_DUP             11
#define BSD_SYSCALL_DUP2            12
#define BSD_SYSCALL_PIPE            13
#define BSD_SYSCALL_IOCTL           14
#define BSD_SYSCALL_GETPID          15
#define BSD_SYSCALL_GETPPID         16
#define BSD_SYSCALL_GETUID          17
#define BSD_SYSCALL_GETEUID         18
#define BSD_SYSCALL_GETGID          19
#define BSD_SYSCALL_GETEGID         20
#define BSD_SYSCALL_SETUID          21
#define BSD_SYSCALL_SETGID          22
#define BSD_SYSCALL_KILL            23
#define BSD_SYSCALL_SIGACTION       24
#define BSD_SYSCALL_SIGPROCMASK     25
#define BSD_SYSCALL_BRK             26
#define BSD_SYSCALL_SBRK            27
#define BSD_SYSCALL_MMAP            28
#define BSD_SYSCALL_MUNMAP          29
#define BSD_SYSCALL_CHDIR           30
#define BSD_SYSCALL_GETCWD          31
#define BSD_SYSCALL_STAT            32
#define BSD_SYSCALL_FSTAT           33
#define BSD_SYSCALL_ACCESS          34
#define BSD_SYSCALL_FCNTL           35
#define BSD_SYSCALL_CHMOD           36
#define BSD_SYSCALL_UNLINK          37
#define BSD_SYSCALL_RENAME          38
#define BSD_SYSCALL_MKDIR           39
#define BSD_SYSCALL_RMDIR           40
#define BSD_SYSCALL_SELECT          41
#define BSD_SYSCALL_GETTIMEOFDAY    42
#define BSD_SYSCALL_GETRUSAGE       43
#define BSD_SYSCALL_GETRLIMIT       44
#define BSD_SYSCALL_SETRLIMIT       45

/* ========================================================================
 *  System Call RPC Protocol
 * ======================================================================== */

#define BSD_MSG_ID_SYSCALL      0x600
#define BSD_MSG_ID_SYSCALL_RET  0x601

/* System call request: sent by a task to the BSD server */
typedef struct bsd_syscall_request {
    mach_msg_header_t   header;
    int32_t             syscall_num;
    int32_t             arg1;
    int32_t             arg2;
    int32_t             arg3;
    int32_t             arg4;
    int32_t             arg5;
    int32_t             arg6;
    int32_t             caller_pid;     /* PID of calling process */
} bsd_syscall_request_t;

/* System call reply: sent back by the BSD server */
typedef struct bsd_syscall_reply {
    mach_msg_header_t   header;
    int32_t             ret_code;
    int32_t             errno_val;
    int32_t             out_arg1;
    int32_t             out_arg2;
} bsd_syscall_reply_t;

/* ========================================================================
 *  File Descriptor
 * ======================================================================== */

typedef struct bsd_fd_entry {
    int             host_fd;        /* Host OS file descriptor */
    int             flags;          /* FD flags (CLOEXEC, etc.) */
    int             access_mode;    /* O_RDONLY, O_WRONLY, O_RDWR */
    BOOL            in_use;
    char            path[BSD_MAX_CWD]; /* For fstat */
} bsd_fd_entry_t;

/* ========================================================================
 *  Process Entry
 * ======================================================================== */

typedef struct bsd_process {
    pid_t               pid;
    pid_t               ppid;
    pid_t               pgid;
    pid_t               sid;
    int                 exit_status;
    int                 exit_signal;
    BOOL                in_use;
    BOOL                running;
    BOOL                stopped;
    BOOL                zombie;
    mach_port_t         task_port;
    mach_port_t         bootstrap_port;
    uid_t               uid;
    uid_t               euid;
    gid_t               gid;
    gid_t               egid;
    char                cwd[BSD_MAX_CWD];
    char                *argv[BSD_MAX_ARGS];
    int                 argc;
    char                *envp[BSD_MAX_ENV];
    int                 envc;
    bsd_fd_entry_t      fd_table[BSD_MAX_FD];
    int                 fd_count;
    unsigned long       signal_actions;  /* Bitmask of signal handlers */
    unsigned long       signal_pending;
    unsigned long       signal_mask;
    void                *brk_addr;
    void                *mmap_base;
    size_t              mmap_size;
} bsd_process_t;

/* ========================================================================
 *  BSD Server State
 * ======================================================================== */

typedef struct bsd_server_state {
    BOOL                initialized;
    BOOL                running;
    mach_port_t         bsd_port;       /* Our receive port */
    mach_port_t         host_port;      /* Kernel host port */
    mach_port_t         host_priv_port; /* Kernel host privilege port */
    bsd_process_t       process_table[BSD_MAX_PROCESSES];
    int                 process_count;
    pid_t               next_pid;       /* Next PID to assign */
} bsd_server_state_t;

extern bsd_server_state_t bsd_server;

/* ========================================================================
 *  BSD IPC Functions
 * ======================================================================== */

kern_return_t bsd_ipc_init(void);
void bsd_ipc_shutdown(void);
kern_return_t bsd_ipc_send(mach_port_t dest, mach_msg_t *msg, mach_msg_size_t size);
kern_return_t bsd_ipc_receive(mach_port_t src, mach_msg_t *msg,
                                mach_msg_size_t size, int timeout);
kern_return_t bsd_ipc_register_service(const char *name);

/* ========================================================================
 *  BSD Process Functions
 * ======================================================================== */

kern_return_t bsd_process_init(void);
kern_return_t bsd_process_alloc(pid_t pid, bsd_process_t **out);
bsd_process_t *bsd_process_lookup(pid_t pid);
void bsd_process_free(bsd_process_t *proc);
kern_return_t bsd_process_reap(void);

kern_return_t bsd_syscall_exit(bsd_process_t *proc, int status);
kern_return_t bsd_syscall_fork(bsd_process_t *proc, bsd_syscall_reply_t *reply);
kern_return_t bsd_syscall_exec(bsd_process_t *proc, const char *path,
                               char *const argv[], char *const envp[]);
kern_return_t bsd_syscall_waitpid(bsd_process_t *proc, int pid,
                                  int *status, int options,
                                  bsd_syscall_reply_t *reply);
kern_return_t bsd_syscall_getpid(bsd_process_t *proc, bsd_syscall_reply_t *reply);
kern_return_t bsd_syscall_getppid(bsd_process_t *proc, bsd_syscall_reply_t *reply);
kern_return_t bsd_syscall_getuid(bsd_process_t *proc, bsd_syscall_reply_t *reply);
kern_return_t bsd_syscall_geteuid(bsd_process_t *proc, bsd_syscall_reply_t *reply);
kern_return_t bsd_syscall_getgid(bsd_process_t *proc, bsd_syscall_reply_t *reply);
kern_return_t bsd_syscall_getegid(bsd_process_t *proc, bsd_syscall_reply_t *reply);
kern_return_t bsd_syscall_setuid(bsd_process_t *proc, uid_t uid);
kern_return_t bsd_syscall_setgid(bsd_process_t *proc, gid_t gid);

/* ========================================================================
 *  BSD FD Functions
 * ======================================================================== */

kern_return_t bsd_fd_init(bsd_process_t *proc);
int bsd_fd_open(bsd_process_t *proc, const char *path, int flags, int mode);
int bsd_fd_close(bsd_process_t *proc, int fd);
int bsd_fd_read(bsd_process_t *proc, int fd, void *buf, size_t nbyte);
int bsd_fd_write(bsd_process_t *proc, int fd, const void *buf, size_t nbyte);
off_t bsd_fd_lseek(bsd_process_t *proc, int fd, off_t offset, int whence);
int bsd_fd_dup(bsd_process_t *proc, int oldfd);
int bsd_fd_dup2(bsd_process_t *proc, int oldfd, int newfd);
int bsd_fd_pipe(bsd_process_t *proc, int fds[2]);
int bsd_fd_ioctl(bsd_process_t *proc, int fd, unsigned long request, void *arg);
int bsd_fd_fcntl(bsd_process_t *proc, int fd, int cmd, int arg);
int bsd_fd_stat(bsd_process_t *proc, const char *path, void *buf);
int bsd_fd_fstat(bsd_process_t *proc, int fd, void *buf);
int bsd_fd_access(bsd_process_t *proc, const char *path, int mode);
int bsd_fd_chdir(bsd_process_t *proc, const char *path);
int bsd_fd_getcwd(bsd_process_t *proc, char *buf, size_t size);
int bsd_fd_chmod(bsd_process_t *proc, const char *path, mode_t mode);
int bsd_fd_unlink(bsd_process_t *proc, const char *path);
int bsd_fd_rename(bsd_process_t *proc, const char *old, const char *new_path);
int bsd_fd_mkdir(bsd_process_t *proc, const char *path, mode_t mode);
int bsd_fd_rmdir(bsd_process_t *proc, const char *path);
int bsd_fd_select(bsd_process_t *proc, int nfds, fd_set *readfds,
                  fd_set *writefds, fd_set *errorfds, struct timeval *timeout);

/* ========================================================================
 *  BSD Signal Functions
 * ======================================================================== */

kern_return_t bsd_signal_init(bsd_process_t *proc);
int bsd_signal_kill(pid_t pid, int sig);
void bsd_signal_deliver(bsd_process_t *proc);

/* ========================================================================
 *  BSD Memory Functions
 * ======================================================================== */

kern_return_t bsd_memory_init(bsd_process_t *proc);
void *bsd_memory_brk(bsd_process_t *proc, void *addr);
void *bsd_memory_sbrk(bsd_process_t *proc, int incr);
void *bsd_memory_mmap(bsd_process_t *proc, void *addr, size_t len,
                      int prot, int flags, int fd, off_t offset);
int bsd_memory_munmap(bsd_process_t *proc, void *addr, size_t len);

/* ========================================================================
 *  BSD Syscall Dispatch
 * ======================================================================== */

kern_return_t bsd_syscall_dispatch(bsd_syscall_request_t *req,
                                   bsd_syscall_reply_t *reply);

/* ========================================================================
 *  BSD Server Main Functions
 * ======================================================================== */

kern_return_t bsd_server_start(void);
void bsd_server_loop(void);
void bsd_server_shutdown(void);
void bsd_server_main(void);

#endif /* _BSD_BSD_H_ */