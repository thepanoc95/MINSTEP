/*
 * mango/mach/mach_kernel.c
 *
 * Main kernel entry point and lifecycle management for the
 * Mango nanokernel.
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

mango_kernel_state_t _mango_kernel;

static volatile int _mango_running = 1;

static void _mango_signal_handler(int sig)
{
    (void)sig;
    _mango_running = 0;
}

/* -----------------------------------------------------------------------
 *  Banner -- printed once at boot before subsystem init
 * ----------------------------------------------------------------------- */

void mango_kernel_banner(void)
{
    klog_info("MINSTEP v%s -- Mango Nanokernel\n", MANGO_KERNEL_VERSION);
    klog_info("Copyright (c) 2026 Miguel V. Mesquita. BSD License.\n");
}

/* -----------------------------------------------------------------------
 *  Kernel initialization
 * ----------------------------------------------------------------------- */

kern_return_t mango_kernel_init(uint32_t boot_flags)
{
    kern_return_t kr;

    klog_init_clock();

    memset(&_mango_kernel, 0, sizeof(_mango_kernel));
    _mango_kernel.boot_flags = boot_flags;
    _mango_kernel.kernel_pid = kal_getpid();

    mango_kernel_banner();

    if (boot_flags & MANGO_BOOT_VERBOSE)
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
    kr = mango_task_init();
    if (kr != KERN_SUCCESS) {
        klog_err("task init failed (%d)\n", kr);
        return kr;
    }

    /* Loader subsystem */
    kr = mango_loader_init();
    if (kr != KERN_SUCCESS) {
        klog_err("loader init failed (%d)\n", kr);
        return kr;
    }

    /* Signal handlers */
    kal_signal(KAL_SIGTERM, _mango_signal_handler);
    kal_signal(KAL_SIGINT, _mango_signal_handler);
    kal_signal(KAL_SIGCHLD, KAL_SIG_IGN);

    /* Mach host ports */
    _mango_kernel.initialized = TRUE;
    _mango_kernel.host_port = mach_port_allocate(MACH_PORT_RIGHT_RECEIVE);
    _mango_kernel.host_priv_port = mach_port_allocate(MACH_PORT_RIGHT_RECEIVE);
    klog_info("host port %d, host privilege port %d\n",
              _mango_kernel.host_port, _mango_kernel.host_priv_port);

    klog_info("kernel initialization complete\n");

    /* Switch to timestamped output for runtime messages */
    klog_end_boot();

    return KERN_SUCCESS;
}

/* -----------------------------------------------------------------------
 *  IPC dispatch loop
 * ----------------------------------------------------------------------- */

void mango_kernel_loop(void)
{
    _mango_kernel.running = TRUE;
    _mango_running = 1;

    klog_info("entering IPC dispatch loop\n");

    while (_mango_running) {
        int status;
        kal_pid_t pid;
        while ((pid = kal_process_reap(&status)) > 0) {
            for (int i = 0; i < TASK_MAX; i++) {
                mango_task_t *task = &mango_task_table[i];
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
    const char *env = kal_env_get("USERFSROOT");
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

/* -----------------------------------------------------------------------
 *  Top-level kernel main (C entry point)
 * ----------------------------------------------------------------------- */

void mango_kernel_main(const char *init_path)
{
    kal_terminal_t term;
    int have_saved_term = (kal_terminal_save(&term) == 0);

    uint32_t flags = 0;

    const char *v = kal_env_get("MANGO_BOOT_VERBOSE");
    if (v && v[0]) flags |= MANGO_BOOT_VERBOSE;

    const char *s = kal_env_get("MANGO_BOOT_SINGLE_USER");
    if (s && s[0]) flags |= MANGO_BOOT_SINGLE_USER;

    const char *userfs = kal_env_get("USERFSROOT");
    if (userfs && userfs[0]) {
        mango_set_userfs_root(userfs);
    }

    kern_return_t kr = mango_kernel_init(flags);
    if (kr != KERN_SUCCESS) {
        klog_err("FATAL: kernel initialization failed (%d)\n", kr);
        return;
    }

    if (!(flags & MANGO_BOOT_SINGLE_USER)) {
        kr = mango_launch_init(mango_get_userfs_root(), init_path);
        if (kr != KERN_SUCCESS) {
            klog_warn("init launch failed (%d), entering single-user mode\n", kr);
        }
    } else {
        klog_info("single-user mode, skipping init\n");
    }

    mango_kernel_loop();
    mango_kernel_shutdown();

    if (have_saved_term) {
        kal_terminal_restore(&term);
    }
}
