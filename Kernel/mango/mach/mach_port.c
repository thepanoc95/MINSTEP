/*
 * mango/mach/mach_port.c
 *
 * Mach port management for the Mango nanokernel.
 *
 * Ports are emulated using Unix domain socket pairs.  Each port
 * with a receive right gets its own socket pair.  The kernel holds
 * one end and the owning task holds the other.  Messages are sent
 * by writing a serialized mach_msg_t into the socket, and received
 * by reading from it.
 */

#include "mach_port.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <poll.h>

/* -----------------------------------------------------------------------
 *  Global port table
 * ----------------------------------------------------------------------- */

mach_port_object_t _mango_port_table[MACH_PORT_TABLE_SIZE];
int                _mango_port_table_count = 0;

/* -----------------------------------------------------------------------
 *  mach_port_allocate
 *
 *  Allocate a new Mach port with the given right type.
 *  Returns the port name (index in the port table), or
 *  MACH_PORT_NULL on failure.
 * ----------------------------------------------------------------------- */

mach_port_t mach_port_allocate(int right)
{
    mach_port_object_t *obj = NULL;
    int i;

    /* Find a free slot */
    for (i = 0; i < MACH_PORT_TABLE_SIZE; i++) {
        if (!_mango_port_table[i].in_use) {
            obj = &_mango_port_table[i];
            break;
        }
    }

    if (!obj) {
        return MACH_PORT_NULL;  /* Table full */
    }

    memset(obj, 0, sizeof(mach_port_object_t));
    obj->name       = i + 1;   /* 1-based; 0 is MACH_PORT_NULL */
    obj->type       = MACH_PORT_TYPE_DYNAMIC;
    obj->rights     = right;
    obj->fd         = -1;
    obj->ref_count  = 1;
    obj->in_use     = TRUE;
    obj->queue_head = 0;
    obj->queue_tail = 0;
    obj->queue_count = 0;

    /* Create the underlying socket pair for receive-right ports */
    if (right == MACH_PORT_RIGHT_RECEIVE) {
        if (mach_port_create_socket_pair(obj) < 0) {
            obj->in_use = FALSE;
            return MACH_PORT_NULL;
        }
    }

    _mango_port_table_count++;
    return obj->name;
}

/* -----------------------------------------------------------------------
 *  mach_port_deallocate
 * ----------------------------------------------------------------------- */

kern_return_t mach_port_deallocate(mach_port_t port)
{
    mach_port_object_t *obj = mach_port_lookup(port);
    if (!obj) return KERN_INVALID_RIGHT;

    if (obj->fd >= 0) {
        close(obj->fd);
        obj->fd = -1;
    }

    /* Drain any queued messages */
    while (obj->queue_count > 0) {
        mach_msg_t *msg = mach_port_dequeue_message(port);
        if (msg) free(msg);
    }

    obj->in_use = FALSE;
    obj->ref_count--;
    _mango_port_table_count--;

    return KERN_SUCCESS;
}

/* -----------------------------------------------------------------------
 *  mach_port_destroy
 * ----------------------------------------------------------------------- */

kern_return_t mach_port_destroy(mach_port_t port)
{
    mach_port_object_t *obj = mach_port_lookup(port);
    if (!obj) return KERN_INVALID_RIGHT;

    obj->ref_count = 0;
    return mach_port_deallocate(port);
}

/* -----------------------------------------------------------------------
 *  mach_port_lookup
 * ----------------------------------------------------------------------- */

mach_port_object_t *mach_port_lookup(mach_port_t port)
{
    if (port <= 0 || port > MACH_PORT_TABLE_SIZE) {
        return NULL;
    }

    mach_port_object_t *obj = &_mango_port_table[port - 1];
    if (!obj->in_use) {
        return NULL;
    }

    return obj;
}

/* -----------------------------------------------------------------------
 *  mach_port_create_socket_pair
 * ----------------------------------------------------------------------- */

int mach_port_create_socket_pair(mach_port_object_t *port_obj)
{
    int fds[2];

    if (socketpair(AF_UNIX, SOCK_STREAM, 0, fds) < 0) {
        return -1;
    }

    /* Kernel end: fds[0]  (non-blocking for poll-based dispatch) */
    /* Task end:   fds[1]  (given to the owning task)              */

    port_obj->fd = fds[0];

    /* We store the task-end fd in a special way: close it here,
     * and the task will get it via port right insertion.  For
     * simplicity in this usermode implementation, we keep both
     * ends in the kernel and use the message queue instead. */

    close(fds[1]);
    return fds[0];
}

/* -----------------------------------------------------------------------
 *  mach_port_queue_message
 * ----------------------------------------------------------------------- */

kern_return_t mach_port_queue_message(mach_port_t port, mach_msg_t *msg)
{
    mach_port_object_t *obj = mach_port_lookup(port);
    if (!obj) return KERN_INVALID_RIGHT;

    if (obj->queue_count >= MACH_PORT_QUEUE_MAX) {
        return KERN_NO_SPACE;
    }

    /* Copy the message into the queue */
    mach_msg_t *copy = malloc(msg->header.msgh_size);
    if (!copy) return KERN_FAILURE;

    memcpy(copy, msg, msg->header.msgh_size);

    obj->queue[obj->queue_tail] = copy;
    obj->queue_tail = (obj->queue_tail + 1) % MACH_PORT_QUEUE_MAX;
    obj->queue_count++;

    return KERN_SUCCESS;
}

/* -----------------------------------------------------------------------
 *  mach_port_dequeue_message
 * ----------------------------------------------------------------------- */

mach_msg_t *mach_port_dequeue_message(mach_port_t port)
{
    mach_port_object_t *obj = mach_port_lookup(port);
    if (!obj || obj->queue_count == 0) {
        return NULL;
    }

    mach_msg_t *msg = obj->queue[obj->queue_head];
    obj->queue[obj->queue_head] = NULL;
    obj->queue_head = (obj->queue_head + 1) % MACH_PORT_QUEUE_MAX;
    obj->queue_count--;

    return msg;
}

/* -----------------------------------------------------------------------
 *  mach_port_insert_send / mach_port_insert_receive
 *
 *  In Mango these are simplified: they just add the port to
 *  the task's port table.
 * ----------------------------------------------------------------------- */

kern_return_t mach_port_insert_send(mach_port_t port, mach_port_name_t name)
{
    mach_port_object_t *obj = mach_port_lookup(port);
    if (!obj) return KERN_INVALID_RIGHT;
    obj->rights = MACH_PORT_RIGHT_SEND;
    return KERN_SUCCESS;
}

kern_return_t mach_port_insert_receive(mach_port_t port, mach_port_name_t name)
{
    mach_port_object_t *obj = mach_port_lookup(port);
    if (!obj) return KERN_INVALID_RIGHT;
    obj->rights = MACH_PORT_RIGHT_RECEIVE;
    return KERN_SUCCESS;
}
