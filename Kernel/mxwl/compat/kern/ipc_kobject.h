/*
 * compat/kern/ipc_kobject.h
 *
 * Stub for ipc_kobject_t, needed by ipc_port.h.
 * Not used in the data structure porting phase.
 */

#ifndef MXWL_COMPAT_KERN_IPC_KOBJECT_H
#define MXWL_COMPAT_KERN_IPC_KOBJECT_H

typedef void *ipc_kobject_t;

#define IKOT_NONE           0
#define IKOT_TASK           1
#define IKOT_HOST           2
#define IKOT_HOST_PRIV      3
#define IKOT_PROCESSOR      4
#define IKOT_PROCESSOR_SET  5
#define IKOT_PAGER          6
#define IKOT_OBJECT         7
#define IKOT_NAMED_ENTRY    8
#define IKOT_PORT_SET       9
#define IKOT_COUNT          10

#endif /* MXWL_COMPAT_KERN_IPC_KOBJECT_H */
