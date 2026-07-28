/*
 * mxwl/ipc/ipc.h
 *
 * Maxxwell IPC subsystem -- the glue between Mach message passing
 * and the underlying POSIX environment.
 *
 * IPC in Maxxwell is built on Unix domain socket pairs.  Each Mach
 * port that has a receive right gets its own socket pair.  Senders
 * write formatted mach_msg_t structures into the socket; receivers
 * read them out.  The kernel multiplexes these under a select/poll
 * loop, just as Mach multiplexes IPC under its own port system.
 */

#ifndef MXWL_IPC_IPC_H
#define MXWL_IPC_IPC_H

#include "../mach/mach_types.h"
#include "../mach/mach_msg.h"
#include "../mach/mach_port.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -----------------------------------------------------------------------
 *  IPC bootstrap port
 *
 *  Every task receives a bootstrap port at creation time.  Services
 *  register themselves with the bootstrap server (the kernel) by
 *  checking in on their bootstrap port.  Clients look up services
 *  by name via the bootstrap port.
 * ----------------------------------------------------------------------- */

typedef struct ipc_service_entry {
    char                name[128];          /* Service name                 */
    mach_port_t         port;               /* Port registered to this name */
    BOOL                in_use;
} ipc_service_entry_t;

#define IPC_MAX_SERVICES  128

extern ipc_service_entry_t ipc_service_table[IPC_MAX_SERVICES];
extern int                 ipc_service_count;
extern mach_port_t         ipc_bootstrap_port;

/* -----------------------------------------------------------------------
 *  IPC initialization
 * ----------------------------------------------------------------------- */

/* Initialize the IPC subsystem.  Creates the bootstrap port. */
kern_return_t ipc_init(void);

/* Shut down the IPC subsystem. */
void ipc_shutdown(void);

/* -----------------------------------------------------------------------
 *  Message passing
 * ----------------------------------------------------------------------- */

/* Send a message on a port (blocking). */
kern_return_t mach_msg_send(mach_port_t dest, mach_msg_t *msg,
                            mach_msg_size_t size, int timeout_ms);

/* Receive a message from a port (blocking with timeout). */
kern_return_t mach_msg_receive(mach_port_t src, mach_msg_t *msg,
                               mach_msg_size_t size, int timeout_ms);

/* Combined send/receive (RPC-style). */
kern_return_t mach_msg_rpc(mach_port_t dest, mach_msg_t *msg,
                           mach_msg_size_t send_size,
                           mach_msg_t *reply, mach_msg_size_t reply_size,
                           int timeout_ms);

/* -----------------------------------------------------------------------
 *  Bootstrap server operations
 * ----------------------------------------------------------------------- */

/* Register a named service port with the bootstrap server. */
kern_return_t bootstrap_register(const char *name, mach_port_t port);

/* Look up a named service port. */
kern_return_t bootstrap_lookup(const char *name, mach_port_t *out_port);

/* Handle an incoming bootstrap request message. */
kern_return_t bootstrap_handle_request(mach_msg_t *request, mach_msg_t *reply);

#ifdef __cplusplus
}
#endif

#endif /* MXWL_IPC_IPC_H */
