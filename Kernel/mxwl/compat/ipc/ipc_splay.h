/*
 * compat/ipc/ipc_splay.h
 *
 * Usermode compatibility shim for Mach IPC splay tree.
 *
 * This is a thin wrapper that re-declares the splay tree operations
 * using our compat types.  The actual implementation lives in
 * mxwl/ipc/ipc_splay.c (ported from osfmk).
 *
 * Original: osfmk/kernel/ipc/ipc_splay.h
 */

#ifndef MXWL_COMPAT_IPC_IPC_SPLAY_H
#define MXWL_COMPAT_IPC_IPC_SPLAY_H

#include <mach/port.h>
#include <kern/assert.h>
#include <kern/macro_help.h>
#include <ipc/ipc_entry.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -----------------------------------------------------------------------
 *  ipc_splay_tree -- unassembled top-down splay tree
 *
 *  Entries with large port names live here.  The tree is stored in
 *  unassembled form for efficient repeated lookups.
 * ----------------------------------------------------------------------- */

typedef struct ipc_splay_tree {
    mach_port_t         ist_name;       /* name used in last lookup */
    ipc_tree_entry_t    ist_root;       /* root of middle tree */
    ipc_tree_entry_t    ist_ltree;      /* root of left tree */
    ipc_tree_entry_t   *ist_ltreep;     /* pointer into left tree */
    ipc_tree_entry_t    ist_rtree;      /* root of right tree */
    ipc_tree_entry_t   *ist_rtreep;     /* pointer into right tree */
} *ipc_splay_tree_t;

/* Splay trees rely on the space's lock; no independent lock needed */
#define ist_lock(splay)     /* no locking */
#define ist_unlock(splay)   /* no locking */

/* -----------------------------------------------------------------------
 *  Splay tree operations
 * ----------------------------------------------------------------------- */

extern void ipc_splay_tree_init(ipc_splay_tree_t splay);

extern boolean_t ipc_splay_tree_pick(
    ipc_splay_tree_t    splay,
    mach_port_t        *namep,
    ipc_tree_entry_t   *entryp);

extern ipc_tree_entry_t ipc_splay_tree_lookup(
    ipc_splay_tree_t    splay,
    mach_port_t         name);

extern void ipc_splay_tree_insert(
    ipc_splay_tree_t    splay,
    mach_port_t         name,
    ipc_tree_entry_t    entry);

extern void ipc_splay_tree_delete(
    ipc_splay_tree_t    splay,
    mach_port_t         name,
    ipc_tree_entry_t    entry);

extern void ipc_splay_tree_split(
    ipc_splay_tree_t    splay,
    mach_port_t         name,
    ipc_splay_tree_t    entry);

extern void ipc_splay_tree_join(
    ipc_splay_tree_t    splay,
    ipc_splay_tree_t    small);

extern void ipc_splay_tree_bounds(
    ipc_splay_tree_t    splay,
    mach_port_t         name,
    mach_port_t        *lowerp,
    mach_port_t        *upperp);

extern ipc_tree_entry_t ipc_splay_traverse_start(ipc_splay_tree_t splay);
extern ipc_tree_entry_t ipc_splay_traverse_next(
    ipc_splay_tree_t    splay,
    boolean_t           delete);
extern void ipc_splay_traverse_finish(ipc_splay_tree_t splay);

#ifdef __cplusplus
}
#endif

#endif /* MXWL_COMPAT_IPC_IPC_SPLAY_H */
