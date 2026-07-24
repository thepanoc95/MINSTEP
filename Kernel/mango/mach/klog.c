/*
 * mango/mach/klog.c
 *
 * Kernel message logging implementation.
 */

#include "klog.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

/* -----------------------------------------------------------------------
 *  Global state
 * ----------------------------------------------------------------------- */

int             klog_level = KLOG_INFO;
struct timespec _klog_boot_time = { 0, 0 };

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
