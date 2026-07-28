#ifndef MXWL_SERVER_IPC_H
#define MXWL_SERVER_IPC_H

#include "server.h"
#include "message.h"

typedef void (*mx_ipc_monitor_t)(mx_message_t *msg, void *context);

kern_return_t mx_ipc_init(void);
void          mx_ipc_shutdown(void);

kern_return_t mx_ipc_server_endpoint_create(mx_server_t *server);
void          mx_ipc_server_endpoint_destroy(mx_server_t *server);

kern_return_t mx_server_send(mx_server_t *dest, mx_message_t *msg);
kern_return_t mx_server_reply(mx_message_t *request, mx_message_t *reply);

kern_return_t mx_server_broadcast(uint32_t caps_mask, mx_message_t *msg);

kern_return_t mx_server_notify(mx_server_t *server, uint32_t event,
                                const void *data, uint32_t data_size);

kern_return_t mx_server_rpc(mx_server_t *server, mx_message_t *request,
                             mx_message_t **reply, int timeout_ms);

void          mx_ipc_set_monitor(mx_ipc_monitor_t monitor, void *context);
void          mx_ipc_poll(void);

#endif
