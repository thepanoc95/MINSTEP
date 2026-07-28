#ifndef MXWL_SERVER_BOOTSTRAP_H
#define MXWL_SERVER_BOOTSTRAP_H

#include "server.h"

#define MX_BOOTSTRAP_NAME       "bootstrap"
#define MX_MSWAITER_NAME        "MsWaiter"
#define MX_BOOTSTRAP_VERSION    "1.0"

typedef struct mx_bootstrap_config {
    char  server_path[MX_SERVER_PATH_MAX];
    int   timeout_ms;
    int   autostart_servers;
} mx_bootstrap_config_t;

kern_return_t mx_bootstrap_init(void);
kern_return_t mx_bootstrap_configure(const char *boot_args);

kern_return_t mx_bootstrap_start(void);
kern_return_t mx_bootstrap_start_server(const char *name, const char *path,
                                         uint32_t capabilities);

kern_return_t mx_bootstrap_launch_mswaiter(void);
kern_return_t mx_bootstrap_sequence(void);

const char *mx_bootstrap_get_server_path(void);

#endif
