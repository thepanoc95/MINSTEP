#include "message.h"
#include "../mach/klog.h"
#include "../libkern/libkern.h"

#include <string.h>

kern_return_t mx_msg_create(mx_message_t **out, mach_port_t sender,
                             mach_port_t destination, uint32_t msg_id,
                             mx_msg_type_t type, uint32_t flags,
                             const void *payload, uint32_t payload_size)
{
    if (!out)
        return KERN_INVALID_ARGUMENT;

    if (type < MX_MSG_SYNC || type > MX_MSG_RESPONSE)
        return KERN_INVALID_ARGUMENT;

    if (payload_size > MX_MSG_MAX_PAYLOAD)
        return KERN_MSG_TOO_LARGE;

    size_t total = MX_MSG_HEADER_SIZE + payload_size;
    mx_message_t *msg = libkern_calloc(1, total);
    if (!msg)
        return KERN_FAILURE;

    msg->sender = sender;
    msg->destination = destination;
    msg->msg_id = msg_id;
    msg->type = type;
    msg->payload_size = payload_size;
    msg->flags = flags;

    if (payload && payload_size > 0)
        libkern_memcpy(msg->payload, payload, payload_size);

    *out = msg;
    return KERN_SUCCESS;
}

void mx_msg_destroy(mx_message_t *msg)
{
    libkern_free(msg);
}

kern_return_t mx_msg_validate(const mx_message_t *msg)
{
    if (!msg)
        return KERN_INVALID_ARGUMENT;

    if (msg->type < MX_MSG_SYNC || msg->type > MX_MSG_RESPONSE)
        return KERN_INVALID_ARGUMENT;

    if (msg->payload_size > MX_MSG_MAX_PAYLOAD)
        return KERN_MSG_TOO_LARGE;

    if (msg->sender == MACH_PORT_NULL || msg->sender == MACH_PORT_DEAD)
        return KERN_INVALID_ARGUMENT;

    if (msg->destination == MACH_PORT_NULL || msg->destination == MACH_PORT_DEAD)
        return KERN_INVALID_ARGUMENT;

    return KERN_SUCCESS;
}

mx_message_t *mx_msg_dup(const mx_message_t *msg)
{
    if (!msg)
        return NULL;

    mx_message_t *copy;
    kern_return_t kr = mx_msg_create(
        &copy, msg->sender, msg->destination, msg->msg_id,
        msg->type, msg->flags,
        msg->payload_size > 0 ? msg->payload : NULL,
        msg->payload_size);

    if (kr != KERN_SUCCESS)
        return NULL;

    return copy;
}

uint32_t mx_msg_total_size(const mx_message_t *msg)
{
    if (!msg)
        return 0;
    return MX_MSG_HEADER_SIZE + msg->payload_size;
}
