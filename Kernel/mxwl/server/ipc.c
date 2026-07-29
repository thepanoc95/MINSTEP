#include "ipc.h"
#include "registry.h"
#include "../mach/klog.h"
#include "../ipc/ipc.h"
#include "../mach/mach_port.h"
#include "../libkern/libkern.h"

#include <string.h>

static mx_ipc_monitor_t _monitor = NULL;
static void            *_monitor_ctx = NULL;

kern_return_t mx_ipc_init(void)
{
    klog_sub_info("srv-ipc", "ipc layer initialized\n");
    return KERN_SUCCESS;
}

void mx_ipc_shutdown(void)
{
    _monitor = NULL;
    _monitor_ctx = NULL;
}

kern_return_t mx_ipc_server_endpoint_create(mx_server_t *server)
{
    if (!server)
        return KERN_INVALID_ARGUMENT;

    mach_port_t ep = mach_port_allocate(MACH_PORT_RIGHT_RECEIVE);
    if (ep == MACH_PORT_NULL)
        return KERN_FAILURE;

    mach_port_t mgmt = mach_port_allocate(MACH_PORT_RIGHT_RECEIVE);
    if (mgmt == MACH_PORT_NULL) {
        mach_port_destroy(ep);
        return KERN_FAILURE;
    }

    server->endpoint = ep;
    server->mgmt_port = mgmt;

    return KERN_SUCCESS;
}

void mx_ipc_server_endpoint_destroy(mx_server_t *server)
{
    if (!server)
        return;

    if (server->endpoint != MACH_PORT_NULL)
        mach_port_destroy(server->endpoint);
    if (server->mgmt_port != MACH_PORT_NULL)
        mach_port_destroy(server->mgmt_port);

    server->endpoint = MACH_PORT_NULL;
    server->mgmt_port = MACH_PORT_NULL;
}

kern_return_t mx_server_send(mx_server_t *dest, mx_message_t *msg)
{
    if (!dest || !msg)
        return KERN_INVALID_ARGUMENT;

    if (dest->state != MX_SERVER_READY && dest->state != MX_SERVER_WAITING)
        return KERN_NOT_RECEIVER;

    if (dest->endpoint == MACH_PORT_NULL)
        return KERN_INVALID_OBJECT;

    kern_return_t kr = mx_msg_validate(msg);
    if (kr != KERN_SUCCESS)
        return kr;

    msg->destination = dest->endpoint;

    if (_monitor)
        _monitor(msg, _monitor_ctx);

    return mach_msg_send(dest->endpoint,
                         (mach_msg_t *)msg,
                         mx_msg_total_size(msg), 1000);
}

kern_return_t mx_server_reply(mx_message_t *request, mx_message_t *reply)
{
    if (!request || !reply)
        return KERN_INVALID_ARGUMENT;

    reply->destination = request->sender;
    reply->type = MX_MSG_REPLY;
    reply->flags |= MX_MSG_REPLY;

    mx_server_t *dest = mx_server_lookup_name("kernel");
    if (!dest || dest->endpoint == MACH_PORT_NULL)
        return KERN_INVALID_OBJECT;

    return mach_msg_send(dest->endpoint,
                         (mach_msg_t *)reply,
                         mx_msg_total_size(reply), 1000);
}

kern_return_t mx_server_broadcast(uint32_t caps_mask, mx_message_t *msg)
{
    if (!msg)
        return KERN_INVALID_ARGUMENT;

    kern_return_t result = KERN_SUCCESS;
    mx_server_t *s = mx_server_first();

    while (s) {
        if (s->state == MX_SERVER_READY && (s->capabilities & caps_mask)) {
            kern_return_t kr = mx_server_send(s, msg);
            if (kr != KERN_SUCCESS)
                result = kr;
        }
        s = mx_server_next(s);
    }

    return result;
}

kern_return_t mx_server_notify(mx_server_t *server, uint32_t event,
                                const void *data, uint32_t data_size)
{
    if (!server)
        return KERN_INVALID_ARGUMENT;

    mx_message_t *msg = NULL;
    kern_return_t kr = mx_msg_create(&msg, MACH_PORT_NULL,
                                      server->endpoint, event,
                                       MX_MSG_NOTIFY, MX_MSG_FLAG_NONE,
                                      data, data_size);
    if (kr != KERN_SUCCESS)
        return kr;

    kr = mx_server_send(server, msg);
    mx_msg_destroy(msg);
    return kr;
}

kern_return_t mx_server_rpc(mx_server_t *server, mx_message_t *request,
                             mx_message_t **reply, int timeout_ms)
{
    if (!server || !request || !reply)
        return KERN_INVALID_ARGUMENT;

    request->flags |= MX_MSG_FLAG_EXPECT_REPLY;

    kern_return_t kr = mx_server_send(server, request);
    if (kr != KERN_SUCCESS)
        return kr;

    return mach_msg_receive(server->endpoint,
                            (mach_msg_t *)*reply,
                            sizeof(mx_message_t) + MX_MSG_MAX_PAYLOAD,
                            timeout_ms);
}

void mx_ipc_set_monitor(mx_ipc_monitor_t monitor, void *context)
{
    _monitor = monitor;
    _monitor_ctx = context;
}

void mx_ipc_poll(void)
{
    mx_server_t *s = mx_server_first();
    while (s) {
        if (s->endpoint != MACH_PORT_NULL) {
            mach_port_object_t *obj = mach_port_lookup(s->endpoint);
            if (obj && obj->queue_count > 0) {
                mach_msg_t *raw = mach_port_dequeue_message(s->endpoint);
                if (raw) {
                    mx_message_t *msg = (mx_message_t *)raw;
                    if (_monitor)
                        _monitor(msg, _monitor_ctx);
                    mx_msg_destroy(msg);
                }
            }
        }
        s = mx_server_next(s);
    }
}
