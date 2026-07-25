/*
 * mango/mach/ipc.c
 *
 * C wrapper functions for IPC.
 * The actual implementation is in MangoIPC.m (Objective-C).
 */

#include "ipc.h"
#include <string.h>
#include <stdlib.h>

kern_return_t ipc_init(void)
{
    /* IPC initialization is handled by MangoIPC Objective-C class */
    return KERN_SUCCESS;
}

void ipc_shutdown(void)
{
    /* IPC shutdown is handled by MangoIPC Objective-C class */
}

kern_return_t bootstrap_handle_request(mach_msg_t *request, mach_msg_t *reply)
{
    /* Bootstrap request handling */
    (void)request;  /* unused */
    memset(reply, 0, sizeof(*reply));
    reply->header.msgh_size = sizeof(*reply);
    return KERN_SUCCESS;
}
