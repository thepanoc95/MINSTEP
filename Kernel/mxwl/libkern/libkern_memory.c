#include "libkern_memory.h"

#include <stdlib.h>

void *libkern_malloc(size_t size)
{
    return malloc(size);
}

void *libkern_calloc(size_t nmemb, size_t size)
{
    return calloc(nmemb, size);
}

void *libkern_realloc(void *ptr, size_t size)
{
    return realloc(ptr, size);
}

void libkern_free(void *ptr)
{
    free(ptr);
}

void *libkern_aligned_alloc(size_t alignment, size_t size)
{
    void *ptr;
    if (posix_memalign(&ptr, alignment, size) != 0)
        return NULL;
    return ptr;
}

void libkern_aligned_free(void *ptr)
{
    free(ptr);
}
