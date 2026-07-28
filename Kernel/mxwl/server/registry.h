#ifndef MXWL_SERVER_REGISTRY_H
#define MXWL_SERVER_REGISTRY_H

#include "server.h"

#define MX_SERVER_MAX 64

kern_return_t  mx_registry_init(void);

kern_return_t  mx_server_register(mx_server_t *server);
kern_return_t  mx_server_unregister(mx_server_t *server);

mx_server_t   *mx_server_lookup_id(mx_server_id_t server_id);
mx_server_t   *mx_server_lookup_name(const char *name);
mx_server_t   *mx_server_lookup_capability(mx_capability_t cap);
mx_server_t   *mx_server_lookup_capability_any(uint32_t caps);

kern_return_t  mx_server_find(const char *query, mx_server_t **out);

int            mx_server_list(mx_server_info_t *buf, int max);

int            mx_server_count(void);
mx_server_t   *mx_server_first(void);
mx_server_t   *mx_server_next(mx_server_t *current);

#endif
