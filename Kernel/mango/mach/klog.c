/*
 * mango/mach/klog.c
 *
 * Kernel message logging implementation.
 *
 * Boot-time output follows NeXTSTEP Mach conventions -- plain
 * descriptive text with no timestamps:
 *
 *     MINSTEP v0.1.0 -- Mango Nanokernel
 *     Copyright (c) 2026 Miguel V. Mesquita. BSD License.
 *     physical memory = 14.97 megabytes.
 *     Bootstrap port allocated (port 3).
 *     Task table initialized.
 *     host port 2, host privilege port 3.
 *     kernel initialization complete
 *
 * After boot, messages include a timestamp prefix:
 *
 *     [    0.000012] [task] process exited (pid 42, status 0)
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
int             klog_boot_mode = 1;
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
 *  Print a kernel message to stderr.
 *
 *  During boot (klog_boot_mode == 1), messages are printed as plain
 *  text with no timestamp prefix -- matching NeXTSTEP Mach style:
 *
 *      Bootstrap port allocated (port 3).
 *      Task table initialized.
 *
 *  After boot (klog_boot_mode == 0), a timestamp is prepended:
 *
 *      [    0.000012] [task] process exited (pid 42, status 0)
 *      [    1.234567] WARNING: task table 80% full
 * ----------------------------------------------------------------------- */

void klog_emit(int level, const char *fmt, ...)
{
    if (level > klog_level) return;

    if (klog_boot_mode) {
        /* NeXTSTEP style: plain text, no timestamp */
        switch (level) {
        case KLOG_EMERG:  fprintf(stderr, "EMERGENCY: "); break;
        case KLOG_ALERT:  fprintf(stderr, "ALERT: ");     break;
        case KLOG_CRIT:   fprintf(stderr, "CRIT: ");      break;
        case KLOG_ERR:    fprintf(stderr, "error: ");     break;
        case KLOG_WARN:   fprintf(stderr, "WARNING: ");   break;
        case KLOG_DEBUG:  fprintf(stderr, "debug: ");     break;
        default: break;
        }
    } else {
        /* Post-boot: timestamp prefix */
        double elapsed = klog_time_since_boot();
        switch (level) {
        case KLOG_EMERG:  fprintf(stderr, "[%12.6f] EMERGENCY: ", elapsed); break;
        case KLOG_ALERT:  fprintf(stderr, "[%12.6f] ALERT: ",     elapsed); break;
        case KLOG_CRIT:   fprintf(stderr, "[%12.6f] CRIT: ",      elapsed); break;
        case KLOG_ERR:    fprintf(stderr, "[%12.6f] error: ",     elapsed); break;
        case KLOG_WARN:   fprintf(stderr, "[%12.6f] WARNING: ",   elapsed); break;
        case KLOG_DEBUG:  fprintf(stderr, "[%12.6f] debug: ",     elapsed); break;
        default:          fprintf(stderr, "[%12.6f] ",            elapsed); break;
        }
    }

    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);

    fflush(stderr);
}
