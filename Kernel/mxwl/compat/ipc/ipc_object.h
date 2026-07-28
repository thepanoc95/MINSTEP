/*
 * compat/ipc/ipc_object.h
 *
 * Usermode compatibility shim for Mach IPC objects.
 */

#ifndef MXWL_COMPAT_IPC_IPC_OBJECT_H
#define MXWL_COMPAT_IPC_IPC_OBJECT_H

#include <mach/kern_return.h>
#include <mach/message.h>
#include <mach/port.h>
#include <kern/lock.h>
#include <kern/macro_help.h>
#include <kern/zalloc.h>
#include <ipc/ipc_types.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Typedefs MUST come before the struct */
typedef unsigned int ipc_object_refs_t;
typedef unsigned int ipc_object_bits_t;
typedef unsigned int ipc_object_type_t;

#define IOT_PORT        0
#define IOT_PORT_SET    1
#define IOT_NUMBER      2

typedef struct ipc_object {
    decl_simple_lock_data(, io_lock_data)
    ipc_object_refs_t   io_references;
    ipc_object_bits_t   io_bits;
} *ipc_object_t;

#define IO_NULL         ((ipc_object_t) 0)
#define IO_DEAD         ((ipc_object_t) -1)
#define IO_VALID(io)    (((io) != IO_NULL) && ((io) != IO_DEAD))

#define IO_BITS_KOTYPE  0x0000ffff
#define IO_BITS_OTYPE   0x7fff0000
#define IO_BITS_ACTIVE  0x80000000U

#define io_active(io)       ((int)(io)->io_bits < 0)
#define io_otype(io)        (((io)->io_bits & IO_BITS_OTYPE) >> 16)
#define io_kotype(io)       ((io)->io_bits & IO_BITS_KOTYPE)
#define io_makebits(active, otype, kotype)  \
    (((active) ? IO_BITS_ACTIVE : 0) | ((otype) << 16) | (kotype))

extern zone_t ipc_object_zones[IOT_NUMBER];

#define io_alloc(otype)     ((ipc_object_t) zalloc(ipc_object_zones[(otype)]))
#define io_free(otype, io)  zfree(ipc_object_zones[(otype)], (vm_offset_t)(io))

#define io_lock_init(io)    simple_lock_init(&(io)->io_lock_data)
#define io_lock(io)         simple_lock(&(io)->io_lock_data)
#define io_lock_try(io)     simple_lock_try(&(io)->io_lock_data)
#define io_unlock(io)       simple_unlock(&(io)->io_lock_data)

#define io_check_unlock(io)                         \
MACRO_BEGIN                                         \
    ipc_object_refs_t _refs = (io)->io_references;  \
    io_unlock(io);                                  \
    if (_refs == 0)                                 \
        io_free(io_otype(io), io);                  \
MACRO_END

#define io_reference(io)    MACRO_BEGIN (io)->io_references++; MACRO_END
#define io_release(io)      MACRO_BEGIN (io)->io_references--; MACRO_END

extern void     ipc_object_reference(ipc_object_t);
extern void     ipc_object_release(ipc_object_t);

/* -----------------------------------------------------------------------
 *  Object allocation and manipulation (stubs for now)
 * ----------------------------------------------------------------------- */

extern kern_return_t ipc_object_alloc(ipc_space_t space,
    ipc_object_type_t otype, mach_port_type_t obj_type,
    mach_port_nsrequest_t nsreq, mach_port_t *namep,
    ipc_object_t *objp);

extern kern_return_t ipc_object_alloc_name(ipc_space_t space,
    ipc_object_type_t otype, mach_port_type_t obj_type,
    mach_port_nsrequest_t nsreq, mach_port_t name,
    ipc_object_t *objp);

extern kern_return_t ipc_object_copyout(ipc_space_t space,
    ipc_object_t obj, mach_msg_type_name_t msgt_name,
    boolean_t copy, mach_port_t *namep);

extern kern_return_t ipc_object_translate(ipc_space_t space,
    mach_port_t name, mach_port_right_t right,
    ipc_object_t *objp);

extern void ipc_kobject_destroy(ipc_port_t port);

#ifdef __cplusplus
}
#endif

#endif /* MXWL_COMPAT_IPC_IPC_OBJECT_H */
