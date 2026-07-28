#ifndef MXWL_LIBKERN_MEMORY_H
#define MXWL_LIBKERN_MEMORY_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

void *libkern_malloc(size_t size);
void *libkern_calloc(size_t nmemb, size_t size);
void *libkern_realloc(void *ptr, size_t size);
void  libkern_free(void *ptr);

void *libkern_aligned_alloc(size_t alignment, size_t size);
void  libkern_aligned_free(void *ptr);

#ifdef __cplusplus
}
#endif

#endif /* MXWL_LIBKERN_MEMORY_H */
