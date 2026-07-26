/*
 * mango/mach/mach_kernel.c
 *
 * Main kernel entry point and lifecycle management for the
 * Mango nanokernel.
 */

#include "mach_kernel.h"
#include "klog.h"
#include "../ipc/ipc.h"
#include "../task/task.h"
#include "../mach/mach_port.h"
#include "../loader/mach_loader.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <poll.h>
#include <sys/wait.h>
#include <termios.h>
#include <errno.h>

mango_kernel_state_t _mango_kernel;

static volatile int _mango_running = 1;

static void _mango_signal_handler(int sig)
{
    (void)sig;
    _mango_running = 0;
}

void mango_kernel_banner(void)
{
    klog_info("Mango Nanokernel %s\n", MANGO_KERNEL_VERSION);
    klog_info("MinSTEP Operating Environment\n");
    klog_info("Copyright (c) 2026 Miguel V. Mesquita\n");
    klog_info("Licensed under the BSD License\n");
}

kern_return_t mango_kernel_init(uint32_t boot_flags)
{
    kern_return_t kr;

    /* Initialize the kernel clock first */
    klog_init_clock();

    memset(&_mango_kernel, 0, sizeof(_mango_kernel));
    _mango_kernel.boot_flags = boot_flags;
    _mango_kernel.kernel_pid = getpid();

    mango_kernel_banner();

    if (boot_flags & MANGO_BOOT_VERBOSE) {
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

    signal(SIGTERM, _mango_signal_handler);
    signal(SIGINT, _mango_signal_handler);
    signal(SIGCHLD, SIG_IGN);

    _mango_kernel.initialized = TRUE;
    _mango_kernel.host_port = mach_port_allocate(MACH_PORT_RIGHT_RECEIVE);
    _mango_kernel.host_priv_port = mach_port_allocate(MACH_PORT_RIGHT_RECEIVE);

    klog_info("subsystem initialization complete\n");
    return KERN_SUCCESS;
}

void mango_kernel_loop(void)
{
    _mango_kernel.running = TRUE;
    _mango_running = 1;

    klog_info("entering IPC dispatch loop\n");

    while (_mango_running) {
        int status;
        pid_t pid;
        while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
            for (int i = 0; i < TASK_MAX; i++) {
                mango_task_t *task = &mango_task_table[i];
                if (task->in_use && task->host_pid == pid) {
                    klog_notice("task %s (pid %d) exited with status %d\n",
                                task->name, pid, WEXITSTATUS(status));
                    task->running = FALSE;
                    task->terminated = TRUE;
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

void mango_kernel_shutdown(void)
{
    klog_info("shutting down...\n");

    for (int i = TASK_MAX - 1; i >= 0; i--) {
        mango_task_t *task = &mango_task_table[i];
        if (task->in_use && task->id != 0) {
            mango_task_terminate(task);
        }
    }

    ipc_shutdown();

    _mango_kernel.running = FALSE;
    _mango_kernel.initialized = FALSE;

    klog_info("shutdown complete\n");
}

const char *mango_get_userfs_root(void)
{
    const char *env = getenv("USERFSROOT");
    if (env && env[0]) return env;

    if (_mango_kernel.userfs_root[0]) {
        return _mango_kernel.userfs_root;
    }

    return "/";
}

void mango_set_userfs_root(const char *path)
{
    if (path) {
        strncpy(_mango_kernel.userfs_root, path,
                sizeof(_mango_kernel.userfs_root) - 1);
        _mango_kernel.userfs_root[sizeof(_mango_kernel.userfs_root) - 1] = '\0';
    }
}

void mango_kernel_main(void)
{
    struct termios saved_term;
    int have_saved_term = (tcgetattr(STDIN_FILENO, &saved_term) == 0);

    uint32_t flags = 0;

    const char *v = getenv("MANGO_BOOT_VERBOSE");
    if (v && v[0]) flags |= MANGO_BOOT_VERBOSE;

    const char *s = getenv("MANGO_BOOT_SINGLE_USER");
    if (s && s[0]) flags |= MANGO_BOOT_SINGLE_USER;

    const char *userfs = getenv("USERFSROOT");
    if (userfs && userfs[0]) {
        mango_set_userfs_root(userfs);
    }

    kern_return_t kr = mango_kernel_init(flags);
    if (kr != KERN_SUCCESS) {
        klog_err("FATAL: initialization failed (%d)\n", kr);
        return;
    }

    if (!(flags & MANGO_BOOT_SINGLE_USER)) {
        kr = mango_launch_init(mango_get_userfs_root());
        if (kr != KERN_SUCCESS) {
            klog_warn("init launch failed (%d)\n", kr);
            klog_notice("entering single-user mode\n");
        }
    }

    mango_kernel_loop();
    mango_kernel_shutdown();

    if (have_saved_term) {
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &saved_term);
    }
}
