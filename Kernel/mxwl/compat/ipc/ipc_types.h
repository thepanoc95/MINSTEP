/*
 * compat/ipc/ipc_types.h
 *
 * Usermode compatibility shim for Mach IPC forward declarations.
 *
 * Provides the forward-declared pointer types ipc_space_t,
 * ipc_port_t, ipc_port_request_t used throughout the IPC subsystem.
 */

#ifndef MXWL_COMPAT_IPC_IPC_TYPES_H
#define MXWL_COMPAT_IPC_IPC_TYPES_H

/* Forward declarations */
struct ipc_space;
struct ipc_port;
struct ipc_port_request;
struct ipc_object;
struct ipc_entry;
struct ipc_tree_entry;
struct ipc_pset;

typedef struct ipc_space      *ipc_space_t;
typedef struct ipc_port       *ipc_port_t;

#endif /* MXWL_COMPAT_IPC_IPC_TYPES_H */
