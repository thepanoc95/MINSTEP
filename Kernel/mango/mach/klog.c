/*
 * mango/mach/klog.c
 *
 * Kernel message logging implementation.
 *
 * Output format follows Linux/NetBSD dmesg conventions:
 *
 *     [    0.000000] MINSTEP v0.1.0 -- Mango Nanokernel
 *     [    0.000001] [ipc] bootstrap port ready (port 3)
 *     [    0.000002] [task] kernel task created (pid 42)
 *     [    1.234567] WARNING: [task] task table 80% full
 *     [    1.234568] error: [loader] failed to exec /bin/init
 *
 * Timestamps use 12-character fixed-width fields with 6 decimal places.
 * Severity labels (WARNING:, error:) appear for levels WARN and above.
 * Subsystem tags ([ipc], [task], etc.) are prepended by the caller using
 * the klog_sub_* macros defined in klog.h.
 */

#include "klog.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#if defined(MANGO_KAL_TIME_H)
#include "../kal/kal_time.h"
#else
#include <time.h>
#endif

/* -----------------------------------------------------------------------
 *  Global state
 * ----------------------------------------------------------------------- */

int             klog_level = KLOG_INFO;
static double   _klog_boot_seconds = 0.0;
static int      _klog_clock_initialized = 0;

/* -----------------------------------------------------------------------
 *  klog_init_clock
 *
 *  Capture the monotonic boot timestamp.  Idempotent -- only the
 *  first call takes effect.
 * ----------------------------------------------------------------------- */

void klog_init_clock(void)
{
    if (_klog_clock_initialized) return;
    _klog_clock_initialized = 1;

#if defined(MANGO_KAL_TIME_H)
    _klog_boot_seconds = kal_clock_seconds();
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    _klog_boot_seconds = (double)ts.tv_sec + (double)ts.tv_nsec / 1e9;
#endif
}

/* -----------------------------------------------------------------------
 *  klog_time_since_boot
 *
 *  Return seconds elapsed since klog_init_clock() was called.
 * ----------------------------------------------------------------------- */

double klog_time_since_boot(void)
{
#if defined(MANGO_KAL_TIME_H)
    return kal_clock_seconds() - _klog_boot_seconds;
#else
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    double now_sec = (double)now.tv_sec + (double)now.tv_nsec / 1e9;
    return now_sec - _klog_boot_seconds;
#endif
}

/* -----------------------------------------------------------------------
 *  klog_emit
 *
 *  Print a kernel message to stderr with a Linux/NetBSD-style timestamp.
 *
 *  Format:  [%12.6f] message
 *           %12.6f   -- 12-char field, 6 decimal places
 *                       e.g. [    0.000000]
 *
 *  For severity >= KLOG_WARN, a label is prepended:
 *      WARN   -> WARNING:
 *      ERR    -> error:
 *      CRIT   -> CRIT:
 *      ALERT  -> ALERT:
 *      EMERG  -> EMERGENCY:
 *      DEBUG  -> debug:
 *
 *  Subsystem tags (e.g. [ipc], [task]) are NOT added by klog_emit;
 *  they are part of the caller's format string, added via klog_sub_*
 *  macros in klog.h.
 * ----------------------------------------------------------------------- */

void klog_emit(int level, const char *fmt, ...)
{
    if (level > klog_level) return;

    double elapsed = klog_time_since_boot();

    /* Print timestamp + optional severity prefix */
    switch (level) {
    case KLOG_EMERG:
        fprintf(stderr, "[%12.6f] EMERGENCY: ", elapsed);
        break;
    case KLOG_ALERT:
        fprintf(stderr, "[%12.6f] ALERT: ", elapsed);
        break;
    case KLOG_CRIT:
        fprintf(stderr, "[%12.6f] CRIT: ", elapsed);
        break;
    case KLOG_ERR:
        fprintf(stderr, "[%12.6f] error: ", elapsed);
        break;
    case KLOG_WARN:
        fprintf(stderr, "[%12.6f] WARNING: ", elapsed);
        break;
    case KLOG_DEBUG:
        fprintf(stderr, "[%12.6f] debug: ", elapsed);
        break;
    default:
        fprintf(stderr, "[%12.6f] ", elapsed);
        break;
    }

    /* Print the message body */
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);

    fflush(stderr);
}
