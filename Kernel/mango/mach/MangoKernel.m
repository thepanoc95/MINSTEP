#objc
/*
 * mango/mach/MangoKernel.m
 *
 * Objective-C implementation of the Mango nanokernel lifecycle.
 */

#import "MangoKernel.h"
#import "../ipc/ipc.h"
#import "../task/task.h"
#import "../mach/mach_port.h"
#import "../loader/mach_loader.h"
#import "klog.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <poll.h>
#include <sys/wait.h>
#include <termios.h>
#include <errno.h>

@implementation MangoKernel

- (id)init {
    self = [super init];
    if (self) {
        _initialized = NO;
        _running = NO;
        _boot_flags = 0;
        _kernel_pid = getpid();
        _init_pid = -1;
        _host_port = MACH_PORT_NULL;
        _host_priv_port = MACH_PORT_NULL;
        _kernel_port = MACH_PORT_NULL;
        _userfs_root[0] = '\0';
        _have_saved_term = (tcgetattr(STDIN_FILENO, &_saved_term) == 0);
    }
    return self;
}

- (id)free {
    return [super free];
}

- (void)banner {
    klog_info("Mango Nanokernel %s\n", MANGO_KERNEL_VERSION);
    klog_info("MINSTEP Operating System\n");
    klog_info("Copyright (c) 2026 Miguel V. Mesquita\n");
    klog_info("Licensed under the BSD License\n");
}

- (kern_return_t)start:(uint32_t)flags {
    kern_return_t kr;

    _boot_flags = flags;
    klog_init_clock();

    [self banner];

    if (flags & MANGO_BOOT_VERBOSE) {
        klog_info("verbose boot requested\n");
    }

    klog_info("initializing IPC...\n");
    kr = ipc_init();
    if (kr != KERN_SUCCESS) {
        klog_err("IPC init failed (%d)\n", kr);
        return kr;
    }

    klog_info("initializing tasks...\n");
    kr = mango_task_init();
    if (kr != KERN_SUCCESS) {
        klog_err("task init failed (%d)\n", kr);
        return kr;
    }

    klog_info("initializing loader...\n");
    kr = mango_loader_init();
    if (kr != KERN_SUCCESS) {
        klog_err("loader init failed (%d)\n", kr);
        return kr;
    }

    signal(SIGTERM, SIG_DFL);
    signal(SIGINT, SIG_DFL);
    signal(SIGCHLD, SIG_IGN);

    _initialized = YES;
    _host_port = mach_port_allocate(MACH_PORT_RIGHT_RECEIVE);
    _host_priv_port = mach_port_allocate(MACH_PORT_RIGHT_RECEIVE);

    klog_info("subsystem initialization complete\n");
    return KERN_SUCCESS;
}

- (void)loop {
    _running = YES;

    klog_info("entering IPC dispatch loop\n");

    while (_running) {
        int status;
        pid_t pid;
        while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
            for (int i = 0; i < TASK_MAX; i++) {
                mango_task_t *task = &mango_task_table[i];
                if (task->in_use && task->host_pid == pid) {
                    klog_notice("task %s (pid %d) exited with status %d\n",
                                task->name, pid, WEXITSTATUS(status));
                    task->running = NO;
                    task->terminated = YES;
                    break;
                }
            }
        }

        mach_port_object_t *bp = mach_port_lookup(ipc_bootstrap_port);
        if (bp && bp->queue_count > 0) {
            mach_msg_t *msg = mach_port_dequeue_message(ipc_bootstrap_port);
            if (msg) {
                mach_msg_t reply;
                memset(&reply, 0, sizeof(reply));
                reply.header.msgh_size = sizeof(reply);

                bootstrap_handle_request(msg, &reply);

                if (msg->header.msgh_local_port != MACH_PORT_NULL) {
                    mach_msg_send(msg->header.msgh_local_port,
                                  &reply, reply.header.msgh_size, 1000);
                }

                free(msg);
            }
        }

        usleep(1000);
    }

    klog_info("dispatch loop exiting\n");
}

- (void)shutdown {
    klog_info("shutting down...\n");

    for (int i = TASK_MAX - 1; i >= 0; i--) {
        mango_task_t *task = &mango_task_table[i];
        if (task->in_use && task->id != 0) {
            mango_task_terminate(task);
        }
    }

    ipc_shutdown();

    _running = NO;
    _initialized = NO;

    if (_have_saved_term) {
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &_saved_term);
    }

    klog_info("shutdown complete\n");
}

- (const char *)userfsRoot {
    const char *env = getenv("USERFSROOT");
    if (env && env[0]) return env;

    if (_userfs_root[0]) {
        return _userfs_root;
    }

    return "/usr/local/minstep";
}

- (void)setUserfsRoot:(const char *)path {
    if (path) {
        /* Hardcode size to avoid preprocessor bug with sizeof(ivars) */
        strncpy(_userfs_root, path, 511);
        _userfs_root[511] = '\0';
    }
}

- (kern_return_t)launchInit:(const char *)root {
    if (!root) return KERN_INVALID_ARGUMENT;

    char init_path[1024];
    snprintf(init_path, sizeof(init_path), "%s/private/init", root);

    klog_info("searching for init: %s\n", init_path);

    if (access(init_path, X_OK) != 0) {
        klog_warn("%s not found or not executable\n", init_path);
        klog_notice("entering single-user mode\n");
        return KERN_FAILURE;
    }

    mango_task_t *init_task = NULL;
    kern_return_t kr = mango_task_create(&init_task, "init");
    if (kr != KERN_SUCCESS) {
        klog_err("could not create init task\n");
        return kr;
    }

    strncpy(init_task->binary_path, init_path,
            sizeof(init_task->binary_path) - 1);
    strncpy(init_task->userfs_root, root,
            sizeof(init_task->userfs_root) - 1);

    pid_t pid = fork();
    if (pid < 0) {
        klog_err("fork failed: %s\n", strerror(errno));
        mango_task_terminate(init_task);
        return KERN_FAILURE;
    }

    if (pid == 0) {
        setenv("USERFSROOT", root, 1);
        char port_str[32];
        snprintf(port_str, sizeof(port_str), "%d", init_task->bootstrap_port);
        setenv("MANGO_BOOTSTRAP_PORT", port_str, 1);
        execl(init_path, init_path, (char *)NULL);
        klog_err("exec init failed: %s\n", strerror(errno));
        /* Use function pointer cast to avoid preprocessor translating _exit to self->_exit */
        ((void(*)(int))_exit)(1);
    }

    init_task->host_pid = pid;
    init_task->running = YES;
    _init_pid = pid;

    klog_info("init launched (pid %d, task id %d)\n", pid, init_task->id);

    return KERN_SUCCESS;
}

@end

/* -----------------------------------------------------------------------
 *  Global kernel instance and C entry point
 * ----------------------------------------------------------------------- */

static MangoKernel *_shared_kernel = nil;

void mango_kernel_main(void)
{
    _shared_kernel = [MangoKernel new];

    uint32_t flags = 0;
    const char *v = getenv("MANGO_BOOT_VERBOSE");
    if (v && v[0]) flags |= MANGO_BOOT_VERBOSE;

    const char *s = getenv("MANGO_BOOT_SINGLE_USER");
    if (s && s[0]) flags |= MANGO_BOOT_SINGLE_USER;

    const char *userfs = getenv("USERFSROOT");
    if (userfs && userfs[0]) {
        [_shared_kernel setUserfsRoot:userfs];
    }

    kern_return_t kr = [_shared_kernel start:flags];
    if (kr != KERN_SUCCESS) {
        klog_err("FATAL: initialization failed (%d)\n", kr);
        return;
    }

    if (!(flags & MANGO_BOOT_SINGLE_USER)) {
        const char *root = [_shared_kernel userfsRoot];
        kr = [_shared_kernel launchInit:root];
        if (kr != KERN_SUCCESS) {
            klog_warn("init launch failed (%d)\n", kr);
        }
    }

    [_shared_kernel loop];
    [_shared_kernel shutdown];
    [_shared_kernel free];
}
