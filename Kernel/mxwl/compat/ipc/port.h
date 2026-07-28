/*
 * compat/ipc/port.h
 *
 * Usermode compatibility shim for Mach IPC port definitions.
 *
 * This is the minimal port header needed to satisfy includes from
 * ipc_entry.h (which includes <ipc/port.h> in the osfmk tree).
 * The full port implementation is not needed for data structure porting.
 */

#ifndef MXWL_COMPAT_IPC_PORT_H
#define MXWL_COMPAT_IPC_PORT_H

#include <ipc/ipc_types.h>

/* The full ipc_port structure is defined in ipc_port.h.
 * This header exists to satisfy the include dependency from
 * the osfmk header tree. */

#endif /* MXWL_COMPAT_IPC_PORT_H */
