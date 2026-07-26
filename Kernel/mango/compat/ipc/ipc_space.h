/*
 * compat/ipc/ipc_space.h
 *
 * Usermode compatibility shim for Mach IPC spaces.
 */

#ifndef MANGO_COMPAT_IPC_IPC_SPACE_H
#define MANGO_COMPAT_IPC_IPC_SPACE_H

#include <mach_ipc_compat.h>
#include <norma_ipc.h>

#include <mach/boolean.h>
#include <mach/kern_return.h>
#include <kern/macro_help.h>
#include <kern/lock.h>
#include <ipc/ipc_types.h>
#include <ipc/ipc_entry.h>
#include <ipc/ipc_table.h>
#include <ipc/ipc_splay.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct ipc_port;

/* Typedefs MUST come before the struct that uses them */
typedef unsigned int ipc_space_refs_t;

struct ipc_space {
    decl_simple_lock_data(, is_ref_lock_data)
    ipc_space_refs_t    is_references;

    decl_simple_lock_data(, is_lock_data)
    boolean_t           is_active;
    boolean_t           is_growing;
    ipc_entry_t         is_table;
    ipc_entry_num_t     is_table_size;
    struct ipc_table_size *is_table_next;
    struct ipc_splay_tree is_tree;
    ipc_entry_num_t     is_tree_total;
    ipc_entry_num_t     is_tree_small;
    ipc_entry_num_t     is_tree_hash;

#if MACH_IPC_COMPAT
    struct ipc_port    *is_notify;
#endif
};

#define IS_NULL ((ipc_space_t) 0)

extern zone_t ipc_space_zone;

#define is_alloc()      ((ipc_space_t) zalloc(ipc_space_zone))
#define is_free(is)     zfree(ipc_space_zone, (vm_offset_t)(is))

extern struct ipc_space *ipc_space_kernel;
extern struct ipc_space *ipc_space_reply;

#define is_ref_lock_init(is)  simple_lock_init(&(is)->is_ref_lock_data)

#define ipc_space_reference_macro(is)                   \
MACRO_BEGIN                                             \
    simple_lock(&(is)->is_ref_lock_data);               \
    assert((is)->is_references > 0);                    \
    (is)->is_references++;                              \
    simple_unlock(&(is)->is_ref_lock_data);             \
MACRO_END

#define ipc_space_release_macro(is)                     \
MACRO_BEGIN                                             \
    ipc_space_refs_t _refs;                             \
    simple_lock(&(is)->is_ref_lock_data);               \
    assert((is)->is_references > 0);                    \
    _refs = --(is)->is_references;                      \
    simple_unlock(&(is)->is_ref_lock_data);             \
    if (_refs == 0)                                     \
        is_free(is);                                    \
MACRO_END

#define is_lock_init(is)        simple_lock_init(&(is)->is_lock_data)
#define is_read_lock(is)        simple_lock(&(is)->is_lock_data)
#define is_read_unlock(is)      simple_unlock(&(is)->is_lock_data)
#define is_write_lock(is)       simple_lock(&(is)->is_lock_data)
#define is_write_lock_try(is)   simple_lock_try(&(is)->is_lock_data)
#define is_write_unlock(is)     simple_unlock(&(is)->is_lock_data)
#define is_write_to_read_lock(is)

#define is_reference(is)    ipc_space_reference_macro(is)
#define is_release(is)      ipc_space_release_macro(is)

extern void ipc_space_reference(struct ipc_space *space);
extern void ipc_space_release(struct ipc_space *space);

kern_return_t ipc_space_create(ipc_table_size_t, struct ipc_space **);
kern_return_t ipc_space_create_special(struct ipc_space **);
void          ipc_space_destroy(struct ipc_space *);

#ifdef __cplusplus
}
#endif

#endif /* MANGO_COMPAT_IPC_IPC_SPACE_H */
