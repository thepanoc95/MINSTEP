#ifndef MXWL_SERVER_LOADER_H
#define MXWL_SERVER_LOADER_H

#include "server.h"

kern_return_t mx_loader_init(void);

kern_return_t mx_loader_load(mx_server_t *server, const char *exec_path);
kern_return_t mx_loader_unload(mx_server_t *server);

kern_return_t mx_loader_start(mx_server_t *server);
kern_return_t mx_loader_stop(mx_server_t *server);

kern_return_t mx_loader_setup_ipc(mx_server_t *server);
kern_return_t mx_loader_wait_ready(mx_server_t *server, int timeout_ms);

#endif
