#include "bootstrap.h"
#include "manager.h"
#include "registry.h"
#include "loader.h"
#include "ipc.h"
#include "../mach/klog.h"
#include "../mach/mach_kernel.h"
#include "../libkern/libkern.h"
#include "../kal/kal.h"
#include "../monitor/monitor.h"

#include <string.h>

static mx_bootstrap_config_t _boot_config;
static int _bootstrapped = 0;

static const char *_default_servers[] = {
    "/mach/servers/4bsd",
    "/mach/servers/DeviceKit",
    "/mach/servers/NetServer",
    "/mach/servers/AudioServer",
    NULL
};

kern_return_t mx_bootstrap_init(void)
{
    libkern_memset(&_boot_config, 0, sizeof(_boot_config));
    _boot_config.timeout_ms = 30000;
    _boot_config.autostart_servers = 1;
    _bootstrapped = 0;

    klog_sub_info("srv-boot", "bootstrap subsystem initialized\n");
    return KERN_SUCCESS;
}

kern_return_t mx_bootstrap_configure(const char *boot_args)
{
    if (!boot_args)
        return KERN_INVALID_ARGUMENT;

    const char *prefix = "-srv=";
    size_t plen = libkern_strlen(prefix);

    const char *arg = boot_args;
    while (arg && *arg) {
        while (*arg == ' ') arg++;

        if (libkern_strncmp(arg, prefix, plen) == 0) {
            const char *path = arg + plen;
            const char *end = libkern_strchr(path, ' ');
            size_t path_len = end ? (size_t)(end - path) : libkern_strlen(path);

            size_t copy = path_len < MX_SERVER_PATH_MAX - 1 ? path_len : MX_SERVER_PATH_MAX - 1;
            libkern_memcpy(_boot_config.server_path, path, copy);
            _boot_config.server_path[copy] = '\0';

            klog_sub_info("srv-boot", "bootstrap server: %s\n", _boot_config.server_path);
        }

        arg = libkern_strchr(arg, ' ');
        if (arg) arg++;
    }

    return KERN_SUCCESS;
}

kern_return_t mx_bootstrap_start(void)
{
    klog_info("Maxxwell bootstrap sequence starting...\n");

    kern_return_t kr;

    kr = mx_registry_init();
    if (kr != KERN_SUCCESS) {
        klog_err("registry init failed\n");
        return kr;
    }

    kr = mx_ipc_init();
    if (kr != KERN_SUCCESS) {
        klog_err("IPC init failed\n");
        return kr;
    }

    kr = mx_loader_init();
    if (kr != KERN_SUCCESS) {
        klog_err("loader init failed\n");
        return kr;
    }

    kr = mx_manager_init();
    if (kr != KERN_SUCCESS) {
        klog_err("manager init failed\n");
        return kr;
    }

    kr = mx_bootstrap_sequence();
    if (kr != KERN_SUCCESS) {
        klog_warn("bootstrap sequence incomplete (%d)\n", kr);
    }

    _bootstrapped = 1;
    klog_info("Maxxwell bootstrap sequence complete\n");
    return KERN_SUCCESS;
}

kern_return_t mx_bootstrap_start_server(const char *name, const char *path,
                                         uint32_t capabilities)
{
    if (!name)
        return KERN_INVALID_ARGUMENT;

    mx_server_t *existing = mx_server_lookup_name(name);
    if (existing)
        return KERN_NAME_EXISTS;

    mx_server_t *server = libkern_calloc(1, sizeof(mx_server_t));
    if (!server)
        return KERN_FAILURE;

    kern_return_t kr = mx_server_init(server, name);
    if (kr != KERN_SUCCESS) {
        libkern_free(server);
        return kr;
    }

    server->capabilities = capabilities;
    server->flags = MX_SERVER_SYSTEM | MX_SERVER_AUTOSTART;

    if (path)
        libkern_strncpy(server->path, path, MX_SERVER_PATH_MAX - 1);

    kr = mx_server_register(server);
    if (kr != KERN_SUCCESS) {
        libkern_free(server);
        return kr;
    }

    kr = mx_manager_start_server(server);
    if (kr != KERN_SUCCESS) {
        klog_sub_warn("srv-boot", "failed to start %s (%d)\n", name, kr);
    }

    return kr;
}

kern_return_t mx_bootstrap_launch_mswaiter(void)
{
    klog_sub_info("srv-boot", "launching MsWaiter kernel monitor...\n");

    mx_monitor_run();
    return KERN_SUCCESS;
}

static kern_return_t _start_default_servers(void)
{
    if (!_boot_config.autostart_servers)
        return KERN_SUCCESS;

    if (_boot_config.server_path[0] != '\0') {
        kern_return_t kr = mx_bootstrap_start_server(
            MX_BOOTSTRAP_NAME,
            _boot_config.server_path,
            MX_CAP_BOOTSTRAP | MX_CAP_PROCESS | MX_CAP_IPC);

        if (kr != KERN_SUCCESS) {
            klog_sub_warn("srv-boot", "configured bootstrap server failed (%d), starting MsWaiter\n", kr);
            return mx_bootstrap_launch_mswaiter();
        }

        return KERN_SUCCESS;
    }

    return mx_bootstrap_launch_mswaiter();
}

kern_return_t mx_bootstrap_sequence(void)
{
    kern_return_t kr = _start_default_servers();
    if (kr != KERN_SUCCESS) {
        klog_sub_err("srv-boot", "no servers available\n");
        return kr;
    }

    return KERN_SUCCESS;
}

const char *mx_bootstrap_get_server_path(void)
{
    return _boot_config.server_path[0] != '\0'
           ? _boot_config.server_path
           : NULL;
}
