/*
 * compat/ipc/ipc_hash.h
 *
 * Usermode compatibility shim for Mach IPC hash table.
 *
 * Provides the reverse hash table interface: (space, object) -> name.
 * The implementation is in mxwl/ipc/ipc_hash.c (ported from osfmk).
 *
 * Original: osfmk/kernel/ipc/ipc_hash.h
 */

#ifndef MXWL_COMPAT_IPC_IPC_HASH_H
#define MXWL_COMPAT_IPC_IPC_HASH_H

#include <mach_ipc_debug.h>
#include <mach/boolean.h>
#include <mach/kern_return.h>
#include <ipc/ipc_types.h>
#include <ipc/ipc_object.h>
#include <ipc/ipc_entry.h>
#include <ipc/ipc_space.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -----------------------------------------------------------------------
 *  Initialize the hash table subsystem
 * ----------------------------------------------------------------------- */

extern void ipc_hash_init(void);

/* -----------------------------------------------------------------------
 *  High-level: (space, obj) -> (name, entry)
 *  Tries local table first, then global splay tree hash.
 * ----------------------------------------------------------------------- */

extern boolean_t ipc_hash_lookup(
    ipc_space_t     space,
    ipc_object_t    obj,
    mach_port_t    *namep,
    ipc_entry_t    *entryp);

extern void ipc_hash_insert(
    ipc_space_t     space,
    ipc_object_t    obj,
    mach_port_t     name,
    ipc_entry_t     entry);

extern void ipc_hash_delete(
    ipc_space_t     space,
    ipc_object_t    obj,
    mach_port_t     name,
    ipc_entry_t     entry);

/* -----------------------------------------------------------------------
 *  Global (splay tree entry) primitives
 * ----------------------------------------------------------------------- */

extern boolean_t ipc_hash_global_lookup(
    ipc_space_t         space,
    ipc_object_t        obj,
    mach_port_t        *namep,
    ipc_tree_entry_t   *entryp);

extern void ipc_hash_global_insert(
    ipc_space_t         space,
    ipc_object_t        obj,
    mach_port_t         name,
    ipc_tree_entry_t    entry);

extern void ipc_hash_global_delete(
    ipc_space_t         space,
    ipc_object_t        obj,
    mach_port_t         name,
    ipc_tree_entry_t    entry);

/* -----------------------------------------------------------------------
 *  Local (table entry) primitives
 * ----------------------------------------------------------------------- */

extern boolean_t ipc_hash_local_lookup(
    ipc_space_t     space,
    ipc_object_t    obj,
    mach_port_t    *namep,
    ipc_entry_t    *entryp);

extern void ipc_hash_local_insert(
    ipc_space_t         space,
    ipc_object_t        obj,
    mach_port_index_t   index,
    ipc_entry_t         entry);

extern void ipc_hash_local_delete(
    ipc_space_t         space,
    ipc_object_t        obj,
    mach_port_index_t   index,
    ipc_entry_t         entry);

#ifdef __cplusplus
}
#endif

#endif /* MXWL_COMPAT_IPC_IPC_HASH_H */
