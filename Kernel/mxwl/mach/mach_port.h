/*
 * mxwl/mach/mach_port.h
 *
 * Mach port abstraction for the Maxxwell nanokernel.
 *
 * In real Mach, ports are kernel objects that name a message queue.
 * In Maxxwell, each port maps to a Unix domain socket pair, giving
 * us bidirectional message passing within the host POSIX environment.
 */

#ifndef MXWL_MACH_PORT_H
#define MXWL_MACH_PORT_H

#include "mach_types.h"
#include "mach_msg.h"

#include <sys/un.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -----------------------------------------------------------------------
 *  Limits
 * ----------------------------------------------------------------------- */

#define MACH_PORT_TABLE_SIZE    256
#define MACH_PORT_QUEUE_MAX     64

/* -----------------------------------------------------------------------
 *  Port object
 *
 *  Each allocated port is described by one of these.  The kernel
 *  maintains a global table of all live ports.
 * ----------------------------------------------------------------------- */

typedef struct mach_port_object {
    mach_port_t         name;               /* Port name (index in table)   */
    int                 type;               /* MACH_PORT_TYPE_*              */
    int                 rights;             /* Right held by kernel/owner   */
    int                 fd;                 /* Unix domain socket fd (-1 if unused) */
    int                 ref_count;          /* Reference count               */
    BOOL                in_use;             /* Slot is allocated             */

    /* Message queue for this port (for receive right holders) */
    mach_msg_t         *queue[MACH_PORT_QUEUE_MAX];
    int                 queue_head;
    int                 queue_tail;
    int                 queue_count;
} mach_port_object_t;

/* -----------------------------------------------------------------------
 *  Port table
 * ----------------------------------------------------------------------- */

extern mach_port_object_t mxwl_port_table[MACH_PORT_TABLE_SIZE];
extern int                mxwl_port_table_count;

/* -----------------------------------------------------------------------
 *  Port API
 * ----------------------------------------------------------------------- */

/* Allocate a new port with the given right */
mach_port_t mach_port_allocate(int right);

/* Deallocate a port */
kern_return_t mach_port_deallocate(mach_port_t port);

/* Insert a send right for a port into a task's port space */
kern_return_t mach_port_insert_send(mach_port_t port, mach_port_name_t name);

/* Insert a receive right for a port into a task's port space */
kern_return_t mach_port_insert_receive(mach_port_t port, mach_port_name_t name);

/* Destroy all rights for a port */
kern_return_t mach_port_destroy(mach_port_t port);

/* Look up a port by name */
mach_port_object_t *mach_port_lookup(mach_port_t port);

/* Create a socket pair for a port (internal) */
int mach_port_create_socket_pair(mach_port_object_t *port_obj);

/* Enqueue a message on a port's receive queue */
kern_return_t mach_port_queue_message(mach_port_t port, mach_msg_t *msg);

/* Dequeue a message from a port's receive queue */
mach_msg_t *mach_port_dequeue_message(mach_port_t port);

#ifdef __cplusplus
}
#endif

#endif /* MXWL_MACH_PORT_H */
