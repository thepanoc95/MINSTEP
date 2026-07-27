/*
 * mango/kal/kal_memory.h
 *
 * Kernel Abstraction Layer -- memory management.
 *
 * Thin wrappers around host memory allocation.  On platforms
 * without a standard C library, these can be backed by a
 * custom heap or a fixed-size memory pool.
 */

#ifndef MANGO_KAL_MEMORY_H
#define MANGO_KAL_MEMORY_H

#include "kal_platform.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Allocate zero-initialized memory of (count * size) bytes. */
void *kal_calloc(size_t count, size_t size);

/* Allocate uninitialized memory. */
void *kal_malloc(size_t size);

/* Reallocate memory. */
void *kal_realloc(void *ptr, size_t new_size);

/* Free memory. */
void  kal_free(void *ptr);

#ifdef __cplusplus
}
#endif

#endif /* MANGO_KAL_MEMORY_H */
