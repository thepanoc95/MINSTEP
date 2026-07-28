/*
 * mxwl/mach/ipc.h
 *
 * C wrapper functions for IPC.
 */

#ifndef MXWL_MACH_IPC_H
#define MXWL_MACH_IPC_H

#include "mach_types.h"
#include "mach_msg.h"

kern_return_t ipc_init(void);
void ipc_shutdown(void);
kern_return_t bootstrap_handle_request(mach_msg_t *request, mach_msg_t *reply);

#endif /* MXWL_MACH_IPC_H */
