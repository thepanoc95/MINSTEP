#ifndef MXWL_COMPAT_IPC_IPC_KMSG_QUEUE_H
#define MXWL_COMPAT_IPC_IPC_KMSG_QUEUE_H

/* Forward declaration */
struct ipc_kmsg;

struct ipc_kmsg_queue {
    struct ipc_kmsg *ikmq_base;
};

typedef struct ipc_kmsg_queue *ipc_kmsg_queue_t;

#define IKMQ_NULL ((ipc_kmsg_queue_t) 0)

#endif
