/*
 * compat/ipc/ipc_init.h
 *
 * IPC subsystem initialization declarations.
 *
 * Provides ipc_tree_entry_max (used by ipc_hash_init to size
 * the global reverse hash table) and ipc_table_init.
 *
 * Original: osfmk/kernel/ipc/ipc_init.h
 */

#ifndef MXWL_COMPAT_IPC_IPC_INIT_H
#define MXWL_COMPAT_IPC_IPC_INIT_H

#include <mach/kern_return.h>
#include <mach/vm_types.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Maximum number of tree entries -- used to size the hash table */
extern unsigned int ipc_tree_entry_max;

/* Initialize the IPC table growth schedule */
extern void ipc_table_init(void);

/* Initialize the IPC subsystem (zones, hash, etc.) */
extern kern_return_t ipc_init(void);

#ifdef __cplusplus
}
#endif

#endif /* MXWL_COMPAT_IPC_IPC_INIT_H */
