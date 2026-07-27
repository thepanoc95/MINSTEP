#ifndef MANGO_MACH_KLOG_H
#define MANGO_MACH_KLOG_H

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -----------------------------------------------------------------------
 *  Log levels (syslog-compatible)
 * ----------------------------------------------------------------------- */

#define KLOG_EMERG   0
#define KLOG_ALERT   1
#define KLOG_CRIT    2
#define KLOG_ERR     3
#define KLOG_WARN    4
#define KLOG_NOTICE  5
#define KLOG_INFO    6
#define KLOG_DEBUG   7

extern int klog_level;

/* -----------------------------------------------------------------------
 *  Core API
 * ----------------------------------------------------------------------- */

void klog_init_clock(void);
double klog_time_since_boot(void);

void klog_emit(int level, const char *fmt, ...)
    __attribute__((format(printf, 2, 3)));

/* -----------------------------------------------------------------------
 *  Level macros
 * ----------------------------------------------------------------------- */

#define klog_emerg(fmt, ...)  klog_emit(KLOG_EMERG,  fmt, ##__VA_ARGS__)
#define klog_alert(fmt, ...)  klog_emit(KLOG_ALERT,  fmt, ##__VA_ARGS__)
#define klog_crit(fmt, ...)   klog_emit(KLOG_CRIT,   fmt, ##__VA_ARGS__)
#define klog_err(fmt, ...)    klog_emit(KLOG_ERR,    fmt, ##__VA_ARGS__)
#define klog_warn(fmt, ...)   klog_emit(KLOG_WARN,   fmt, ##__VA_ARGS__)
#define klog_notice(fmt, ...) klog_emit(KLOG_NOTICE, fmt, ##__VA_ARGS__)
#define klog_info(fmt, ...)   klog_emit(KLOG_INFO,   fmt, ##__VA_ARGS__)
#define klog_debug(fmt, ...)  klog_emit(KLOG_DEBUG,  fmt, ##__VA_ARGS__)

/* -----------------------------------------------------------------------
 *  Subsystem-tagged macros
 *
 *  These prepend a [subsystem] tag to the message, producing output like:
 *      [    0.000012] [ipc] bootstrap port ready (port 3)
 *      [    0.000023] [task] kernel task created (pid 42)
 * ----------------------------------------------------------------------- */

#define klog_sub_info(sub, fmt, ...)   klog_emit(KLOG_INFO,   "[" sub "] " fmt, ##__VA_ARGS__)
#define klog_sub_notice(sub, fmt, ...) klog_emit(KLOG_NOTICE, "[" sub "] " fmt, ##__VA_ARGS__)
#define klog_sub_warn(sub, fmt, ...)   klog_emit(KLOG_WARN,   "[" sub "] " fmt, ##__VA_ARGS__)
#define klog_sub_err(sub, fmt, ...)    klog_emit(KLOG_ERR,    "[" sub "] " fmt, ##__VA_ARGS__)
#define klog_sub_crit(sub, fmt, ...)   klog_emit(KLOG_CRIT,   "[" sub "] " fmt, ##__VA_ARGS__)
#define klog_sub_debug(sub, fmt, ...)  klog_emit(KLOG_DEBUG,  "[" sub "] " fmt, ##__VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif /* MANGO_MACH_KLOG_H */
