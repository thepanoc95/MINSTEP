/*
 * mango/mach/mach_msg.h
 *
 * Mach message structure and message-passing definitions.
 * This header defines the wire format for IPC messages
 * within the Mango nanokernel.
 */

#ifndef MANGO_MACH_MSG_H
#define MANGO_MACH_MSG_H

#include "mach_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -----------------------------------------------------------------------
 *  Message header
 *
 *  Every Mach message begins with this fixed-size header.
 *  Variable-size data follows the header, aligned to the
 *  natural word boundary.
 *
 *  This layout is compatible with the real Mach msg_header_t
 *  from <mach/message.h>.
 * ----------------------------------------------------------------------- */

typedef struct mach_msg_header {
    mach_msg_size_t     msgh_size;           /* Total message size in bytes  */
    mach_port_t         msgh_remote_port;    /* Destination (or source) port */
    mach_port_t         msgh_local_port;     /* Reply port                    */
    mach_msg_type_name_t msgh_voucher_port;  /* Voucher (always 0 in Mango)  */
    mach_msg_id_t       msgh_id;             /* Message ID                    */
} mach_msg_header_t;

/* -----------------------------------------------------------------------
 *  Message descriptor (inline data descriptor)
 *
 *  Describes a single item of inline data within a message.
 *  Multiple descriptors may follow the header.
 * ----------------------------------------------------------------------- */

typedef struct mach_msg_type_descriptor {
    mach_msg_type_name_t msgt_name;          /* Type of the data             */
    unsigned int         msgt_size;          /* Size in bits                 */
    unsigned int         msgt_number;        /* Number of elements           */
    unsigned int         msgt_inline:1,      /* Data is inline in message    */
                         msgt_longform:1,    /* Long-form descriptor         */
                         msgt_deallocate:1,  /* Deallocate on send           */
                         msgt_pad1:29;
} mach_msg_type_descriptor_t;

/* -----------------------------------------------------------------------
 *  OOL (Out-of-Line) data descriptor
 * ----------------------------------------------------------------------- */

typedef struct mach_msg_ool_descriptor {
    void                *address;            /* Address of OOL data          */
    mach_msg_size_t      size;               /* Size in bytes                */
    BOOL                 deallocate;         /* Deallocate after copy-in     */
    mach_msg_type_name_t copy;               /* Copy strategy                */
    unsigned int         pad1;
} mach_msg_ool_descriptor_t;

/* -----------------------------------------------------------------------
 *  Port descriptor (for sending port rights inline)
 * ----------------------------------------------------------------------- */

typedef struct mach_msg_port_descriptor {
    mach_port_t          name;               /* Port name to send            */
    mach_msg_type_name_t disposition;        /* SEND / SEND_ONCE / etc.      */
    unsigned int         pad1;
} mach_msg_port_descriptor_t;

/* -----------------------------------------------------------------------
 *  Mango IPC message structure
 *
 *  A complete Mango IPC message.  The header is always present.
 *  Payload data follows immediately after.
 * ----------------------------------------------------------------------- */

#define MACH_MSG_BODY_MAX  4096

typedef struct mach_msg {
    mach_msg_header_t    header;
    /* descriptor array follows */
    /* payload data follows     */
} mach_msg_t;

/* -----------------------------------------------------------------------
 *  Convenience: compute total message size
 * ----------------------------------------------------------------------- */

#define MACH_MSG_SIZE(payload_size) \
    (sizeof(mach_msg_header_t) + (payload_size))

/* -----------------------------------------------------------------------
 *  Well-known message IDs
 * ----------------------------------------------------------------------- */

/* Bootstrap subsystem */
#define MACH_MSG_ID_BOOTSTRAP_CHECKIN     0x100
#define MACH_MSG_ID_BOOTSTRAP_REGISTER    0x101
#define MACH_MSG_ID_BOOTSTRAP_LOOKUP      0x102

/* Task subsystem */
#define MACH_MSG_ID_TASK_CREATE           0x200
#define MACH_MSG_ID_TASK_TERMINATE        0x201
#define MACH_MSG_ID_TASK_INFO             0x202
#define MACH_MSG_ID_TASK_SET_NAME         0x203

/* Thread subsystem */
#define MACH_MSG_ID_THREAD_CREATE         0x300
#define MACH_MSG_ID_THREAD_TERMINATE      0x301
#define MACH_MSG_ID_THREAD_RESUME         0x302
#define MACH_MSG_ID_THREAD_SUSPEND        0x303

/* Host subsystem */
#define MACH_MSG_ID_HOST_INFO             0x400
#define MACH_MSG_ID_HOST_GET_SPECIAL_PORT  0x401

/* Exception handling */
#define MACH_MSG_ID_EXCEPTION_raise       0x500
#define MACH_MSG_ID_EXCEPTION_raise_STATE 0x501

/* -----------------------------------------------------------------------
 *  Payload structures for well-known messages
 * ----------------------------------------------------------------------- */

/* Bootstrap checkin request: task asks for a service port */
typedef struct mach_bootstrap_request {
    mach_msg_header_t   header;
    mach_msg_id_t       name_length;        /* Length of service name       */
    char                service_name[128];   /* NUL-terminated service name  */
} mach_bootstrap_request_t;

/* Bootstrap checkin reply: kernel returns the port */
typedef struct mach_bootstrap_reply {
    mach_msg_header_t   header;
    mach_port_t         service_port;        /* The port for the service     */
} mach_bootstrap_reply_t;

/* Task create request */
typedef struct mach_task_create_request {
    mach_msg_header_t   header;
    mach_port_t         host_port;
    mach_port_t         bootstrap_port;
    unsigned int        flags;
} mach_task_create_request_t;

/* Task create reply */
typedef struct mach_task_create_reply {
    mach_msg_header_t   header;
    kern_return_t       ret_code;
    mach_port_t         task_port;
} mach_task_create_reply_t;

/* Generic return-only reply */
typedef struct mach_reply {
    mach_msg_header_t   header;
    kern_return_t       ret_code;
} mach_reply_t;

#ifdef __cplusplus
}
#endif

#endif /* MANGO_MACH_MSG_H */
