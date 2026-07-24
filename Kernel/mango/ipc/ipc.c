/*
 * mango/ipc/ipc.c
 *
 * IPC subsystem implementation for the Mango nanokernel.
 */

#include "ipc.h"
#include "../mach/klog.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <poll.h>
#include <errno.h>

ipc_service_entry_t _ipc_service_table[IPC_MAX_SERVICES];
int                 _ipc_service_count = 0;
mach_port_t         _ipc_bootstrap_port = MACH_PORT_NULL;

kern_return_t ipc_init(void)
{
    _ipc_bootstrap_port = mach_port_allocate(MACH_PORT_RIGHT_RECEIVE);
    if (_ipc_bootstrap_port == MACH_PORT_NULL) {
        klog_err("could not allocate bootstrap port\n");
        return KERN_FAILURE;
    }

    _ipc_service_count = 0;
    memset(_ipc_service_table, 0, sizeof(_ipc_service_table));

    klog_info("bootstrap port ready (port %d)\n", _ipc_bootstrap_port);
    return KERN_SUCCESS;
}

void ipc_shutdown(void)
{
    if (_ipc_bootstrap_port != MACH_PORT_NULL) {
        mach_port_destroy(_ipc_bootstrap_port);
        _ipc_bootstrap_port = MACH_PORT_NULL;
    }
}

kern_return_t mach_msg_send(mach_port_t dest, mach_msg_t *msg,
                            mach_msg_size_t size, int timeout_ms)
{
    mach_port_object_t *port = mach_port_lookup(dest);
    if (!port) return KERN_INVALID_RIGHT;

    msg->header.msgh_remote_port = dest;
    msg->header.msgh_size = size;

    return mach_port_queue_message(dest, msg);
}

kern_return_t mach_msg_receive(mach_port_t src, mach_msg_t *msg,
                               mach_msg_size_t size, int timeout_ms)
{
    mach_port_object_t *port = mach_port_lookup(src);
    if (!port) return KERN_INVALID_RIGHT;

    mach_msg_t *queued = NULL;
    struct pollfd pfd;
    int elapsed = 0;

    pfd.fd = port->fd;
    pfd.events = POLLIN;

    while (!queued) {
        queued = mach_port_dequeue_message(src);
        if (queued) break;

        int ret = poll(&pfd, 1, 100);
        if (ret < 0) {
            if (errno == EINTR) continue;
            return KERN_FAILURE;
        }

        if (ret > 0 && (pfd.revents & POLLIN)) {
            queued = mach_port_dequeue_message(src);
            if (queued) break;
        }

        elapsed += 100;
        if (timeout_ms > 0 && elapsed >= timeout_ms) {
            return KERN_ABORTED;
        }
    }

    mach_msg_size_t copy_size = queued->header.msgh_size;
    if (copy_size > size) copy_size = size;
    memcpy(msg, queued, copy_size);
    free(queued);

    return KERN_SUCCESS;
}

kern_return_t mach_msg_rpc(mach_port_t dest, mach_msg_t *msg,
                           mach_msg_size_t send_size,
                           mach_msg_t *reply, mach_msg_size_t reply_size,
                           int timeout_ms)
{
    mach_port_t reply_port = mach_port_allocate(MACH_PORT_RIGHT_RECEIVE);
    if (reply_port == MACH_PORT_NULL) {
        return KERN_FAILURE;
    }

    msg->header.msgh_local_port = reply_port;

    kern_return_t kr = mach_msg_send(dest, msg, send_size, timeout_ms);
    if (kr != KERN_SUCCESS) {
        mach_port_destroy(reply_port);
        return kr;
    }

    kr = mach_msg_receive(reply_port, reply, reply_size, timeout_ms);

    mach_port_destroy(reply_port);
    return kr;
}

kern_return_t bootstrap_register(const char *name, mach_port_t port)
{
    if (!name || _ipc_service_count >= IPC_MAX_SERVICES) {
        return KERN_FAILURE;
    }

    for (int i = 0; i < _ipc_service_count; i++) {
        if (_ipc_service_table[i].in_use &&
            strcmp(_ipc_service_table[i].name, name) == 0) {
            return KERN_NAME_EXISTS;
        }
    }

    ipc_service_entry_t *entry = &_ipc_service_table[_ipc_service_count];
    strncpy(entry->name, name, sizeof(entry->name) - 1);
    entry->name[sizeof(entry->name) - 1] = '\0';
    entry->port = port;
    entry->in_use = TRUE;
    _ipc_service_count++;

    klog_info("service registered: %s (port %d)\n", name, port);
    return KERN_SUCCESS;
}

kern_return_t bootstrap_lookup(const char *name, mach_port_t *out_port)
{
    if (!name || !out_port) return KERN_INVALID_ARGUMENT;

    for (int i = 0; i < _ipc_service_count; i++) {
        if (_ipc_service_table[i].in_use &&
            strcmp(_ipc_service_table[i].name, name) == 0) {
            *out_port = _ipc_service_table[i].port;
            return KERN_SUCCESS;
        }
    }

    return KERN_INVALID_NAME;
}

kern_return_t bootstrap_handle_request(mach_msg_t *request, mach_msg_t *reply)
{
    if (!request || !reply) return KERN_INVALID_ARGUMENT;

    mach_msg_id_t id = request->header.msgh_id;

    switch (id) {
    case MACH_MSG_ID_BOOTSTRAP_REGISTER: {
        mach_bootstrap_request_t *req = (mach_bootstrap_request_t *)request;
        kern_return_t kr = bootstrap_register(req->service_name,
                                              req->header.msgh_local_port);
        reply->header.msgh_id = id + 1;
        ((mach_reply_t *)reply)->ret_code = kr;
        return kr;
    }

    case MACH_MSG_ID_BOOTSTRAP_LOOKUP: {
        mach_bootstrap_request_t *req = (mach_bootstrap_request_t *)request;
        mach_port_t svc_port = MACH_PORT_NULL;
        kern_return_t kr = bootstrap_lookup(req->service_name, &svc_port);

        mach_bootstrap_reply_t *rep = (mach_bootstrap_reply_t *)reply;
        rep->header.msgh_id = id + 1;
        rep->service_port = svc_port;
        rep->header.msgh_size = sizeof(mach_bootstrap_reply_t);
        return kr;
    }

    default:
        klog_warn("unknown bootstrap request: %d\n", id);
        return KERN_INVALID_ARGUMENT;
    }
}
