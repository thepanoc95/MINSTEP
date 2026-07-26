#ifndef MANGO_COMPAT_MACH_NOTIFY_H
#define MANGO_COMPAT_MACH_NOTIFY_H

#include <mach/port.h>
#include <mach/message.h>

#define MACH_NOTIFY_FIRST            0100
#define MACH_NOTIFY_PORT_DELETED     (MACH_NOTIFY_FIRST + 001)
#define MACH_NOTIFY_MSG_ACCEPTED     (MACH_NOTIFY_FIRST + 002)
#define MACH_NOTIFY_PORT_DESTROYED   (MACH_NOTIFY_FIRST + 005)
#define MACH_NOTIFY_NO_SENDERS       (MACH_NOTIFY_FIRST + 006)
#define MACH_NOTIFY_SEND_ONCE        (MACH_NOTIFY_FIRST + 007)
#define MACH_NOTIFY_DEAD_NAME        (MACH_NOTIFY_FIRST + 010)
#define MACH_NOTIFY_LAST             (MACH_NOTIFY_FIRST + 015)

typedef struct {
    mach_msg_header_t   not_header;
    mach_msg_type_t     not_type;
    mach_port_t         not_port;
} mach_port_deleted_notification_t;

typedef struct {
    mach_msg_header_t   not_header;
    mach_msg_type_t     not_type;
    mach_port_t         not_port;
} mach_msg_accepted_notification_t;

typedef struct {
    mach_msg_header_t   not_header;
    mach_msg_type_t     not_type;
    mach_port_t         not_port;
} mach_port_destroyed_notification_t;

typedef struct {
    mach_msg_header_t   not_header;
    mach_msg_type_t     not_type;
    unsigned int        not_count;
} mach_no_senders_notification_t;

typedef struct {
    mach_msg_header_t   not_header;
} mach_send_once_notification_t;

typedef struct {
    mach_msg_header_t   not_header;
    mach_msg_type_t     not_type;
    mach_port_t         not_port;
} mach_dead_name_notification_t;

#endif
