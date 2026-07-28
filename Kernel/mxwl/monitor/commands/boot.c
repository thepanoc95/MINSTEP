#include "../monitor.h"
#include "boot.h"
#include "../../mach/klog.h"
#include "../../kal/kal.h"

static kern_return_t _boot_handler(int argc, char **argv,
                                    char *out, size_t out_size)
{
    if (argc < 2) {
        libkern_snprintf(out, out_size, "Usage: boot <server path>\n");
        return KERN_INVALID_ARGUMENT;
    }

    libkern_snprintf(out, out_size, "Booting server: %s\n", argv[1]);
    klog_sub_info("mon", "boot requested: %s\n", argv[1]);

    return KERN_SUCCESS;
}

static kern_return_t _halt_handler(int argc, char **argv,
                                    char *out, size_t out_size)
{
    (void)argc; (void)argv;
    libkern_snprintf(out, out_size, "Shutting down...\n");
    klog_sub_info("mon", "halt requested\n");
    extern void mx_monitor_stop_loop(void);
    mx_monitor_stop_loop();
    return KERN_ABORTED;
}

static kern_return_t _continue_handler(int argc, char **argv,
                                        char *out, size_t out_size)
{
    (void)argc; (void)argv;
    libkern_snprintf(out, out_size, "Continuing boot sequence...\n");
    extern void mx_monitor_stop_loop(void);
    mx_monitor_stop_loop();
    return KERN_ABORTED;
}

static kern_return_t _reboot_handler(int argc, char **argv,
                                      char *out, size_t out_size)
{
    (void)argc; (void)argv;
    libkern_snprintf(out, out_size, "Rebooting...\n");
    klog_sub_info("mon", "reboot requested\n");
    extern void mx_monitor_stop_loop(void);
    mx_monitor_stop_loop();
    return KERN_ABORTED;
}

static kern_return_t _load_handler(int argc, char **argv,
                                    char *out, size_t out_size)
{
    if (argc < 2) {
        libkern_snprintf(out, out_size, "Usage: load <module path>\n");
        return KERN_INVALID_ARGUMENT;
    }

    libkern_snprintf(out, out_size, "Loading module: %s\n", argv[1]);
    klog_sub_info("mon", "load module: %s\n", argv[1]);
    return KERN_SUCCESS;
}

static kern_return_t _unload_handler(int argc, char **argv,
                                      char *out, size_t out_size)
{
    if (argc < 2) {
        libkern_snprintf(out, out_size, "Usage: unload <module name>\n");
        return KERN_INVALID_ARGUMENT;
    }

    libkern_snprintf(out, out_size, "Unloading module: %s\n", argv[1]);
    klog_sub_info("mon", "unload module: %s\n", argv[1]);
    return KERN_SUCCESS;
}

static kern_return_t _boot_cmd_line_handler(int argc, char **argv,
                                              char *out, size_t out_size)
{
    if (argc < 2) {
        libkern_snprintf(out, out_size, "Usage: bootcmd <command line>\n");
        return KERN_INVALID_ARGUMENT;
    }

    libkern_snprintf(out, out_size, "Boot command line set\n");
    return KERN_SUCCESS;
}

mx_cmd_t _boot_cmd = {
    .name       = "boot",
    .help_short = "Boot the kernel with a server path",
    .help_long  = "Usage: boot <server path>\n"
                  "Boot the kernel using a specific server executable.",
    .handler    = _boot_handler,
};

mx_cmd_t _halt_cmd = {
    .name       = "halt",
    .help_short = "Halt (power off) the system",
    .help_long  = "Usage: halt\n"
                  "Immediately halts the system and powers off.",
    .handler    = _halt_handler,
};

mx_cmd_t _reboot_cmd = {
    .name       = "reboot",
    .help_short = "Reboot the system",
    .help_long  = "Usage: reboot\n"
                  "Immediately reboots the system.",
    .handler    = _reboot_handler,
};

mx_cmd_t _continue_cmd = {
    .name       = "continue",
    .help_short = "Continue boot sequence",
    .help_long  = "Usage: continue\n"
                  "Resume the normal boot sequence after MsWaiter.",
    .handler    = _continue_handler,
};

mx_cmd_t _load_cmd = {
    .name       = "load",
    .help_short = "Load a kernel module",
    .help_long  = "Usage: load <module path>\n"
                  "Load a kernel extension or module.",
    .handler    = _load_handler,
};

mx_cmd_t _unload_cmd = {
    .name       = "unload",
    .help_short = "Unload a kernel module",
    .help_long  = "Usage: unload <module name>\n"
                  "Unload a previously loaded kernel module.",
    .handler    = _unload_handler,
};

mx_cmd_t _boot_cmd_line = {
    .name       = "bootcmd",
    .help_short = "Set boot command line",
    .help_long  = "Usage: bootcmd <command line>\n"
                  "Set the boot command line for the next boot.",
    .handler    = _boot_cmd_line_handler,
};
