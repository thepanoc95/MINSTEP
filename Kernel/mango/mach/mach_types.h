/*
 * mango/mach/mach_types.h
 *
 * Core Mach-compatible type definitions for the Mango nanokernel.
 * These types mirror the real Mach kernel types so that code
 * written against Mango can be retargeted at a real Mach kernel
 * with minimal changes.
 */

#ifndef MANGO_MACH_TYPES_H
#define MANGO_MACH_TYPES_H

#include <stdint.h>
#include <stddef.h>

/* -----------------------------------------------------------------------
 *  Fundamental scalar types
 * ----------------------------------------------------------------------- */

typedef int             mach_port_t;
typedef int             mach_port_name_t;
typedef unsigned int    mach_msg_type_name_t;
typedef unsigned int    mach_msg_size_t;
typedef int             mach_msg_id_t;
typedef unsigned int    mach_msg_option_t;
typedef int             mach_port_right_t;
typedef int             kern_return_t;
typedef int             mach_task_t;
typedef int             mach_thread_t;

/* -----------------------------------------------------------------------
 *  Null / invalid sentinels
 * ----------------------------------------------------------------------- */

#define MACH_PORT_NULL          ((mach_port_t)0)
#define MACH_PORT_INVALID       ((mach_port_t)-1)
#define MACH_PORT_DEAD          ((mach_port_t)-2)

/* -----------------------------------------------------------------------
 *  Port rights
 * ----------------------------------------------------------------------- */

#define MACH_PORT_RIGHT_SEND        0
#define MACH_PORT_RIGHT_RECEIVE     1
#define MACH_PORT_RIGHT_SEND_ONCE   2
#define MACH_PORT_RIGHT_DEAD_NAME   3

/* -----------------------------------------------------------------------
 *  Port types (for ip_object)
 * ----------------------------------------------------------------------- */

#define MACH_PORT_TYPE_DYNAMIC      0
#define MACH_PORT_TYPE_NAMED        1
#define MACH_PORT_TYPE_HOST         2
#define MACH_PORT_TYPE_HOST_PRIV    3
#define MACH_PORT_TYPE_BOOTSTRAP    4

/* -----------------------------------------------------------------------
 *  Message options
 * ----------------------------------------------------------------------- */

#define MACH_MSG_OPTION_NONE        0x00000000
#define MACH_SEND_MSG              0x00000001
#define MACH_RCV_MSG               0x00000002
#define MACH_SEND_TIMEOUT          0x00000010
#define MACH_RCV_TIMEOUT           0x00000020
#define MACH_SEND_NOTIFY           0x00000040
#define MACH_RCV_NOTIFY            0x00000080

/* -----------------------------------------------------------------------
 *  Message sizes
 * ----------------------------------------------------------------------- */

#define MACH_MSG_SIZE_LIMIT         (256 * 1024)
#define MACH_MSG_VIRTUAL_LIMIT      (256 * 1024)

/* -----------------------------------------------------------------------
 *  Kernel return codes
 * ----------------------------------------------------------------------- */

#define KERN_SUCCESS               0
#define KERN_INVALID_ARGUMENT      4
#define KERN_FAILURE               5
#define KERN_INVALID_HOST          7
#define KERN_INVALID_RIGHT         8
#define KERN_INVALID_OBJECT        9
#define KERN_INVALID_NAME          10
#define KERN_NOT_RECEIVER          14
#define KERN_NO_SPACE              29
#define KERN_INVALID_CAPABILITY    30
#define KERN_TERMINATED            46
#define KERN_MSG_TOO_LARGE         52
#define KERN_INVALID_TASK          73
#define KERN_NOT_IN_SET            77
#define KERN_NAME_EXISTS           78
#define KERN_ABORTED               90
#define KERN_INVALID_MEMORY_CONTROL 94

/* -----------------------------------------------------------------------
 *  Boolean
 * ----------------------------------------------------------------------- */

#ifndef BOOL
typedef int BOOL;
#endif
#ifndef TRUE
#define TRUE  1
#endif
#ifndef FALSE
#define FALSE 0
#endif
#ifndef YES
#define YES   1
#endif
#ifndef NO
#define NO    0
#endif

/* -----------------------------------------------------------------------
 *  Convenience macros
 * ----------------------------------------------------------------------- */

#define MACH_MSG_HEADER_NULL \
    { .msgh_size = 0, .msgh_remote_port = MACH_PORT_NULL, \
      .msgh_local_port = MACH_PORT_NULL, .msgh_id = 0 }

#endif /* MANGO_MACH_TYPES_H */
