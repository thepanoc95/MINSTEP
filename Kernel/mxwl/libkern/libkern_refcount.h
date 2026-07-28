#ifndef MXWL_LIBKERN_REFCOUNT_H
#define MXWL_LIBKERN_REFCOUNT_H

#include <stdint.h>
#include "libkern_macros.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct refcount {
    uint32_t refs;
} refcount_t;

#define REFCOUNT_INIT(n)  { (n) }
#define REFCOUNT_STATIC(n) { .refs = (n) }

static inline void refcount_init(refcount_t *rc, uint32_t n)
{
    rc->refs = n;
}

static inline void refcount_retain(refcount_t *rc)
{
    rc->refs++;
}

static inline uint32_t refcount_release(refcount_t *rc)
{
    if (rc->refs > 0)
        rc->refs--;
    return rc->refs;
}

static inline uint32_t refcount_get(const refcount_t *rc)
{
    return rc->refs;
}

static inline int refcount_is_valid(const refcount_t *rc)
{
    return rc->refs > 0;
}

static inline int refcount_is_one(const refcount_t *rc)
{
    return rc->refs == 1;
}

typedef struct refcount_atomic {
    volatile uint32_t refs;
} refcount_atomic_t;

#define REFCOUNT_ATOMIC_INIT(n) { (n) }

static inline void refcount_atomic_init(refcount_atomic_t *rc, uint32_t n)
{
    __atomic_store_n(&rc->refs, n, __ATOMIC_RELAXED);
}

static inline void refcount_atomic_retain(refcount_atomic_t *rc)
{
    __atomic_add_fetch(&rc->refs, 1, __ATOMIC_ACQ_REL);
}

static inline uint32_t refcount_atomic_release(refcount_atomic_t *rc)
{
    return __atomic_sub_fetch(&rc->refs, 1, __ATOMIC_ACQ_REL);
}

static inline uint32_t refcount_atomic_get(const refcount_atomic_t *rc)
{
    return __atomic_load_n(&rc->refs, __ATOMIC_ACQUIRE);
}

#ifdef __cplusplus
}
#endif

#endif /* MXWL_LIBKERN_REFCOUNT_H */
