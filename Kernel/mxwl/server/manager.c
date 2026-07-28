#include "manager.h"
#include "registry.h"
#include "loader.h"
#include "ipc.h"
#include "../mach/klog.h"
#include "../mach/mach_port.h"
#include "../libkern/libkern.h"
#include "../kal/kal.h"

#include <string.h>
#include <stdio.h>

kern_return_t mx_manager_init(void)
{
    klog_sub_info("srv-mgr", "server manager initialized\n");
    return KERN_SUCCESS;
}

void mx_manager_shutdown(void)
{
    mx_server_t *s = mx_server_first();
    while (s) {
        mx_manager_stop_server(s);
        s = mx_server_next(s);
    }
    mx_ipc_shutdown();
    klog_sub_info("srv-mgr", "server manager shut down\n");
}

kern_return_t mx_manager_start_server(mx_server_t *server)
{
    if (!server)
        return KERN_INVALID_ARGUMENT;

    kern_return_t kr;

    if (server->path[0] != '\0') {
        kr = mx_loader_load(server, server->path);
        if (kr != KERN_SUCCESS) {
            klog_sub_err("srv-mgr", "failed to load %s (%d)\n",
                         server->name, kr);
            return kr;
        }
    }

    kr = mx_loader_start(server);
    if (kr != KERN_SUCCESS) {
        klog_sub_err("srv-mgr", "failed to start %s (%d)\n",
                     server->name, kr);
        mx_loader_unload(server);
        return kr;
    }

    kr = mx_loader_wait_ready(server, 5000);
    if (kr != KERN_SUCCESS) {
        klog_sub_warn("srv-mgr", "%s not ready within timeout (%d)\n",
                      server->name, kr);
    }

    klog_sub_info("srv-mgr", "%s started (state: %s)\n",
                  server->name, mx_server_state_string(server->state));

    return KERN_SUCCESS;
}

kern_return_t mx_manager_stop_server(mx_server_t *server)
{
    if (!server)
        return KERN_INVALID_ARGUMENT;

    if (server->state == MX_SERVER_STOPPED)
        return KERN_SUCCESS;

    klog_sub_info("srv-mgr", "stopping %s...\n", server->name);
    mx_loader_stop(server);
    mx_loader_unload(server);

    return KERN_SUCCESS;
}

kern_return_t mx_manager_restart_server(mx_server_t *server)
{
    if (!server)
        return KERN_INVALID_ARGUMENT;

    klog_sub_info("srv-mgr", "restarting %s...\n", server->name);
    mx_manager_stop_server(server);
    return mx_manager_start_server(server);
}

kern_return_t mx_server_start(mx_server_id_t server_id)
{
    mx_server_t *server = mx_server_lookup_id(server_id);
    if (!server)
        return KERN_INVALID_OBJECT;
    return mx_manager_start_server(server);
}

kern_return_t mx_server_start_by_name(const char *name)
{
    mx_server_t *server = mx_server_lookup_name(name);
    if (!server)
        return KERN_INVALID_OBJECT;
    return mx_manager_start_server(server);
}

kern_return_t mx_server_stop(mx_server_id_t server_id)
{
    mx_server_t *server = mx_server_lookup_id(server_id);
    if (!server)
        return KERN_INVALID_OBJECT;
    return mx_manager_stop_server(server);
}

kern_return_t mx_server_stop_by_name(const char *name)
{
    mx_server_t *server = mx_server_lookup_name(name);
    if (!server)
        return KERN_INVALID_OBJECT;
    return mx_manager_stop_server(server);
}

kern_return_t mx_server_restart(mx_server_id_t server_id)
{
    mx_server_t *server = mx_server_lookup_id(server_id);
    if (!server)
        return KERN_INVALID_OBJECT;
    return mx_manager_restart_server(server);
}

kern_return_t mx_server_restart_by_name(const char *name)
{
    mx_server_t *server = mx_server_lookup_name(name);
    if (!server)
        return KERN_INVALID_OBJECT;
    return mx_manager_restart_server(server);
}

void mx_manager_poll(void)
{
    mx_ipc_poll();
}

static kern_return_t _cmd_servers(int argc, char **argv,
                                   char *out, size_t out_size)
{
    (void)argc;
    (void)argv;

    int count = mx_server_count();
    int written = snprintf(out, out_size,
                           "Servers: %d\n", count);

    mx_server_t *s = mx_server_first();
    while (s && written < (int)out_size) {
        char caps[MX_SERVER_CAPS_STR_MAX];
        mx_server_caps_string(s->capabilities, caps, sizeof(caps));

        written += snprintf(out + written, out_size - written,
                            "  %-3u  %-20s  %-8s  flags=%02x  caps=%s\n",
                            s->server_id, s->name,
                            mx_server_state_string(s->state),
                            s->flags, caps);
        s = mx_server_next(s);
    }

    if (written >= (int)out_size && out_size > 0)
        out[out_size - 1] = '\0';

    return KERN_SUCCESS;
}

static kern_return_t _cmd_start(int argc, char **argv,
                                 char *out, size_t out_size)
{
    if (argc < 2) {
        snprintf(out, out_size, "usage: start <server>\n");
        return KERN_INVALID_ARGUMENT;
    }

    mx_server_t *server;
    kern_return_t kr = mx_server_find(argv[1], &server);
    if (kr != KERN_SUCCESS) {
        snprintf(out, out_size, "server '%s' not found\n", argv[1]);
        return kr;
    }

    kr = mx_manager_start_server(server);
    snprintf(out, out_size, "start %s: %s\n", server->name,
             kr == KERN_SUCCESS ? "ok" : "failed");
    return kr;
}

static kern_return_t _cmd_stop(int argc, char **argv,
                                char *out, size_t out_size)
{
    if (argc < 2) {
        snprintf(out, out_size, "usage: stop <server>\n");
        return KERN_INVALID_ARGUMENT;
    }

    mx_server_t *server;
    kern_return_t kr = mx_server_find(argv[1], &server);
    if (kr != KERN_SUCCESS) {
        snprintf(out, out_size, "server '%s' not found\n", argv[1]);
        return kr;
    }

    kr = mx_manager_stop_server(server);
    snprintf(out, out_size, "stop %s: %s\n", server->name,
             kr == KERN_SUCCESS ? "ok" : "failed");
    return kr;
}

static kern_return_t _cmd_restart(int argc, char **argv,
                                   char *out, size_t out_size)
{
    if (argc < 2) {
        snprintf(out, out_size, "usage: restart <server>\n");
        return KERN_INVALID_ARGUMENT;
    }

    mx_server_t *server;
    kern_return_t kr = mx_server_find(argv[1], &server);
    if (kr != KERN_SUCCESS) {
        snprintf(out, out_size, "server '%s' not found\n", argv[1]);
        return kr;
    }

    kr = mx_manager_restart_server(server);
    snprintf(out, out_size, "restart %s: %s\n", server->name,
             kr == KERN_SUCCESS ? "ok" : "failed");
    return kr;
}

static kern_return_t _cmd_info(int argc, char **argv,
                                char *out, size_t out_size)
{
    if (argc < 2) {
        snprintf(out, out_size, "usage: info <server>\n");
        return KERN_INVALID_ARGUMENT;
    }

    mx_server_t *server;
    kern_return_t kr = mx_server_find(argv[1], &server);
    if (kr != KERN_SUCCESS) {
        snprintf(out, out_size, "server '%s' not found\n", argv[1]);
        return kr;
    }

    char caps[MX_SERVER_CAPS_STR_MAX];
    mx_server_caps_string(server->capabilities, caps, sizeof(caps));

    snprintf(out, out_size,
             "Server:   %s\n"
             "ID:       %u\n"
             "Version:  %s\n"
             "Path:     %s\n"
             "State:    %s\n"
             "Task ID:  %d\n"
             "PID:      %d\n"
             "Endpoint: %d\n"
             "Mgmt:     %d\n"
             "Caps:     %s\n"
             "Flags:    0x%02x\n"
             "Restarts: %d\n",
             server->name,
             server->server_id,
             server->version,
             server->path,
             mx_server_state_string(server->state),
             server->task ? server->task->id : -1,
             server->task ? server->task->host_pid : -1,
             server->endpoint,
             server->mgmt_port,
             caps,
             server->flags,
             server->restart_count);

    return KERN_SUCCESS;
}

static kern_return_t _cmd_trace(int argc, char **argv,
                                 char *out, size_t out_size)
{
    (void)argc;
    (void)argv;
    snprintf(out, out_size, "server trace: not implemented\n");
    return KERN_FAILURE;
}

static kern_return_t _cmd_panic(int argc, char **argv,
                                 char *out, size_t out_size)
{
    if (argc < 2) {
        snprintf(out, out_size, "usage: panic <server>\n");
        return KERN_INVALID_ARGUMENT;
    }

    mx_server_t *server;
    kern_return_t kr = mx_server_find(argv[1], &server);
    if (kr != KERN_SUCCESS) {
        snprintf(out, out_size, "server '%s' not found\n", argv[1]);
        return kr;
    }

    mx_server_set_state(server, MX_SERVER_PANIC);
    snprintf(out, out_size, "server %s set to PANIC\n", server->name);
    return KERN_SUCCESS;
}

typedef struct {
    const char *name;
    mx_manager_cmd_handler_t handler;
} cmd_entry_t;

static cmd_entry_t _commands[] = {
    {"servers", _cmd_servers},
    {"start",   _cmd_start},
    {"stop",    _cmd_stop},
    {"restart", _cmd_restart},
    {"info",    _cmd_info},
    {"trace",   _cmd_trace},
    {"panic",   _cmd_panic},
    {NULL, NULL}
};

kern_return_t mx_manager_handle_command(int argc, char **argv,
                                         char *out, size_t out_size)
{
    if (argc < 1 || !argv || !out)
        return KERN_INVALID_ARGUMENT;

    for (int i = 0; _commands[i].name; i++) {
        if (libkern_strcmp(argv[0], _commands[i].name) == 0)
            return _commands[i].handler(argc, argv, out, out_size);
    }

    snprintf(out, out_size, "unknown command: %s\n", argv[0]);
    return KERN_INVALID_ARGUMENT;
}
