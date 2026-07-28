#ifndef MXWL_SERVER_MANAGER_H
#define MXWL_SERVER_MANAGER_H

#include "server.h"

kern_return_t mx_manager_init(void);
void          mx_manager_shutdown(void);

kern_return_t mx_server_start(mx_server_id_t server_id);
kern_return_t mx_server_start_by_name(const char *name);

kern_return_t mx_server_stop(mx_server_id_t server_id);
kern_return_t mx_server_stop_by_name(const char *name);

kern_return_t mx_server_restart(mx_server_id_t server_id);
kern_return_t mx_server_restart_by_name(const char *name);

kern_return_t mx_manager_start_server(mx_server_t *server);
kern_return_t mx_manager_stop_server(mx_server_t *server);
kern_return_t mx_manager_restart_server(mx_server_t *server);

void          mx_manager_poll(void);

typedef kern_return_t (*mx_manager_cmd_handler_t)(int argc, char **argv,
                                                    char *out, size_t out_size);

kern_return_t mx_manager_handle_command(int argc, char **argv,
                                         char *out, size_t out_size);

#endif
