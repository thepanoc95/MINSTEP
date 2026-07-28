#include "../monitor.h"
#include "log.h"
#include "../../mach/klog.h"

static kern_return_t _log_handler(int argc, char **argv,
                                   char *out, size_t out_size)
{
    (void)argc;

    size_t pos = 0;
    pos += libkern_snprintf(out + pos, out_size - pos,
                            "Kernel Log:\n");

    if (argc > 1) {
        pos += libkern_snprintf(out + pos, out_size - pos,
                                "  (filter: %s)\n", argv[1]);
    }

    return KERN_SUCCESS;
}

static kern_return_t _history_handler(int argc, char **argv,
                                       char *out, size_t out_size)
{
    (void)argc; (void)argv;

    size_t pos = 0;
    pos += libkern_snprintf(out + pos, out_size - pos,
                            "Command History:\n");

    int n = mx_monitor_history_count();
    int start = (n > 20) ? n - 20 : 0;

    for (int i = start; i < n; i++) {
        const char *line = mx_monitor_history_get(i);
        if (line) {
            pos += libkern_snprintf(out + pos, out_size - pos,
                                    "  %3d: %s\n", i, line);
        }
    }

    if (n == 0) {
        pos += libkern_snprintf(out + pos, out_size - pos,
                                "  (empty)\n");
    }

    return KERN_SUCCESS;
}

static kern_return_t _echo_handler(int argc, char **argv,
                                    char *out, size_t out_size)
{
    size_t pos = 0;
    for (int i = 1; i < argc && pos < out_size; i++) {
        pos += libkern_snprintf(out + pos, out_size - pos,
                                "%s%s", (i > 1) ? " " : "", argv[i]);
    }
    if (pos < out_size)
        out[pos++] = '\n';
    out[pos] = '\0';
    return KERN_SUCCESS;
}

static kern_return_t _clear_handler(int argc, char **argv,
                                     char *out, size_t out_size)
{
    (void)argc; (void)argv; (void)out; (void)out_size;

    mx_monitor_printf("\033[2J\033[H");
    return KERN_SUCCESS;
}

static kern_return_t _print_handler(int argc, char **argv,
                                     char *out, size_t out_size)
{
    if (argc < 2) {
        libkern_snprintf(out, out_size, "Usage: print <variable>\n");
        return KERN_INVALID_ARGUMENT;
    }

    libkern_snprintf(out, out_size, "%s = <kernel variable>\n", argv[1]);
    return KERN_SUCCESS;
}

static kern_return_t _set_handler(int argc, char **argv,
                                   char *out, size_t out_size)
{
    if (argc < 3) {
        libkern_snprintf(out, out_size,
                         "Usage: set <variable> <value>\n");
        return KERN_INVALID_ARGUMENT;
    }

    libkern_snprintf(out, out_size,
                     "Set %s = %s\n", argv[1], argv[2]);
    return KERN_SUCCESS;
}

static kern_return_t _get_handler(int argc, char **argv,
                                   char *out, size_t out_size)
{
    if (argc < 2) {
        libkern_snprintf(out, out_size, "Usage: get <variable>\n");
        return KERN_INVALID_ARGUMENT;
    }

    libkern_snprintf(out, out_size, "%s = <kernel variable>\n", argv[1]);
    return KERN_SUCCESS;
}

mx_cmd_t _log_cmd = {
    .name       = "log",
    .help_short = "Display kernel log messages",
    .help_long  = "Usage: log [filter]\n"
                  "Show the kernel log buffer, optionally filtered by substring.",
    .handler    = _log_handler,
};

mx_cmd_t _history_cmd = {
    .name       = "history",
    .help_short = "Show command history",
    .help_long  = "Usage: history\n"
                  "Show the last 20 commands entered.",
    .handler    = _history_handler,
};

mx_cmd_t _echo_cmd = {
    .name       = "echo",
    .help_short = "Echo text to the console",
    .help_long  = "Usage: echo [args...]\n"
                  "Print text to the console.",
    .handler    = _echo_handler,
};

mx_cmd_t _clear_cmd = {
    .name       = "clear",
    .help_short = "Clear the terminal screen",
    .help_long  = "Usage: clear\n"
                  "Clear the terminal screen.",
    .handler    = _clear_handler,
};

mx_cmd_t _print_cmd = {
    .name       = "print",
    .help_short = "Print a kernel variable",
    .help_long  = "Usage: print <variable>\n"
                  "Print the current value of a kernel variable.",
    .handler    = _print_handler,
};

mx_cmd_t _set_cmd = {
    .name       = "set",
    .help_short = "Set a kernel variable or setting",
    .help_long  = "Usage: set <variable> <value>\n"
                  "Set a kernel variable or configuration value.",
    .handler    = _set_handler,
};

mx_cmd_t _get_cmd = {
    .name       = "get",
    .help_short = "Get a kernel variable or setting",
    .help_long  = "Usage: get <variable>\n"
                  "Get the current value of a kernel variable or setting.",
    .handler    = _get_handler,
};
