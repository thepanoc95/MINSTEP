/*
 * compat/kern/zalloc.h
 *
 * Usermode compatibility shim for Mach kernel zone allocator.
 *
 * Mach zones are fixed-size object caches.  For the initial port,
 * we implement them as pools backed by malloc, with per-zone
 * metadata.  This is sufficient for the IPC subsystem's needs
 * (allocating ipc_object, ipc_port, ipc_space, ipc_tree_entry).
 *
 * Future improvement: replace with a slab allocator for better
 * locality and O(1) alloc/free.
 */

#ifndef MANGO_COMPAT_KERN_ZALLOC_H
#define MANGO_COMPAT_KERN_ZALLOC_H

#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward -- vm_offset_t / vm_size_t are defined in <mach/vm_types.h> */

#include <mach/vm_types.h>

/* -----------------------------------------------------------------------
 *  zone_t -- a simplified zone allocator
 *
 *  Each zone tracks a fixed-size object class.  We keep it simple:
 *  allocation is just malloc; deallocation is just free.
 *  A free-list could be added later for performance.
 * ----------------------------------------------------------------------- */

typedef struct mango_zone {
    const char     *z_name;        /* human-readable name          */
    unsigned int    z_elem_size;    /* size of each element         */
    unsigned int    z_count;        /* number of elements allocated */
} mango_zone_t;

typedef mango_zone_t *zone_t;

/* -----------------------------------------------------------------------
 *  Zone primitives
 * ----------------------------------------------------------------------- */

static inline zone_t zinit(
    unsigned int    elem_size,
    unsigned int    max_elems,
    unsigned int    alloc_size,     /* ignored in usermode */
    const char     *name)
{
    zone_t z = (zone_t) malloc(sizeof(mango_zone_t));
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

#ifdef __cplusplus
}
#endif

#endif /* MANGO_COMPAT_KERN_ZALLOC_H */
