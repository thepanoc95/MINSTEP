#ifndef MXWL_SERVER_SERVER_H
#define MXWL_SERVER_SERVER_H

#include "../mach/mach_types.h"
#include "../task/task.h"

#define MX_SERVER_NAME_MAX      64
#define MX_SERVER_VERSION_MAX   32
#define MX_SERVER_PATH_MAX      256
#define MX_SERVER_ARGV_MAX      64
#define MX_SERVER_CAPS_STR_MAX  256

typedef enum mx_server_state {
    MX_SERVER_STOPPED  = 0,
    MX_SERVER_LOADING  = 1,
    MX_SERVER_STARTING = 2,
    MX_SERVER_READY    = 3,
    MX_SERVER_WAITING  = 4,
    MX_SERVER_FAILED   = 5,
    MX_SERVER_PANIC    = 6,
    MX_SERVER_STOPPING = 7,
} mx_server_state_t;

typedef enum mx_capability {
    MX_CAP_NONE       = 0,
    MX_CAP_FILESYSTEM = (1 << 0),
    MX_CAP_DISPLAY    = (1 << 1),
    MX_CAP_NETWORK    = (1 << 2),
    MX_CAP_AUDIO      = (1 << 3),
    MX_CAP_PROCESS    = (1 << 4),
    MX_CAP_BSD        = (1 << 5),
    MX_CAP_DEVICE     = (1 << 6),
    MX_CAP_STORAGE    = (1 << 7),
    MX_CAP_IPC        = (1 << 8),
    MX_CAP_MONITOR    = (1 << 9),
    MX_CAP_INPUT      = (1 << 10),
    MX_CAP_BOOTSTRAP  = (1U << 31),
} mx_capability_t;

typedef enum mx_server_flags {
    MX_SERVER_AUTOSTART  = (1 << 0),
    MX_SERVER_CRITICAL   = (1 << 1),
    MX_SERVER_RESTART    = (1 << 2),
    MX_SERVER_HIDDEN     = (1 << 3),
    MX_SERVER_SYSTEM     = (1 << 4),
    MX_SERVER_PERSISTENT = (1 << 5),
} mx_server_flags_t;

typedef uint32_t mx_server_id_t;

typedef struct mx_server mx_server_t;

struct mx_server {
    mx_server_id_t      server_id;
    char                name[MX_SERVER_NAME_MAX];
    char                version[MX_SERVER_VERSION_MAX];
    char                path[MX_SERVER_PATH_MAX];
    mx_server_state_t   state;
    mxwl_task_t        *task;
    mxwl_thread_t      *thread;
    mach_port_t         endpoint;
    mach_port_t         mgmt_port;
    uint32_t            capabilities;
    uint32_t            flags;
    int                 argc;
    char               *argv[MX_SERVER_ARGV_MAX];
    uint32_t            permissions;
    int                 restart_count;
    int                 restart_delay_ms;
    mx_server_t        *next;
};

typedef struct mx_server_info {
    mx_server_id_t      server_id;
    char                name[MX_SERVER_NAME_MAX];
    char                version[MX_SERVER_VERSION_MAX];
    mx_server_state_t   state;
    uint32_t            capabilities;
    uint32_t            flags;
    mach_port_t         endpoint;
    int                 pid;
} mx_server_info_t;

kern_return_t mx_server_init(mx_server_t *server, const char *name);
void          mx_server_destroy(mx_server_t *server);

kern_return_t mx_server_set_state(mx_server_t *server, mx_server_state_t state);
mx_server_state_t mx_server_get_state(const mx_server_t *server);

const char *mx_server_state_string(mx_server_state_t state);
const char *mx_server_caps_string(uint32_t capabilities, char *buf, size_t n);

#endif
