/*
 * @BSD_LICENSE_HEADER BEGIN
 * Copyright (c) 2026, thepanoc95 All rights reserved.

  * Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
  *  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  *  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  *  * All advertising materials mentioning features or use of this software must display the following acknowledgement: This product includes software developed by thepanoc95.
  *  * Neither the name of thepanoc95 nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THEPANOC95 AS IS AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THEPANOC95 BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * THIS SOFTWARE IS PROVIDED BY THEPANOC95 AS IS AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THEPANOC95 BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.[6]
 *
 * @BSD_LICENSE_HEADER END
 */

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
#include <sys/stat.h>

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
    klog_info("MINSTEP v%s -- Mango Nanokernel\n", MANGO_KERNEL_VERSION);
    klog_info("Copyright (c) 2026 Miguel V. Mesquita. BSD License.\n");
}

- (kern_return_t)start:(uint32_t)flags {
    kern_return_t kr;

    _boot_flags = flags;
    klog_init_clock();

    [self banner];

    if (flags & MANGO_BOOT_VERBOSE)
        klog_info("[boot] flags: verbose\n");
    else
        klog_info("[boot] flags: 0x%02x\n", (unsigned)flags);
    klog_info("[boot] kernel: pid %d, uid 0\n", _kernel_pid);

    klog_info("[ipc] initializing bootstrap port\n");
    kr = ipc_init();
    if (kr != KERN_SUCCESS) {
        klog_err("[ipc] init failed (%d)\n", kr);
        return kr;
    }

    klog_info("[task] initializing task table\n");
    kr = mango_task_init();
    if (kr != KERN_SUCCESS) {
        klog_err("[task] init failed (%d)\n", kr);
        return kr;
    }

    klog_info("[loader] initializing binary loader\n");
    kr = mango_loader_init();
    if (kr != KERN_SUCCESS) {
        klog_err("[loader] init failed (%d)\n", kr);
        return kr;
    }

    signal(SIGTERM, SIG_DFL);
    signal(SIGINT, SIG_DFL);
    signal(SIGCHLD, SIG_IGN);

    _initialized = YES;
    _host_port = mach_port_allocate(MACH_PORT_RIGHT_RECEIVE);
    _host_priv_port = mach_port_allocate(MACH_PORT_RIGHT_RECEIVE);
    klog_info("[mach] host port %d, host privilege port %d\n",
              _host_port, _host_priv_port);

    klog_info("kernel initialization complete\n");
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
                    klog_notice("[task] %s (pid %d) exited with status %d\n",
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

    return "/";
}

- (void)setUserfsRoot:(const char *)path {
    if (path) {
        strncpy(_userfs_root, path, 511);
        _userfs_root[511] = '\0';
    }
}

- (kern_return_t)launchInit:(const char *)root initPath:(const char *)init_path_arg {
    if (!root) return KERN_INVALID_ARGUMENT;

    char init_path[1024];

    if (init_path_arg && init_path_arg[0]) {
        strncpy(init_path, init_path_arg, sizeof(init_path) - 1);
        init_path[sizeof(init_path) - 1] = '\0';
        klog_info("[init] using init: %s\n", init_path);
    } else {
        snprintf(init_path, sizeof(init_path), "%s/private/init", root);
        klog_info("[init] searching for init: %s (root=%s)\n", init_path, root);
    }

    if (access(init_path, F_OK) != 0) {
        klog_warn("[init] %s does not exist (errno=%d: %s)\n",
                  init_path, errno, strerror(errno));
        klog_info("[boot] entering single-user mode\n");
        return KERN_FAILURE;
    }

    if (access(init_path, X_OK) != 0) {
        klog_warn("[init] %s not executable (errno=%d: %s)\n",
                  init_path, errno, strerror(errno));
        klog_info("[boot] entering single-user mode\n");
        return KERN_FAILURE;
    }

    mango_task_t *init_task = NULL;
    kern_return_t kr = mango_task_create(&init_task, "init");
    if (kr != KERN_SUCCESS) {
        klog_err("[init] could not create init task (%d)\n", kr);
        return kr;
    }

    strncpy(init_task->binary_path, init_path,
            sizeof(init_task->binary_path) - 1);
    strncpy(init_task->userfs_root, root,
            sizeof(init_task->userfs_root) - 1);

    pid_t pid = fork();
    if (pid < 0) {
        klog_err("[init] fork failed: %s\n", strerror(errno));
        mango_task_terminate(init_task);
        return KERN_FAILURE;
    }

    if (pid == 0) {
        setenv("USERFSROOT", root, 1);
        char port_str[32];
        snprintf(port_str, sizeof(port_str), "%d", init_task->bootstrap_port);
        setenv("MANGO_BOOTSTRAP_PORT", port_str, 1);
        execl(init_path, init_path, (char *)NULL);
        klog_err("[init] exec failed: %s\n", strerror(errno));
        ((void(*)(int))_exit)(1);
    }

    init_task->host_pid = pid;
    init_task->running = YES;
    _init_pid = pid;

    klog_info("[init] launched (pid %d, task id %d)\n", pid, init_task->id);

    return KERN_SUCCESS;
}

@end

/* -----------------------------------------------------------------------
 *  Global kernel instance and C entry point
 * ----------------------------------------------------------------------- */

static MangoKernel *_shared_kernel = nil;

void mango_kernel_main(const char *init_path)
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
        klog_err("FATAL: kernel initialization failed (%d)\n", kr);
        return;
    }

    if (!(flags & MANGO_BOOT_SINGLE_USER)) {
        const char *root = objc_msgSend(_shared_kernel, sel_registerName("userfsRoot"));
        if (!root) root = "/";

        char resolved[1024];
        if (init_path && init_path[0]) {
            strncpy(resolved, init_path, sizeof(resolved) - 1);
            resolved[sizeof(resolved) - 1] = '\0';
            klog_info("[init] using init: %s\n", resolved);
        } else {
            snprintf(resolved, sizeof(resolved), "%s/private/init", root);
            klog_info("[init] searching for init: %s\n", resolved);
            if (access(resolved, X_OK) != 0) {
                klog_warn("[init] %s not executable, falling back to /sbin/init\n", resolved);
                strncpy(resolved, "/sbin/init", sizeof(resolved) - 1);
            }
        }

        pid_t pid = fork();
        if (pid < 0) {
            klog_err("[init] fork failed: %s\n", strerror(errno));
        } else if (pid == 0) {
            setenv("USERFSROOT", root, 1);
            execl(resolved, resolved, (char *)NULL);
            klog_err("[init] exec failed: %s\n", strerror(errno));
            ((void(*)(int))_exit)(1);
        } else {
            _shared_kernel->_init_pid = pid;
            klog_info("[init] launched (pid %d)\n", pid);
        }
    } else {
        klog_info("[boot] single-user mode, skipping init\n");
    }

    [_shared_kernel loop];
    [_shared_kernel shutdown];
    [_shared_kernel free];
}
