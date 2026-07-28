/*
 * compat/ipc/ipc_notify.h
 *
 * Usermode compatibility shim for Mach IPC notification declarations.
 *
 * Original: osfmk/kernel/ipc/ipc_notify.h
 */

#ifndef MXWL_COMPAT_IPC_IPC_NOTIFY_H
#define MXWL_COMPAT_IPC_IPC_NOTIFY_H

#include <mach_ipc_compat.h>

extern void
ipc_notify_init(void);

extern void
ipc_notify_port_deleted(ipc_port_t port, mach_port_t name);

extern void
ipc_notify_msg_accepted(ipc_port_t port, mach_port_t name);

extern void
ipc_notify_port_destroyed(ipc_port_t port, ipc_port_t right);

extern void
ipc_notify_no_senders(ipc_port_t port, mach_port_mscount_t mscount);

extern void
ipc_notify_send_once(ipc_port_t port);

extern void
ipc_notify_dead_name(ipc_port_t port, mach_port_t name);

#endif	/* MXWL_COMPAT_IPC_IPC_NOTIFY_H */
