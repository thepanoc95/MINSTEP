/*
 * mxwl/mach/ipc.c
 *
 * C wrapper functions for IPC.
 * The actual implementation is in MaxxwellIPC.m (Objective-C).
 */

#include "ipc.h"
#include <string.h>
#include <stdlib.h>

void ipc_shutdown(void)
{
    /* IPC shutdown is handled by MaxxwellIPC Objective-C class */
}

kern_return_t bootstrap_handle_request(mach_msg_t *request, mach_msg_t *reply)
{
    /* Bootstrap request handling */
    (void)request;  /* unused */
    memset(reply, 0, sizeof(*reply));
    reply->header.msgh_size = sizeof(*reply);
    return KERN_SUCCESS;
}
