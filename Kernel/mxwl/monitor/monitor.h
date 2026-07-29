#ifndef MXWL_MONITOR_MONITOR_H
#define MXWL_MONITOR_MONITOR_H

#include "../mach/mach_types.h"
#include "../libkern/libkern.h"

#define MX_MONITOR_NAME      "MsWaiter"
#define MX_MONITOR_VERSION   "2.0"
#define MX_MONITOR_KVER      "0.1.12-rc1"
#define MX_MONITOR_PROMPT    "----> "
#define MX_MONITOR_MAX_LINE  512
#define MX_MONITOR_MAX_ARGS  64
#define MX_MONITOR_HISTORY   64
#define MX_MONITOR_OUT_BUF   4096

typedef kern_return_t (*mx_cmd_handler_t)(int argc, char **argv,
                                           char *out, size_t out_size);

typedef struct mx_cmd {
    const char       *name;
    const char       *help_short;
    const char       *help_long;
    mx_cmd_handler_t  handler;
    struct mx_cmd    *next;
} mx_cmd_t;

kern_return_t mx_monitor_init(void);
void          mx_monitor_run(void);
void          mx_monitor_printf(const char *fmt, ...)
                __attribute__((format(printf, 1, 2)));
void          mx_monitor_puts(const char *s);
void          mx_monitor_putchar(int c);
int           mx_monitor_getchar(void);
int           mx_monitor_input(char *buf, size_t n);
int           mx_monitor_process(const char *line);

kern_return_t mx_monitor_register_command(mx_cmd_t *cmd);
mx_cmd_t    *mx_monitor_find_command(const char *name);
mx_cmd_t    *mx_monitor_first_command(void);
mx_cmd_t    *mx_monitor_next_command(mx_cmd_t *cmd);

int mx_monitor_history_add(const char *line);
int mx_monitor_history_count(void);
const char *mx_monitor_history_get(int index);
const char *mx_monitor_history_prev(void);
const char *mx_monitor_history_next(void);

int mx_monitor_complete(const char *prefix, char *buf, size_t n);

#endif
