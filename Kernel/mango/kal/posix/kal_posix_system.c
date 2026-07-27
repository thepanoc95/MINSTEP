/*
 * mango/kal/posix/kal_posix_system.c
 *
 * POSIX backend for KAL system information.
 */

#include "../kal_system.h"

#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <sys/utsname.h>

size_t kal_get_page_size(void)
{
    long ps = sysconf(_SC_PAGESIZE);
    return (ps > 0) ? (size_t)ps : 4096;
}

uint64_t kal_get_total_memory(void)
{
    long pages = sysconf(_SC_PHYS_PAGES);
    long page_size = sysconf(_SC_PAGESIZE);
    if (pages <= 0 || page_size <= 0) return 0;
    return (uint64_t)pages * (uint64_t)page_size;
}

uint64_t kal_get_available_memory(void)
{
    long pages = sysconf(_SC_AVPHYS_PAGES);
    long page_size = sysconf(_SC_PAGESIZE);
    if (pages <= 0 || page_size <= 0) return 0;
    return (uint64_t)pages * (uint64_t)page_size;
}

const char *kal_get_machine_type(void)
{
    static char machine[64] = {0};
    struct utsname u;

    if (machine[0]) return machine;

    if (uname(&u) == 0) {
        strncpy(machine, u.machine, sizeof(machine) - 1);
    } else {
        strncpy(machine, "unknown", sizeof(machine) - 1);
    }

    return machine;
}

const char *kal_get_os_name(void)
{
    static char osname[64] = {0};
    struct utsname u;

    if (osname[0]) return osname;

    if (uname(&u) == 0) {
        snprintf(osname, sizeof(osname), "%s %s", u.sysname, u.release);
    } else {
        strncpy(osname, "unknown", sizeof(osname) - 1);
    }

    return osname;
}

void kal_usleep(unsigned int usec)
{
    usleep((useconds_t)usec);
}
