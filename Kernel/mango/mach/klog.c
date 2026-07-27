/*
 * mango/mach/klog.c
 *
 * Kernel message logging implementation.
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
 *  Capture the boot timestamp.  Safe to call multiple times;
 *  only the first call takes effect.
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
 *  Print a kernel message to stderr with a NetBSD-style timestamp.
 *
 *  Format: [  %9.7f] message
 *          ^          ^
 *          |          +-- the message text
 *          +-- timestamp with padding
 *
 *  Example output:
 *      [   0.0000000] Mango Nanokernel 0.1.0
 *      [   0.0012345] initializing IPC...
 *      [   2.1165136] err: port table full
 * ----------------------------------------------------------------------- */

void klog_emit(int level, const char *fmt, ...)
{
    if (level > klog_level) return;

    double elapsed = klog_time_since_boot();

    /* Print timestamp prefix */
    fprintf(stderr, "[%10.7f] ", elapsed);

    /* Print the message */
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);

    fflush(stderr);
}
