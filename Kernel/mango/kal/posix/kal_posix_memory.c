/*
 * mango/kal/posix/kal_posix_memory.c
 *
 * POSIX backend for KAL memory management.
 */

#include "../kal_memory.h"

#include <stdlib.h>

void *kal_calloc(size_t count, size_t size)
{
    return calloc(count, size);
}

void *kal_malloc(size_t size)
{
    return malloc(size);
}

void *kal_realloc(void *ptr, size_t new_size)
{
    return realloc(ptr, new_size);
}

void kal_free(void *ptr)
{
    free(ptr);
}
