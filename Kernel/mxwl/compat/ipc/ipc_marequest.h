#ifndef MXWL_COMPAT_IPC_IPC_MAREQUEST_H
#define MXWL_COMPAT_IPC_IPC_MAREQUEST_H

#include <mach/kern_return.h>
#include <mach/port.h>

struct ipc_space;

typedef struct ipc_marequest {
    struct ipc_space *imar_space;
    mach_port_t imar_name;
    struct ipc_port *imar_soright;
    struct ipc_marequest *imar_next;
} *ipc_marequest_t;

#define IMAR_NULL ((ipc_marequest_t) 0)

extern void ipc_marequest_init(void);
extern mach_msg_return_t ipc_marequest_create(struct ipc_space *, mach_port_t, struct ipc_port *, ipc_marequest_t *);
extern void ipc_marequest_cancel(struct ipc_space *, mach_port_t);
extern void ipc_marequest_rename(struct ipc_space *, mach_port_t, mach_port_t);
extern void ipc_marequest_destroy(ipc_marequest_t);

#endif
