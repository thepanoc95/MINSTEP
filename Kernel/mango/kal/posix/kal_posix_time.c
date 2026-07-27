/*
 * mango/kal/posix/kal_posix_time.c
 *
 * POSIX backend for KAL timing and clock.
 */

#include "../kal_time.h"

#include <time.h>

static struct timespec _kal_boot_time;
static int _kal_boot_time_initialized = 0;

static void _kal_ensure_boot_time(void)
{
    if (!_kal_boot_time_initialized) {
        clock_gettime(CLOCK_MONOTONIC, &_kal_boot_time);
        _kal_boot_time_initialized = 1;
    }
}

uint64_t kal_clock_monotonic_ns(void)
{
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);

    _kal_ensure_boot_time();

    uint64_t sec  = (uint64_t)(now.tv_sec  - _kal_boot_time.tv_sec);
    uint64_t nsec = (uint64_t)(now.tv_nsec - _kal_boot_time.tv_nsec);
    return sec * 1000000000ULL + nsec;
}

void kal_clock_monotonic(kal_timespec_t *ts)
{
    _kal_ensure_boot_time();

    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);

    ts->tv_sec  = (uint64_t)(now.tv_sec - _kal_boot_time.tv_sec);
    ts->tv_nsec = (uint32_t)(now.tv_nsec - _kal_boot_time.tv_nsec);
}

double kal_clock_seconds(void)
{
    return (double)kal_clock_monotonic_ns() / 1e9;
}
