/*
 * Developer/BSD/bsd_ipc.c
 *
 * Mach IPC interface for the BSD server.
 * Wraps the kernel's Mach IPC primitives into a clean interface
 * for the BSD server's message-passing needs.
 */

#include "bsd.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

/* -----------------------------------------------------------------------
 *  IPC initialization
 * ----------------------------------------------------------------------- */

kern_return_t bsd_ipc_init(void)
{
    bsd_server.bsd_port = mach_port_allocate(MACH_PORT_RIGHT_RECEIVE);
    if (bsd_server.bsd_port == MACH_PORT_NULL) {
        fprintf(stderr, "[bsd] could not allocate BSD port\n");
        return KERN_FAILURE;
    }

    fprintf(stderr, "[bsd] BSD port allocated (port %d)\n",
            bsd_server.bsd_port);
    return KERN_SUCCESS;
}

/* -----------------------------------------------------------------------
 *  IPC shutdown
 * ----------------------------------------------------------------------- */

void bsd_ipc_shutdown(void)
{
    if (bsd_server.bsd_port != MACH_PORT_NULL) {
        mach_port_destroy(bsd_server.bsd_port);
        bsd_server.bsd_port = MACH_PORT_NULL;
    }
}

/* -----------------------------------------------------------------------
 *  Send a message on a Mach port
 * ----------------------------------------------------------------------- */

kern_return_t bsd_ipc_send(mach_port_t dest, mach_msg_t *msg, mach_msg_size_t size)
{
    if (dest == MACH_PORT_NULL || !msg) return KERN_INVALID_ARGUMENT;

    mach_port_object_t *port = mach_port_lookup(dest);
    if (!port) return KERN_INVALID_RIGHT;

    msg->header.msgh_remote_port = dest;
    msg->header.msgh_size = size;

    return mach_port_queue_message(dest, msg);
}

/* -----------------------------------------------------------------------
 *  Receive a message from a Mach port (polling-based)
 * ----------------------------------------------------------------------- */

kern_return_t bsd_ipc_receive(mach_port_t src, mach_msg_t *msg,
                              mach_msg_size_t size, int timeout)
{
    if (src == MACH_PORT_NULL || !msg) return KERN_INVALID_ARGUMENT;

    mach_port_object_t *port = mach_port_lookup(src);
    if (!port) return KERN_INVALID_RIGHT;

    mach_msg_t *queued = mach_port_dequeue_message(src);
    if (!queued) {
        usleep(1000);
        queued = mach_port_dequeue_message(src);
        if (!queued && timeout > 0) {
            return KERN_ABORTED;
        }
    }

    if (queued) {
        mach_msg_size_t copy_size = queued->header.msgh_size;
        if (copy_size > size) copy_size = size;
        memcpy(msg, queued, copy_size);
        free(queued);
        return KERN_SUCCESS;
    }

    return KERN_FAILURE;
}

/* -----------------------------------------------------------------------
 *  Register service with bootstrap
 * ----------------------------------------------------------------------- */

kern_return_t bsd_ipc_register_service(const char *name)
{
    return bootstrap_register(name, bsd_server.bsd_port);
}