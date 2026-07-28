#include "../monitor.h"
#include "debug.h"
#include "../../mach/klog.h"

static kern_return_t _debug_handler(int argc, char **argv,
                                     char *out, size_t out_size)
{
    size_t pos = 0;

    if (argc < 2) {
        pos += libkern_snprintf(out + pos, out_size - pos,
                                "Usage: debug <option> [args...]\n");
        pos += libkern_snprintf(out + pos, out_size - pos,
                                "Options: flags, regs, stack, crash\n");
        return KERN_INVALID_ARGUMENT;
    }

    if (libkern_strcmp(argv[1], "flags") == 0) {
        pos += libkern_snprintf(out + pos, out_size - pos,
                                "Kernel debug flags: 0x%08x\n", 0);
    } else if (libkern_strcmp(argv[1], "regs") == 0) {
        pos += libkern_snprintf(out + pos, out_size - pos,
                                "Register dump not available\n");
    } else {
        pos += libkern_snprintf(out + pos, out_size - pos,
                                "Unknown debug option: %s\n", argv[1]);
    }

    return KERN_SUCCESS;
}

static kern_return_t _panic_handler(int argc, char **argv,
                                     char *out, size_t out_size)
{
    (void)out; (void)out_size;

    const char *msg = "Manual panic triggered from MsWaiter";
    if (argc > 1)
        msg = argv[1];

    mx_panic(msg);
    return KERN_ABORTED;
}

static kern_return_t _trace_handler(int argc, char **argv,
                                     char *out, size_t out_size)
{
    (void)argc; (void)argv;

    libkern_snprintf(out, out_size,
                     "Stack trace not available on this platform\n");
    return KERN_SUCCESS;
}

static kern_return_t _signal_handler(int argc, char **argv,
                                      char *out, size_t out_size)
{
    if (argc < 3) {
        libkern_snprintf(out, out_size,
                         "Usage: signal <pid> <signum>\n");
        return KERN_INVALID_ARGUMENT;
    }

    int pid = libkern_atoi(argv[1]);
    int sig = libkern_atoi(argv[2]);

    libkern_snprintf(out, out_size,
                     "Signal %d sent to PID %d (stub)\n", sig, pid);
    return KERN_SUCCESS;
}

static kern_return_t _call_handler(int argc, char **argv,
                                    char *out, size_t out_size)
{
    if (argc < 2) {
        libkern_snprintf(out, out_size,
                         "Usage: call <address> [args...]\n");
        return KERN_INVALID_ARGUMENT;
    }

    libkern_snprintf(out, out_size,
                     "Call to 0x%s not supported\n", argv[1]);
    return KERN_SUCCESS;
}

mx_cmd_t _debug_cmd = {
    .name       = "debug",
    .help_short = "Kernel debugging tools",
    .help_long  = "Usage: debug flags\n"
                  "       debug regs\n"
                  "       debug stack\n"
                  "       debug crash\n"
                  "Various kernel debugging commands.",
    .handler    = _debug_handler,
};

mx_cmd_t _panic_cmd = {
    .name       = "panic",
    .help_short = "Trigger a kernel panic",
    .help_long  = "Usage: panic [message]\n"
                  "Triggers an immediate kernel panic with an optional message.",
    .handler    = _panic_handler,
};

mx_cmd_t _trace_cmd = {
    .name       = "trace",
    .help_short = "Print kernel stack trace",
    .help_long  = "Usage: trace\n"
                  "Prints the current kernel stack backtrace.",
    .handler    = _trace_handler,
};

mx_cmd_t _signal_cmd = {
    .name       = "signal",
    .help_short = "Send a signal to a process",
    .help_long  = "Usage: signal <pid> <signum>\n"
                  "Sends a signal to a process by PID.",
    .handler    = _signal_handler,
};

mx_cmd_t _call_cmd = {
    .name       = "call",
    .help_short = "Call a kernel function at a given address",
    .help_long  = "Usage: call <address> [args...]\n"
                  "Calls a kernel function by its address.",
    .handler    = _call_handler,
};
