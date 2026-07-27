/*
 * mango/kal/kal_time.h
 *
 * Kernel Abstraction Layer -- timing and clock.
 *
 * Abstracts monotonic clock access for kernel timestamps.
 */

#ifndef MANGO_KAL_TIME_H
#define MANGO_KAL_TIME_H

#include "kal_platform.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -----------------------------------------------------------------------
 *  Monotonic clock
 * ----------------------------------------------------------------------- */

/* High-resolution monotonic timestamp in nanoseconds since an arbitrary epoch.
 * The epoch is fixed for the lifetime of the process (i.e. time since boot
 * or since first call). */
uint64_t kal_clock_monotonic_ns(void);

/* Monotonic time in seconds (with fractional part in nanoseconds). */
typedef struct kal_timespec {
    uint64_t    tv_sec;
    uint32_t    tv_nsec;
} kal_timespec_t;

void kal_clock_monotonic(kal_timespec_t *ts);

/* Return monotonic time as a double (seconds since epoch). */
double kal_clock_seconds(void);

#ifdef __cplusplus
}
#endif

#endif /* MANGO_KAL_TIME_H */
