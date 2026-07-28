/*
 * Developer/BSD/bsd.c
 *
 * Main entry point for the BSD subsystem server.
 *
 * The BSD server runs as a privileged user-space server that
 * provides POSIX/BSD system call services to Mach tasks.  It
 * registers with the bootstrap server as the "bsd" service and
 * enters a dispatch loop, processing system call RPC messages
 * from client tasks.
 *
 * Architecture:
 *
 *   +-----------+         +-----------+
 *   |  User     | ------> |  BSD      | ------> Host POSIX
 *   |  Task     |  Mach   |  Server   |       (fork, open,
 *   |  (client) |  IPC    |  (this)   |        read, write...)
 *   +-----------+         +-----------+
 *         |                      |
 *         +------> Kernel <------+
 *                (bootstrap)
 *
 * Copyright (c) 2026 MinSTEP Project
 * Licensed under the MIT License.
 */

#include "bsd.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>

/* ========================================================================
 *  Global server state
 * ======================================================================== */

bsd_server_state_t bsd_server;

/* ========================================================================
 *  BSD server initialization
 * ======================================================================== */

kern_return_t bsd_server_start(void)
{
    kern_return_t kr;

    fprintf(stderr, "%s v%s\n", BSD_SERVER_NAME, BSD_SERVER_VERSION);
    fprintf(stderr, "Copyright (c) 2026 MinSTEP Project. MIT License.\n");

    /* Initialize global server state */
    memset(&bsd_server, 0, sizeof(bsd_server));
    bsd_server.initialized = NO;
    bsd_server.running = NO;
    bsd_server.host_port = MACH_PORT_NULL;
    bsd_server.host_priv_port = MACH_PORT_NULL;

    /* Get bootstrap port from environment */
    const char *port_str = getenv("MANGO_BOOTSTRAP_PORT");
    if (port_str) {
        bsd_server.host_port = (mach_port_t)atoi(port_str);
    }

    /* Initialize IPC subsystem */
    kr = bsd_ipc_init();
    if (kr != KERN_SUCCESS) {
        fprintf(stderr, "[bsd] IPC init failed (%d)\n", kr);
        return kr;
    }

    /* Register as the "bsd" service */
    kr = bsd_ipc_register_service("bsd");
    if (kr != KERN_SUCCESS) {
        fprintf(stderr, "[bsd] service registration failed (%d)\n", kr);
        return kr;
    }

    fprintf(stderr, "[bsd] registered as 'bsd' service (port %d)\n",
            bsd_server.bsd_port);

    /* Initialize the process table */
    kr = bsd_process_init();
    if (kr != KERN_SUCCESS) {
        fprintf(stderr, "[bsd] process init failed (%d)\n", kr);
        return kr;
    }

    fprintf(stderr, "[bsd] BSD subsystem ready\n");

    bsd_server.initialized = YES;
    return KERN_SUCCESS;
}

/* -----------------------------------------------------------------------
 *  Main dispatch loop
 * ----------------------------------------------------------------------- */

void bsd_server_loop(void)
{
    bsd_server.running = YES;

    fprintf(stderr, "[bsd] entering dispatch loop\n");

    while (bsd_server.running) {
        /* Reap zombie children */
        bsd_process_reap();

        /* Deliver pending signals to running processes */
        for (int i = 0; i < BSD_MAX_PROCESSES; i++) {
            bsd_process_t *proc = &bsd_server.process_table[i];
            if (proc->in_use && proc->running) {
                bsd_signal_deliver(proc);
            }
        }

        /* Check for incoming messages on the BSD port */
        mach_port_object_t *bp = mach_port_lookup(bsd_server.bsd_port);
        if (bp && bp->queue_count > 0) {
            mach_msg_t *msg = mach_port_dequeue_message(bsd_server.bsd_port);
            if (msg) {
                /* Dispatch the message */
                mach_msg_id_t id = msg->header.msgh_id;

                if (id == BSD_MSG_ID_SYSCALL) {
                    /* System call RPC */
                    bsd_syscall_request_t *req = (bsd_syscall_request_t *)msg;
                    bsd_syscall_reply_t reply;

                    bsd_syscall_dispatch(req, &reply);

                    /* Send reply back via the reply port */
                    if (req->header.msgh_local_port != MACH_PORT_NULL) {
                        reply.header.msgh_remote_port =
                            req->header.msgh_local_port;
                        mach_port_queue_message(
                            req->header.msgh_local_port,
                            (mach_msg_t *)&reply);
                    }
                } else if (id == MACH_MSG_ID_BOOTSTRAP_REGISTER ||
                           id == MACH_MSG_ID_BOOTSTRAP_LOOKUP) {
                    /* Bootstrap requests forwarded by the kernel */
                    mach_msg_t reply_msg;
                    memset(&reply_msg, 0, sizeof(reply_msg));
                    reply_msg.header.msgh_size = sizeof(reply_msg);

                    bootstrap_handle_request(msg, &reply_msg);

                    if (msg->header.msgh_local_port != MACH_PORT_NULL) {
                        mach_port_queue_message(
                            msg->header.msgh_local_port, &reply_msg);
                    }
                } else {
                    fprintf(stderr, "[bsd] unknown message id: %d\n", id);
                }

                free(msg);
            }
        }

        usleep(1000);
    }

    fprintf(stderr, "[bsd] dispatch loop exiting\n");
}

/* -----------------------------------------------------------------------
 *  Shutdown
 * ----------------------------------------------------------------------- */

void bsd_server_shutdown(void)
{
    fprintf(stderr, "[bsd] shutting down...\n");

    /* Terminate all tracked processes */
    for (int i = BSD_MAX_PROCESSES - 1; i >= 0; i--) {
        bsd_process_t *proc = &bsd_server.process_table[i];
        if (proc->in_use && proc->running) {
            kill(proc->pid, SIGTERM);
            bsd_syscall_exit(proc, 0);
        }
    }

    bsd_ipc_shutdown();

    bsd_server.running = NO;
    bsd_server.initialized = NO;

    fprintf(stderr, "[bsd] shutdown complete\n");
}

/* ========================================================================
 *  Main entry point (called from main())
 * ======================================================================== */

void bsd_server_main(void)
{
    kern_return_t kr = bsd_server_start();
    if (kr != KERN_SUCCESS) {
        fprintf(stderr, "[bsd] FATAL: initialization failed (%d)\n", kr);
        return;
    }

    bsd_server_loop();
    bsd_server_shutdown();
}
