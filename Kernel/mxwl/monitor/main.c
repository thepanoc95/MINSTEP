#include "monitor.h"
#include "../mach/klog.h"
#include "../kal/kal.h"
#include "../server/manager.h"

#include <stdarg.h>
#include <stdio.h>

static int _monitor_running = 0;

static void _register_builtins(void);

kern_return_t mx_monitor_init(void)
{
    _monitor_running = 0;
    _register_builtins();

    klog_sub_info("mon", "MsWaiter v%s initialized\n", MX_MONITOR_VERSION);
    return KERN_SUCCESS;
}

static void _show_banner(void)
{
    mx_monitor_printf("\n");
    mx_monitor_printf("Welcome to the Maxxwell Operating System.\n");
    mx_monitor_printf("(%s %s)\n", MX_MONITOR_NAME, MX_MONITOR_VERSION);
    mx_monitor_printf("\n");
    mx_monitor_printf("Type \"help\" for available commands.\n");
    mx_monitor_printf("\n");
}

void mx_monitor_stop_loop(void)
{
    _monitor_running = 0;
}

void mx_monitor_run(void)
{
    _monitor_running = 1;

    kal_terminal_t term;
    int have_term = (kal_terminal_save(&term) == 0);
    if (have_term)
        kal_terminal_raw(&term);

    _show_banner();

    while (_monitor_running) {
        char line[MX_MONITOR_MAX_LINE];

        mx_monitor_printf("%s", MX_MONITOR_PROMPT);

        int n = mx_monitor_input(line, sizeof(line));
        if (n < 0)
            break;
        if (n == 0)
            continue;

        mx_monitor_history_add(line);
        mx_monitor_process(line);
    }

    if (have_term)
        kal_terminal_restore(&term);

    klog_sub_info("mon", "MsWaiter exiting\n");
}

void mx_monitor_printf(const char *fmt, ...)
{
    char buf[MX_MONITOR_OUT_BUF];
    va_list args;
    va_start(args, fmt);
    int n = vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    if (n > 0)
        mx_monitor_puts(buf);
}

void mx_monitor_puts(const char *s)
{
    kal_write(1, s, libkern_strlen(s));
}

void mx_monitor_putchar(int c)
{
    char ch = (char)c;
    kal_write(1, &ch, 1);
}

int mx_monitor_getchar(void)
{
    unsigned char ch;
    ssize_t n = kal_read(0, &ch, 1);
    if (n <= 0)
        return -1;
    return ch;
}

int mx_monitor_input(char *buf, size_t n)
{
    size_t pos = 0;

    while (pos < n - 1) {
        int c = mx_monitor_getchar();
        if (c < 0)
            return -1;

        if (c == '\r' || c == '\n') {
            mx_monitor_putchar('\r');
            mx_monitor_putchar('\n');
            break;
        }

        if (c == '\b' || c == 127) {
            if (pos > 0) {
                pos--;
                mx_monitor_putchar('\b');
                mx_monitor_putchar(' ');
                mx_monitor_putchar('\b');
            }
            continue;
        }

        if (c == '\t') {
            if (pos > 0) {
                char prefix[MX_MONITOR_MAX_LINE];
                libkern_memcpy(prefix, buf, pos);
                prefix[pos] = '\0';

                char completed[MX_MONITOR_MAX_LINE];
                int n_completed = mx_monitor_complete(prefix, completed, sizeof(completed));
                if (n_completed > 0) {
                    size_t clen = libkern_strlen(completed);
                    if (clen > pos) {
                        for (size_t i = pos; i < clen && pos < n - 1; i++) {
                            buf[pos++] = completed[i];
                            mx_monitor_putchar(completed[i]);
                        }
                    }
                }
            }
            continue;
        }

        if (c >= 32 && c <= 126) {
            buf[pos++] = (char)c;
            mx_monitor_putchar(c);
        }
    }

    buf[pos] = '\0';
    return (int)pos;
}

int mx_monitor_process(const char *line)
{
    if (!line || line[0] == '\0')
        return 0;

    char line_copy[MX_MONITOR_MAX_LINE];
    libkern_strncpy(line_copy, line, sizeof(line_copy));

    char *argv[MX_MONITOR_MAX_ARGS];
    int argc = 0;

    char *p = line_copy;
    while (*p && argc < MX_MONITOR_MAX_ARGS) {
        while (*p == ' ' || *p == '\t')
            p++;
        if (*p == '\0')
            break;
        argv[argc++] = p;
        while (*p && *p != ' ' && *p != '\t')
            p++;
        if (*p) {
            *p = '\0';
            p++;
        }
    }

    if (argc == 0)
        return 0;

    mx_cmd_t *cmd = mx_monitor_find_command(argv[0]);
    if (!cmd) {
        mx_monitor_printf("Unknown command: %s\n", argv[0]);
        return KERN_INVALID_ARGUMENT;
    }

    char out[MX_MONITOR_OUT_BUF];
    out[0] = '\0';
    kern_return_t kr = cmd->handler(argc, argv, out, sizeof(out));

    if (out[0])
        mx_monitor_printf("%s", out);

    if (kr != KERN_SUCCESS && kr != KERN_ABORTED) {
        if (out[0] == '\0')
            mx_monitor_printf("Command failed: %d\n", kr);
    }

    return kr;
}

#include "commands/help.h"
#include "commands/boot.h"
#include "commands/server.h"
#include "commands/memory.h"
#include "commands/ports.h"
#include "commands/tasks.h"
#include "commands/debug.h"
#include "commands/fs.h"
#include "commands/log.h"
#include "commands/sysinfo.h"

static void _register_builtins(void)
{
    extern mx_cmd_t _help_cmd;
    extern mx_cmd_t _boot_cmd;
    extern mx_cmd_t _halt_cmd;
    extern mx_cmd_t _reboot_cmd;
    extern mx_cmd_t _server_cmd;
    extern mx_cmd_t _start_cmd;
    extern mx_cmd_t _stop_cmd;
    extern mx_cmd_t _restart_cmd;
    extern mx_cmd_t _memory_cmd;
    extern mx_cmd_t _ports_cmd;
    extern mx_cmd_t _tasks_cmd;
    extern mx_cmd_t _threads_cmd;
    extern mx_cmd_t _debug_cmd;
    extern mx_cmd_t _panic_cmd;
    extern mx_cmd_t _trace_cmd;
    extern mx_cmd_t _signal_cmd;
    extern mx_cmd_t _call_cmd;
    extern mx_cmd_t _ls_cmd;
    extern mx_cmd_t _cd_cmd;
    extern mx_cmd_t _pwd_cmd;
    extern mx_cmd_t _cat_cmd;
    extern mx_cmd_t _mount_cmd;
    extern mx_cmd_t _unmount_cmd;
    extern mx_cmd_t _log_cmd;
    extern mx_cmd_t _history_cmd;
    extern mx_cmd_t _echo_cmd;
    extern mx_cmd_t _clear_cmd;
    extern mx_cmd_t _print_cmd;
    extern mx_cmd_t _set_cmd;
    extern mx_cmd_t _get_cmd;
    extern mx_cmd_t _sysinfo_cmd;
    extern mx_cmd_t _version_cmd;
    extern mx_cmd_t _uptime_cmd;
    extern mx_cmd_t _cpu_cmd;
    extern mx_cmd_t _modules_cmd;
    extern mx_cmd_t _devices_cmd;
    extern mx_cmd_t _continue_cmd;
    extern mx_cmd_t _load_cmd;
    extern mx_cmd_t _unload_cmd;
    extern mx_cmd_t _boot_cmd_line;

    mx_monitor_register_command(&_help_cmd);
    mx_monitor_register_command(&_boot_cmd);
    mx_monitor_register_command(&_halt_cmd);
    mx_monitor_register_command(&_reboot_cmd);
    mx_monitor_register_command(&_server_cmd);
    mx_monitor_register_command(&_start_cmd);
    mx_monitor_register_command(&_stop_cmd);
    mx_monitor_register_command(&_restart_cmd);
    mx_monitor_register_command(&_memory_cmd);
    mx_monitor_register_command(&_ports_cmd);
    mx_monitor_register_command(&_tasks_cmd);
    mx_monitor_register_command(&_threads_cmd);
    mx_monitor_register_command(&_debug_cmd);
    mx_monitor_register_command(&_panic_cmd);
    mx_monitor_register_command(&_trace_cmd);
    mx_monitor_register_command(&_signal_cmd);
    mx_monitor_register_command(&_call_cmd);
    mx_monitor_register_command(&_ls_cmd);
    mx_monitor_register_command(&_cd_cmd);
    mx_monitor_register_command(&_pwd_cmd);
    mx_monitor_register_command(&_cat_cmd);
    mx_monitor_register_command(&_mount_cmd);
    mx_monitor_register_command(&_unmount_cmd);
    mx_monitor_register_command(&_log_cmd);
    mx_monitor_register_command(&_history_cmd);
    mx_monitor_register_command(&_echo_cmd);
    mx_monitor_register_command(&_clear_cmd);
    mx_monitor_register_command(&_print_cmd);
    mx_monitor_register_command(&_set_cmd);
    mx_monitor_register_command(&_get_cmd);
    mx_monitor_register_command(&_sysinfo_cmd);
    mx_monitor_register_command(&_version_cmd);
    mx_monitor_register_command(&_uptime_cmd);
    mx_monitor_register_command(&_cpu_cmd);
    mx_monitor_register_command(&_modules_cmd);
    mx_monitor_register_command(&_devices_cmd);
    mx_monitor_register_command(&_continue_cmd);
    mx_monitor_register_command(&_load_cmd);
    mx_monitor_register_command(&_unload_cmd);
    mx_monitor_register_command(&_boot_cmd_line);
}
