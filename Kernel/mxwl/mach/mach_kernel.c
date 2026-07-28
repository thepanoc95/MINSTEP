/*
 * mxwl/mach/mach_kernel.c
 *
 * Main kernel entry point and lifecycle management for the
 * Maxxwell nanokernel.
 *
 * Boot output follows NeXTSTEP Mach conventions -- plain
 * descriptive text with no timestamps.
 */

#include "mach_kernel.h"
#include "klog.h"
#include "../ipc/ipc.h"
#include "../task/task.h"
#include "../mach/mach_port.h"
#include "../loader/mach_loader.h"
#include "../kal/kal.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

mxwl_kernel_state_t _mxwl_kernel;

static volatile int _mxwl_running = 1;

static void _mxwl_signal_handler(int sig)
{
    (void)sig;
    _mxwl_running = 0;
}

/* -----------------------------------------------------------------------
 *  Banner -- printed once at boot before subsystem init
 * ----------------------------------------------------------------------- */

void mxwl_kernel_banner(void)
{
    klog_info("MINSTEP v%s -- Maxxwell Nanokernel\n", MXWL_KERNEL_VERSION);
    klog_info("Copyright (c) 2026 Miguel V. Mesquita. BSD License.\n");
}

/* -----------------------------------------------------------------------
 *  Kernel initialization
 * ----------------------------------------------------------------------- */

kern_return_t mxwl_kernel_init(uint32_t boot_flags)
{
    kern_return_t kr;

    klog_init_clock();

    memset(&_mxwl_kernel, 0, sizeof(_mxwl_kernel));
    _mxwl_kernel.boot_flags = boot_flags;
    _mxwl_kernel.kernel_pid = kal_getpid();

    mxwl_kernel_banner();

    if (boot_flags & MXWL_BOOT_VERBOSE)
        klog_info("verbose boot\n");
    else
        klog_info("boot flags: 0x%02x\n", (unsigned)boot_flags);

    /* IPC subsystem */
    kr = ipc_init();
    if (kr != KERN_SUCCESS) {
        klog_err("ipc init failed (%d)\n", kr);
        return kr;
    }

    /* Task subsystem */
    kr = mxwl_task_init();
    if (kr != KERN_SUCCESS) {
        klog_err("task init failed (%d)\n", kr);
        return kr;
    }

    /* Loader subsystem */
    kr = mxwl_loader_init();
    if (kr != KERN_SUCCESS) {
        klog_err("loader init failed (%d)\n", kr);
        return kr;
    }

    /* Signal handlers */
    kal_signal(KAL_SIGTERM, _mxwl_signal_handler);
    kal_signal(KAL_SIGINT, _mxwl_signal_handler);
    kal_signal(KAL_SIGCHLD, KAL_SIG_IGN);

    /* Mach host ports */
    _mxwl_kernel.initialized = TRUE;
    _mxwl_kernel.host_port = mach_port_allocate(MACH_PORT_RIGHT_RECEIVE);
    _mxwl_kernel.host_priv_port = mach_port_allocate(MACH_PORT_RIGHT_RECEIVE);
    klog_info("host port %d, host privilege port %d\n",
              _mxwl_kernel.host_port, _mxwl_kernel.host_priv_port);

    klog_info("kernel initialization complete\n");

    /* Switch to timestamped output for runtime messages */
    klog_end_boot();

    return KERN_SUCCESS;
}

/* -----------------------------------------------------------------------
 *  IPC dispatch loop
 * ----------------------------------------------------------------------- */

void mxwl_kernel_loop(void)
{
    _mxwl_kernel.running = TRUE;
    _mxwl_running = 1;

    klog_info("entering IPC dispatch loop\n");

    while (_mxwl_running) {
        int status;
        kal_pid_t pid;
        while ((pid = kal_process_reap(&status)) > 0) {
            for (int i = 0; i < TASK_MAX; i++) {
                mxwl_task_t *task = &mxwl_task_table[i];
                if (task->in_use && task->host_pid == pid) {
                    klog_notice("[task] %s (pid %d) exited with status %d\n",
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

                kal_free(msg);
            }
        }

        kal_usleep(1000);
    }

    klog_info("dispatch loop exiting\n");
}

/* -----------------------------------------------------------------------
 *  Shutdown
 * ----------------------------------------------------------------------- */

void mxwl_kernel_shutdown(void)
{
    klog_info("shutting down...\n");

    for (int i = TASK_MAX - 1; i >= 0; i--) {
        mxwl_task_t *task = &mxwl_task_table[i];
        if (task->in_use && task->id != 0) {
            mxwl_task_terminate(task);
        }
    }

    ipc_shutdown();

    _mxwl_kernel.running = FALSE;
    _mxwl_kernel.initialized = FALSE;

    klog_info("shutdown complete\n");
}

const char *mxwl_get_userfs_root(void)
{
    const char *env = kal_env_get("USERFSROOT");
    if (env && env[0]) return env;

    if (_mxwl_kernel.userfs_root[0]) {
        return _mxwl_kernel.userfs_root;
    }

    return "/";
}

void mxwl_set_userfs_root(const char *path)
{
    if (path) {
        strncpy(_mxwl_kernel.userfs_root, path,
                sizeof(_mxwl_kernel.userfs_root) - 1);
        _mxwl_kernel.userfs_root[sizeof(_mxwl_kernel.userfs_root) - 1] = '\0';
    }
}

/* -----------------------------------------------------------------------
 *  Top-level kernel main (C entry point)
 * ----------------------------------------------------------------------- */

void mxwl_kernel_main(const char *init_path)
{
    kal_terminal_t term;
    int have_saved_term = (kal_terminal_save(&term) == 0);

    uint32_t flags = 0;

    const char *v = kal_env_get("MXWL_BOOT_VERBOSE");
    if (v && v[0]) flags |= MXWL_BOOT_VERBOSE;

    const char *s = kal_env_get("MXWL_BOOT_SINGLE_USER");
    if (s && s[0]) flags |= MXWL_BOOT_SINGLE_USER;

    const char *userfs = kal_env_get("USERFSROOT");
    if (userfs && userfs[0]) {
        mxwl_set_userfs_root(userfs);
    }

    kern_return_t kr = mxwl_kernel_init(flags);
    if (kr != KERN_SUCCESS) {
        klog_err("FATAL: kernel initialization failed (%d)\n", kr);
        return;
    }

    if (!(flags & MXWL_BOOT_SINGLE_USER)) {
        kr = mxwl_launch_init(mxwl_get_userfs_root(), init_path);
        if (kr != KERN_SUCCESS) {
            klog_warn("init launch failed (%d), entering single-user mode\n", kr);
        }
    } else {
        klog_info("single-user mode, skipping init\n");
    }

    mxwl_kernel_loop();
    mxwl_kernel_shutdown();

    if (have_saved_term) {
        kal_terminal_restore(&term);
    }
}
