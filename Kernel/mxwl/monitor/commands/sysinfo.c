#include "../monitor.h"
#include "sysinfo.h"
#include "../../kal/kal.h"

static kern_return_t _sysinfo_handler(int argc, char **argv,
                                       char *out, size_t out_size)
{
    (void)argc; (void)argv;

    size_t pos = 0;
    pos += libkern_snprintf(out + pos, out_size - pos,
                            "System Information:\n");
    pos += libkern_snprintf(out + pos, out_size - pos,
                            "  Monitor:      %s v%s\n",
                            MX_MONITOR_NAME, MX_MONITOR_VERSION);
    pos += libkern_snprintf(out + pos, out_size - pos,
                            "  Kernel:       %s\n", MX_MONITOR_KVER);
    pos += libkern_snprintf(out + pos, out_size - pos,
                            "  Machine:      %s\n", kal_get_machine_type());
    pos += libkern_snprintf(out + pos, out_size - pos,
                            "  OS:           %s\n", kal_get_os_name());
    pos += libkern_snprintf(out + pos, out_size - pos,
                            "  Page Size:    %zu bytes\n", kal_get_page_size());
    pos += libkern_snprintf(out + pos, out_size - pos,
                            "  Total Memory: %llu KB\n",
                            (unsigned long long)(kal_get_total_memory() / 1024));
    pos += libkern_snprintf(out + pos, out_size - pos,
                            "  Available:    %llu KB\n",
                            (unsigned long long)(kal_get_available_memory() / 1024));

    return KERN_SUCCESS;
}

static kern_return_t _version_handler(int argc, char **argv,
                                       char *out, size_t out_size)
{
    (void)argc; (void)argv;

    libkern_snprintf(out, out_size,
                     "Maxwell Kernel v%s\n"
                     "MsWaiter v%s\n"
                     "Machine: %s\n"
                     "OS: %s\n",
                     MX_MONITOR_KVER,
                     MX_MONITOR_VERSION,
                     kal_get_machine_type(),
                     kal_get_os_name());
    return KERN_SUCCESS;
}

static kern_return_t _uptime_handler(int argc, char **argv,
                                      char *out, size_t out_size)
{
    (void)argc; (void)argv;

    libkern_snprintf(out, out_size,
                     "Uptime tracking not yet available\n");
    return KERN_SUCCESS;
}

static kern_return_t _cpu_handler(int argc, char **argv,
                                   char *out, size_t out_size)
{
    (void)argc; (void)argv;

    libkern_snprintf(out, out_size,
                     "CPU Information:\n"
                     "  Architecture: %s\n",
                     kal_get_machine_type());
    return KERN_SUCCESS;
}

static kern_return_t _modules_handler(int argc, char **argv,
                                       char *out, size_t out_size)
{
    (void)argc; (void)argv;

    libkern_snprintf(out, out_size,
                     "Loaded Modules:\n"
                     "  (module tracking not yet available)\n");
    return KERN_SUCCESS;
}

static kern_return_t _devices_handler(int argc, char **argv,
                                       char *out, size_t out_size)
{
    (void)argc; (void)argv;

    libkern_snprintf(out, out_size,
                     "Device List:\n"
                     "  (device enumeration not yet available)\n");
    return KERN_SUCCESS;
}

mx_cmd_t _sysinfo_cmd = {
    .name       = "sysinfo",
    .help_short = "Display comprehensive system information",
    .help_long  = "Usage: sysinfo\n"
                  "Shows kernel version, machine type, memory, and other info.",
    .handler    = _sysinfo_handler,
};

mx_cmd_t _version_cmd = {
    .name       = "version",
    .help_short = "Display kernel and monitor versions",
    .help_long  = "Usage: version\n"
                  "Shows the kernel and MsWaiter version strings.",
    .handler    = _version_handler,
};

mx_cmd_t _uptime_cmd = {
    .name       = "uptime",
    .help_short = "Display system uptime",
    .help_long  = "Usage: uptime\n"
                  "Shows how long the system has been running.",
    .handler    = _uptime_handler,
};

mx_cmd_t _cpu_cmd = {
    .name       = "cpu",
    .help_short = "Display CPU information",
    .help_long  = "Usage: cpu\n"
                  "Shows processor architecture and feature information.",
    .handler    = _cpu_handler,
};

mx_cmd_t _modules_cmd = {
    .name       = "modules",
    .help_short = "List loaded kernel modules",
    .help_long  = "Usage: modules\n"
                  "Show all currently loaded kernel modules/extensions.",
    .handler    = _modules_handler,
};

mx_cmd_t _devices_cmd = {
    .name       = "devices",
    .help_short = "List detected devices",
    .help_long  = "Usage: devices\n"
                  "Show all hardware devices detected by the kernel.",
    .handler    = _devices_handler,
};
