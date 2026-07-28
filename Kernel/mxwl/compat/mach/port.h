/*
 * compat/mach/port.h
 *
 * Usermode compatibility shim for Mach port types and macros.
 *
 * Provides mach_port_t, mach_port_name_t, and the MACH_PORT_INDEX /
 * MACH_PORT_GEN / MACH_PORT_MAKE macros used throughout ipc_entry
 * and ipc_space.
 *
 * PORT_GENERATIONS is disabled in Maxxwell.  Port names are just
 * raw table indices with no generation counter.  This matches
 * IE_BITS_GEN_MASK == 0 in ipc_entry.h.
 */

#ifndef MXWL_COMPAT_MACH_PORT_H
#define MXWL_COMPAT_MACH_PORT_H

#include <mach/vm_types.h>

/* -----------------------------------------------------------------------
 *  Fundamental port types
 * ----------------------------------------------------------------------- */

typedef int             mach_port_t;
typedef int             mach_port_name_t;

#define MACH_PORT_NULL          ((mach_port_t)0)
#define MACH_PORT_INVALID       ((mach_port_t)-1)
#define MACH_PORT_DEAD          ((mach_port_t)-2)
#define MACH_PORT_MAX           ((mach_port_t)0fffffffU)

#define MACH_PORT_VALID(name)   \
    ((name) != MACH_PORT_NULL && (name) != MACH_PORT_INVALID)

/* -----------------------------------------------------------------------
 *  Port name decomposition macros
 *
 *  Without PORT_GENERATIONS, port names are just raw table indices.
 *  MACH_PORT_MAKE constructs a name from an index and a generation
 *  (which is always 0 in Maxxwell).
 * ----------------------------------------------------------------------- */

#define MACH_PORT_INDEX(name)   ((mach_port_index_t)(name))
#define MACH_PORT_GEN(name)     ((mach_port_gen_t)0)
#define MACH_PORT_MAKE(index, gen) ((mach_port_t)(index))
#define MACH_PORT_NGEN(name)    ((mach_port_t)0)

/* MACH_PORT_MAKEB requires IE_BITS_RIGHT_MASK from ipc_entry.h.
 * It is placed in ipc_entry.h as a macro. */

/* -----------------------------------------------------------------------
 *  User-reference limits
 * ----------------------------------------------------------------------- */

#define MACH_PORT_UREFS_MAX     ((mach_port_urefs_t)((1 << 16) - 1))

/* -----------------------------------------------------------------------
 *  Port rights (for ipc_object io_bits)
 * ----------------------------------------------------------------------- */

#define MACH_PORT_TYPE_NONE         0x00000000
#define MACH_PORT_TYPE_SEND         0x00010000
#define MACH_PORT_TYPE_RECEIVE      0x00020000
#define MACH_PORT_TYPE_SEND_ONCE    0x00040000
#define MACH_PORT_TYPE_PORT_SET     0x00080000
#define MACH_PORT_TYPE_DEAD_NAME    0x00100000
#define MACH_PORT_TYPE_FORMAT       0x00200000
#define MACH_PORT_TYPE_SEQUENCE     0x00400000

/* Port right constants (for ipc_right_delta) */
#define MACH_PORT_RIGHT_SEND        0
#define MACH_PORT_RIGHT_RECEIVE     1
#define MACH_PORT_RIGHT_SEND_ONCE   2
#define MACH_PORT_RIGHT_PORT_SET    3
#define MACH_PORT_RIGHT_DEAD_NAME   4

typedef unsigned int mach_port_right_t;
typedef unsigned int mach_port_nsrequest_t;

#define MACH_PORT_RIGHTS_LIMIT      5

#define MACH_PORT_TYPE_ALL_RIGHTS   \
    (MACH_PORT_TYPE_SEND | MACH_PORT_TYPE_RECEIVE | \
     MACH_PORT_TYPE_SEND_ONCE | MACH_PORT_TYPE_PORT_SET | \
     MACH_PORT_TYPE_DEAD_NAME)

#endif /* MXWL_COMPAT_MACH_PORT_H */
