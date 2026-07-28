/*
 * compat/mach/message.h
 *
 * Usermode compatibility shim for Mach message types.
 *
 * Provides the mach_msg_header_t and related message structures
 * used by the IPC subsystem (especially ipc_kmsg.c and mach_msg.c).
 * For the data-structure porting phase (ipc_splay, ipc_hash),
 * only a few basic definitions are needed.
 */

#ifndef MXWL_COMPAT_MACH_MESSAGE_H
#define MXWL_COMPAT_MACH_MESSAGE_H

#include <mach/kern_return.h>
#include <mach/port.h>

/* -----------------------------------------------------------------------
 *  Scalar types (must come before struct definitions)
 * ----------------------------------------------------------------------- */

typedef int mach_msg_size_t;
typedef int mach_msg_id_t;
typedef unsigned int mach_msg_option_t;
typedef unsigned int mach_msg_type_t;
typedef unsigned int mach_msg_type_name_t;
typedef unsigned int mach_msg_return_t;
typedef int mach_port_delta_t;
typedef unsigned int mach_msg_timeout_t;

#define MACH_MSG_TIMEOUT_NONE       0

/* -----------------------------------------------------------------------
 *  Mach message type long form
 * ----------------------------------------------------------------------- */

typedef struct mach_msg_type_long {
    unsigned int    msgt_name : 8;
    unsigned int    msgt_size : 8;
    unsigned int    msgt_number : 24;
    unsigned int    msgt_inline : 1;
    unsigned int    msgt_longform : 1;
    unsigned int    msgt_deallocate : 1;
    unsigned int    msgt_unused : 5;
} mach_msg_type_long_t;

/* -----------------------------------------------------------------------
 *  Message option flags
 * ----------------------------------------------------------------------- */

#define MACH_MSG_OPTION_NONE        0x00000000
#define MACH_SEND_MSG              0x00000001
#define MACH_RCV_MSG               0x00000002
#define MACH_SEND_TIMEOUT          0x00000010
#define MACH_RCV_TIMEOUT           0x00000020
#define MACH_SEND_NOTIFY           0x00000040
#define MACH_RCV_NOTIFY            0x00000080
#define MACH_SEND_OVERWRITE        0x00000400
#define MACH_RCV_OVERWRITE         0x00000000
#define MACH_SEND_VOUCHER          0x00000800
#define MACH_RCV_VOUCHER           0x00001000
#define MACH_SEND_TRAILER          0x00002000
#define MACH_RCV_TRAILER           0x00004000
#define MACH_SEND_NOEMPTY_NOTIFY   0x00001000

/* -----------------------------------------------------------------------
 *  Message header
 * ----------------------------------------------------------------------- */

typedef struct mach_msg_header {
    mach_msg_size_t     msgh_size;
    mach_port_t         msgh_remote_port;
    mach_port_t         msgh_local_port;
    unsigned int        msgh_reserved;
    mach_msg_id_t       msgh_id;
} mach_msg_header_t;

typedef struct mach_msg_base {
    mach_msg_size_t     msgh_size;
    mach_port_t         msgh_remote_port;
    mach_port_t         msgh_local_port;
    unsigned int        msgh_voucher_port;
    mach_msg_id_t       msgh_id;
} mach_msg_base_t;

/* -----------------------------------------------------------------------
 *  Message body
 * ----------------------------------------------------------------------- */

typedef struct mach_msg_body {
    mach_msg_size_t msgh_descriptor_count;
} mach_msg_body_t;

/* -----------------------------------------------------------------------
 *  Message sizes
 * ----------------------------------------------------------------------- */

#define MACH_MSG_SIZE_LIMIT     (256 * 1024)
#define MACH_MSG_VIRTUAL_LIMIT  (256 * 1024)

/* -----------------------------------------------------------------------
 *  Message return codes
 * ----------------------------------------------------------------------- */

#define MACH_MSG_SUCCESS            0
#define MACH_SEND_SUCCESS           0
#define MACH_SEND_IN_PROGRESS       0x10000001
#define MACH_SEND_INVALID_DEST      0x10000010
#define MACH_SEND_TIMED_OUT         0x10000011
#define MACH_SEND_MSG_TOO_SMALL     0x10000018
#define MACH_SEND_INVALID_BUFFER    0x1000001C
#define MACH_SEND_INVALID_MEMORY    0x10000024
#define MACH_SEND_INVALID_HEADER    0x1000002C
#define MACH_SEND_INVALID_BODY      0x10000038
#define MACH_SEND_INVALID_RT_OOL_SIZE 0x10000040
#define MACH_RCV_SUCCESS            0
#define MACH_RCV_IN_PROGRESS        0x10004001
#define MACH_RCV_INVALID_NAME       0x10004005
#define MACH_RCV_TIMED_OUT          0x10004007
#define MACH_RCV_TOO_LARGE          0x1000400C
#define MACH_RCV_INTERRUPTED        0x1000400E
#define MACH_RCV_PORT_DIED          0x10004010
#define MACH_RCV_PORT_CHANGED       0x10004011
#define MACH_RCV_INVALID_NOTIFY     0x10004012
#define MACH_RCV_INVALID_DATA       0x10004014
#define MACH_RCV_INVALID_TYPE       0x10004016

/* -----------------------------------------------------------------------
 *  Port queue limit
 * ----------------------------------------------------------------------- */

#define MACH_PORT_QLIMIT_DEFAULT    5
#define MACH_PORT_QLIMIT_LARGE      1024

/* -----------------------------------------------------------------------
 *  Message header bits
 * ----------------------------------------------------------------------- */

#define MACH_MSGH_BITS_REMOTE_MASK  0x0000001f
#define MACH_MSGH_BITS_LOCAL_MASK   0x00001f00
#define MACH_MSGH_BITS_VOUCHER_MASK 0x001f0000
#define MACH_MSGH_BITS_HAD_VOUCHER_MASK 0x00200000
#define MACH_MSGH_BITS_BITS_MASK    0x001fffff
#define MACH_MSGH_BITS_REMOTE(remote)    ((remote) & MACH_MSGH_BITS_REMOTE_MASK)
#define MACH_MSGH_BITS_LOCAL(local)      (((local) << 8) & MACH_MSGH_BITS_LOCAL_MASK)
#define MACH_MSGH_BITS_SET(remote, local, voucher) \
    (MACH_MSGH_BITS_REMOTE(remote) | MACH_MSGH_BITS_LOCAL(local) | \
     ((voucher) & MACH_MSGH_BITS_VOUCHER_MASK))

/* -----------------------------------------------------------------------
 *  Send/Receive option flags (extended)
 * ----------------------------------------------------------------------- */

#define MACH_SEND_ALWAYS            0x00010000
#define MACH_SEND_EXPORT            0x00000800
#define MACH_SEND_IMPORT            0x00001000

/* -----------------------------------------------------------------------
 *  Message type name constants
 * ----------------------------------------------------------------------- */

#define MACH_MSG_TYPE_PORT_NAME     0
#define MACH_MSG_TYPE_PORT_SEND     20
#define MACH_MSG_TYPE_PORT_SEND_ONCE 21
#define MACH_MSG_TYPE_PORT_RECEIVE  22
#define MACH_MSG_TYPE_COPY_SEND     19
#define MACH_MSG_TYPE_MAKE_SEND     20
#define MACH_MSG_TYPE_MAKE_SEND_ONCE 21

/* -----------------------------------------------------------------------
 *  Copy-out flags
 * ----------------------------------------------------------------------- */

#define MACH_MSG_TYPE_MOVE_SEND     0
#define MACH_MSG_TYPE_MOVE_SEND_ONCE 1
#define MACH_MSG_TYPE_COPY_RECEIVE  2
#define MACH_MSG_TYPE_MAKE_RECEIVE  3
#define MACH_MSG_TYPE_MOVE_RECEIVE  4

/* -----------------------------------------------------------------------
 *  vm_map_t stub -- not used in usermode but referenced by prototypes
 * ----------------------------------------------------------------------- */

typedef void *vm_map_t;

/* -----------------------------------------------------------------------
 *  Trailer types
 * ----------------------------------------------------------------------- */

#define MACH_MSG_TRAILER_FORMAT_0  0

typedef struct mach_msg_trailer {
    unsigned int    msgh_trailer_type;
    unsigned int    msgh_trailer_size;
} mach_msg_trailer_t;

#endif /* MXWL_COMPAT_MACH_MESSAGE_H */
