#include "server.h"
#include "../mach/klog.h"
#include "../libkern/libkern.h"

#include <string.h>

static uint32_t _next_server_id = 1;

static const char *_state_names[] = {
    "STOPPED", "LOADING", "STARTING", "READY",
    "WAITING", "FAILED", "PANIC", "STOPPING"
};

static const char *_cap_names[] = {
    "filesystem", "display", "network", "audio", "process",
    "bsd", "device", "storage", "ipc", "monitor", "input",
};

kern_return_t mx_server_init(mx_server_t *server, const char *name)
{
    if (!server || !name)
        return KERN_INVALID_ARGUMENT;

    libkern_memset(server, 0, sizeof(mx_server_t));
    server->server_id = __atomic_fetch_add(&_next_server_id, 1, __ATOMIC_RELAXED);
    libkern_strncpy(server->name, name, MX_SERVER_NAME_MAX - 1);
    server->state = MX_SERVER_STOPPED;
    server->restart_delay_ms = 1000;
    server->capabilities = MX_CAP_NONE;
    server->permissions = 0;
    return KERN_SUCCESS;
}

void mx_server_destroy(mx_server_t *server)
{
    if (!server)
        return;
    server->state = MX_SERVER_STOPPED;
    server->task = NULL;
    server->thread = NULL;
    server->endpoint = MACH_PORT_NULL;
    server->mgmt_port = MACH_PORT_NULL;
}

kern_return_t mx_server_set_state(mx_server_t *server, mx_server_state_t state)
{
    if (!server)
        return KERN_INVALID_ARGUMENT;
    mx_server_state_t old = server->state;
    server->state = state;
    klog_sub_info("srv", "server %s state %s -> %s\n",
                  server->name,
                  mx_server_state_string(old),
                  mx_server_state_string(state));
    return KERN_SUCCESS;
}

mx_server_state_t mx_server_get_state(const mx_server_t *server)
{
    return server ? server->state : MX_SERVER_STOPPED;
}

const char *mx_server_state_string(mx_server_state_t state)
{
    if (state < 0 || state > MX_SERVER_STOPPING)
        return "UNKNOWN";
    return _state_names[state];
}

const char *mx_server_caps_string(uint32_t capabilities, char *buf, size_t n)
{
    if (!buf || n == 0)
        return NULL;

    buf[0] = '\0';
    size_t pos = 0;
    int first = 1;

    for (int i = 0; i < 11; i++) {
        if (capabilities & (1 << i)) {
            if (!first) {
                if (pos < n - 1)
                    buf[pos++] = ',';
            }
            first = 0;
            size_t slen = libkern_strlen(_cap_names[i]);
            if (pos + slen < n - 1) {
                libkern_memcpy(buf + pos, _cap_names[i], slen);
                pos += slen;
            }
        }
    }

    if (pos == 0 && n > 0)
        buf[pos] = '\0';

    return buf;
}
