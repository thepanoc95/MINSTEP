/*
 * compat/kern/kalloc.h
 *
 * Usermode compatibility shim for Mach kernel kalloc.
 *
 * Maps to standard malloc/free.  Used by ipc_hash.c to allocate
 * the global reverse hash table.
 */

#ifndef MXWL_COMPAT_KERN_KALLOC_H
#define MXWL_COMPAT_KERN_KALLOC_H

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

static inline void *kalloc(unsigned long size)
{
    return calloc(1, size);
}

static inline void kfree(void *data, unsigned long size)
{
    (void)size;
    free(data);
}

#ifdef __cplusplus
}
#endif

#endif /* MXWL_COMPAT_KERN_KALLOC_H */
