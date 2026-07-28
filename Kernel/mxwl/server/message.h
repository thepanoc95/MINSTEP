#ifndef MXWL_SERVER_MESSAGE_H
#define MXWL_SERVER_MESSAGE_H

#include "../mach/mach_types.h"

#define MX_MSG_MAX_PAYLOAD  (64 * 1024)

typedef enum mx_msg_type {
    MX_MSG_INVALID   = 0,
    MX_MSG_SYNC      = 1,
    MX_MSG_ASYNC     = 2,
    MX_MSG_NOTIFY    = 3,
    MX_MSG_REPLY     = 4,
    MX_MSG_REQUEST   = 5,
    MX_MSG_RESPONSE  = 6,
} mx_msg_type_t;

typedef enum mx_msg_flags {
    MX_MSG_FLAG_NONE       = 0,
    MX_MSG_FLAG_URGENT     = (1 << 0),
    MX_MSG_FLAG_EXPECT_REPLY = (1 << 1),
    MX_MSG_FLAG_IS_REPLY   = (1 << 2),
    MX_MSG_FLAG_BROADCAST  = (1 << 3),
    MX_MSG_FLAG_NOTIFY     = (1 << 4),
} mx_msg_flags_t;

typedef struct mx_message {
    mach_port_t         sender;
    mach_port_t         destination;
    uint32_t            msg_id;
    mx_msg_type_t       type;
    uint32_t            payload_size;
    uint32_t            flags;
    uint8_t             payload[];
} mx_message_t;

#define MX_MSG_HEADER_SIZE  offsetof(mx_message_t, payload)

typedef kern_return_t (*mx_msg_handler_t)(mx_message_t *msg, mx_message_t **reply);

kern_return_t mx_msg_create(mx_message_t **out, mach_port_t sender,
                             mach_port_t destination, uint32_t msg_id,
                             mx_msg_type_t type, uint32_t flags,
                             const void *payload, uint32_t payload_size);

void          mx_msg_destroy(mx_message_t *msg);

kern_return_t mx_msg_validate(const mx_message_t *msg);

mx_message_t *mx_msg_dup(const mx_message_t *msg);

uint32_t mx_msg_total_size(const mx_message_t *msg);

#endif
