/*
 * mango/kal/kal_system.h
 *
 * Kernel Abstraction Layer -- system information.
 *
 * Abstracts system queries (page size, memory info, machine type)
 * so the kernel can query host resources portably.
 */

#ifndef MANGO_KAL_SYSTEM_H
#define MANGO_KAL_SYSTEM_H

#include "kal_platform.h"
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -----------------------------------------------------------------------
 *  Memory information
 * ----------------------------------------------------------------------- */

/* Get the system page size in bytes. */
size_t kal_get_page_size(void);

/* Get total physical memory in bytes.  Returns 0 if unknown. */
uint64_t kal_get_total_memory(void);

/* Get available physical memory in bytes.  Returns 0 if unknown. */
uint64_t kal_get_available_memory(void);

/* -----------------------------------------------------------------------
 *  Machine identification
 * ----------------------------------------------------------------------- */

/* Get a string identifying the machine architecture (e.g. "x86_64", "arm64"). */
const char *kal_get_machine_type(void);

/* Get a string identifying the OS name (e.g. "linux", "darwin"). */
const char *kal_get_os_name(void);

/* -----------------------------------------------------------------------
 *  Sleep
 * ----------------------------------------------------------------------- */

/* Sleep for the given number of microseconds. */
void kal_usleep(unsigned int usec);

#ifdef __cplusplus
}
#endif

#endif /* MANGO_KAL_SYSTEM_H */
