#ifndef MXWL_COMPAT_MACH_RPC_H
#define MXWL_COMPAT_MACH_RPC_H
#include <mach/kern_return.h>
#include <mach/message.h>
struct rpc_port_desc {
    mach_port_t name;
    mach_msg_type_name_t msgt_name;
};
#endif
