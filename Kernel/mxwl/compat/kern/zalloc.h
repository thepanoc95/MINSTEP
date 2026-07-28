/*
 * compat/kern/zalloc.h
 *
 * Usermode compatibility shim for Mach kernel zone allocator.
 *
 * Mach zones are fixed-size object caches.  For the initial port,
 * we implement them as pools backed by KAL memory allocation,
 * with per-zone metadata.  This is sufficient for the IPC
 * subsystem's needs (allocating ipc_object, ipc_port, ipc_space,
 * ipc_tree_entry).
 *
 * Future improvement: replace with a slab allocator for better
 * locality and O(1) alloc/free.
 */

#ifndef MXWL_COMPAT_KERN_ZALLOC_H
#define MXWL_COMPAT_KERN_ZALLOC_H

#ifdef __cplusplus
extern "C" {
#endif

/* Forward -- vm_offset_t / vm_size_t are defined in <mach/vm_types.h> */

#include <mach/vm_types.h>

/* -----------------------------------------------------------------------
 *  zone_t -- a simplified zone allocator
 *
 *  Each zone tracks a fixed-size object class.  We keep it simple:
 *  allocation is just KAL calloc; deallocation is just KAL free.
 *  A free-list could be added later for performance.
 * ----------------------------------------------------------------------- */

#ifdef MXWL_KAL_MEMORY_H
/* KAL memory is available */

#include "../kal/kal_memory.h"

typedef struct mxwl_zone {
    const char     *z_name;        /* human-readable name          */
    unsigned int    z_elem_size;    /* size of each element         */
    unsigned int    z_count;        /* number of elements allocated */
} mxwl_zone_t;

typedef mxwl_zone_t *zone_t;

static inline zone_t zinit(
    unsigned int    elem_size,
    unsigned int    max_elems,
    unsigned int    alloc_size,     /* ignored in usermode */
    const char     *name)
{
    zone_t z = (zone_t) kal_malloc(sizeof(mxwl_zone_t));
    if (z) {
        z->z_name      = name;
        z->z_elem_size = elem_size;
        z->z_count     = 0;
    }
    return z;
}

static inline void *zalloc(zone_t z)
{
    if (!z) return NULL;
    void *p = kal_calloc(1, z->z_elem_size);
    if (p) z->z_count++;
    return p;
}

static inline void zfree(zone_t z, vm_offset_t elem)
{
    if (!z || !elem) return;
    z->z_count--;
    kal_free((void *)elem);
}

static inline void zdestroy(zone_t z)
{
    if (z) kal_free(z);
}

#else
/* Fallback to stdlib */

#include <stdlib.h>
#include <string.h>

typedef struct mxwl_zone {
    const char     *z_name;
    unsigned int    z_elem_size;
    unsigned int    z_count;
} mxwl_zone_t;

typedef mxwl_zone_t *zone_t;

static inline zone_t zinit(
    unsigned int    elem_size,
    unsigned int    max_elems,
    unsigned int    alloc_size,
    const char     *name)
{
    zone_t z = (zone_t) malloc(sizeof(mxwl_zone_t));
    if (z) {
        z->z_name      = name;
        z->z_elem_size = elem_size;
        z->z_count     = 0;
    }
    return z;
}

static inline void *zalloc(zone_t z)
{
    if (!z) return NULL;
    void *p = calloc(1, z->z_elem_size);
    if (p) z->z_count++;
    return p;
}

static inline void zfree(zone_t z, vm_offset_t elem)
{
    if (!z || !elem) return;
    z->z_count--;
    free((void *)elem);
}

static inline void zdestroy(zone_t z)
{
    if (z) free(z);
}

#endif /* MXWL_KAL_MEMORY_H */

#ifdef __cplusplus
}
#endif

#endif /* MXWL_COMPAT_KERN_ZALLOC_H */
