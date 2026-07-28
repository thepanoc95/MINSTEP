#ifndef MXWL_COMPAT_IPC_IPC_TARGET_H
#define MXWL_COMPAT_IPC_IPC_TARGET_H

#include <ipc/ipc_mqueue.h>
#include <ipc/ipc_object.h>
#include <mach/rpc.h>

typedef struct ipc_target {
    struct ipc_object ipt_object;
    mach_port_t ipt_name;
    struct ipc_mqueue ipt_messages;
} *ipc_target_t;

#define IPT_TYPE_MESSAGE_RPC    1
#define IPT_TYPE_MIGRATE_RPC    2

extern void ipc_target_init(struct ipc_target *ipt, mach_port_t name);
extern void ipc_target_terminate(struct ipc_target *ipt);

#define ipt_lock(ipt)       io_lock(&(ipt)->ipt_object)
#define ipt_unlock(ipt)     io_unlock(&(ipt)->ipt_object)
#define ipt_reference(ipt)  io_reference(&(ipt)->ipt_object)
#define ipt_release(ipt)    io_release(&(ipt)->ipt_object)
#define ipt_check_unlock(ipt) io_check_unlock(&(ipt)->ipt_object)

#endif
