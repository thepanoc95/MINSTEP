#include "../monitor.h"
#include "server.h"
#include "../../server/manager.h"
#include "../../mach/klog.h"

static kern_return_t _server_handler(int argc, char **argv,
                                      char *out, size_t out_size)
{
    char *args[32];
    int nargs = 0;

    args[nargs++] = "server";

    for (int i = 1; i < argc && nargs < 31; i++)
        args[nargs++] = argv[i];

    mx_manager_handle_command(nargs, args, out, out_size);
    return KERN_SUCCESS;
}

static kern_return_t _start_handler(int argc, char **argv,
                                     char *out, size_t out_size)
{
    if (argc < 2) {
        libkern_snprintf(out, out_size, "Usage: start <server path>\n");
        return KERN_INVALID_ARGUMENT;
    }

    char *args[] = {
        "start", argv[1],
        NULL
    };
    int nargs = (argv[1] ? 2 : 1);

    mx_manager_handle_command(nargs, args, out, out_size);
    return KERN_SUCCESS;
}

static kern_return_t _stop_handler(int argc, char **argv,
                                    char *out, size_t out_size)
{
    if (argc < 2) {
        libkern_snprintf(out, out_size, "Usage: stop <server name>\n");
        return KERN_INVALID_ARGUMENT;
    }

    char *args[] = {
        "stop", argv[1],
        NULL
    };
    mx_manager_handle_command(2, args, out, out_size);
    return KERN_SUCCESS;
}

static kern_return_t _restart_handler(int argc, char **argv,
                                       char *out, size_t out_size)
{
    if (argc < 2) {
        libkern_snprintf(out, out_size, "Usage: restart <server name>\n");
        return KERN_INVALID_ARGUMENT;
    }

    char *args[] = {
        "restart", argv[1],
        NULL
    };
    mx_manager_handle_command(2, args, out, out_size);
    return KERN_SUCCESS;
}

mx_cmd_t _server_cmd = {
    .name       = "server",
    .help_short = "Manage servers (list, info)",
    .help_long  = "Usage: server list\n"
                  "       server info <name>\n"
                  "List running servers or show info about a specific server.",
    .handler    = _server_handler,
};

mx_cmd_t _start_cmd = {
    .name       = "start",
    .help_short = "Start a server",
    .help_long  = "Usage: start <server path>\n"
                  "Load and start a server from the given executable path.",
    .handler    = _start_handler,
};

mx_cmd_t _stop_cmd = {
    .name       = "stop",
    .help_short = "Stop a running server",
    .help_long  = "Usage: stop <server name>\n"
                  "Gracefully stop a running server by name.",
    .handler    = _stop_handler,
};

mx_cmd_t _restart_cmd = {
    .name       = "restart",
    .help_short = "Restart a server",
    .help_long  = "Usage: restart <server name>\n"
                  "Stop and restart a running server.",
    .handler    = _restart_handler,
};
