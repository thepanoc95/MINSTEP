/*
 * compat/kern/lock.h
 *
 * Usermode compatibility shim for Mach kernel locks.
 *
 * Maps Mach simple_lock operations to KAL mutex primitives.
 * Falls back to POSIX pthreads if KAL is not available.
 */

#ifndef MXWL_COMPAT_KERN_LOCK_H
#define MXWL_COMPAT_KERN_LOCK_H

#ifdef __cplusplus
extern "C" {
#endif

/* -----------------------------------------------------------------------
 *  simple_lock_data
 *
 *  On platforms with the KAL, this wraps kal_mutex_t.
 *  On platforms without the KAL, it falls back to pthreads.
 * ----------------------------------------------------------------------- */

#ifdef MXWL_KAL_THREAD_H
/* KAL is available -- use kal_mutex_t */

#include "../kal/kal_thread.h"

typedef struct {
    kal_mutex_t lock;
} mxwl_simple_lock_data_t;

#define decl_simple_lock_data(var, name) \
    var mxwl_simple_lock_data_t name;

static inline void simple_lock_init(mxwl_simple_lock_data_t *l)
{
    kal_mutex_init(&l->lock);
}

static inline void simple_lock(mxwl_simple_lock_data_t *l)
{
    kal_mutex_lock(&l->lock);
}

static inline void simple_unlock(mxwl_simple_lock_data_t *l)
{
    kal_mutex_unlock(&l->lock);
}

static inline int simple_lock_try(mxwl_simple_lock_data_t *l)
{
    return (kal_mutex_trylock(&l->lock) == 0) ? 1 : 0;
}

static inline void simple_lock_destroy(mxwl_simple_lock_data_t *l)
{
    kal_mutex_destroy(&l->lock);
}

#else
/* KAL not available -- direct pthreads */

#include <pthread.h>

typedef struct {
    pthread_mutex_t lock;
    pthread_mutexattr_t attr;
} mxwl_simple_lock_data_t;

#define decl_simple_lock_data(var, name) \
    var mxwl_simple_lock_data_t name;

static inline void simple_lock_init(mxwl_simple_lock_data_t *l)
{
    pthread_mutexattr_init(&l->attr);
#ifdef MXWL_DEBUG_LOCKS
    pthread_mutexattr_settype(&l->attr, PTHREAD_MUTEX_ERRORCHECK);
#else
    pthread_mutexattr_settype(&l->attr, PTHREAD_MUTEX_NORMAL);
#endif
    pthread_mutex_init(&l->lock, &l->attr);
}

static inline void simple_lock(mxwl_simple_lock_data_t *l)
{
    pthread_mutex_lock(&l->lock);
}

static inline void simple_unlock(mxwl_simple_lock_data_t *l)
{
    pthread_mutex_unlock(&l->lock);
}

static inline int simple_lock_try(mxwl_simple_lock_data_t *l)
{
    return (pthread_mutex_trylock(&l->lock) == 0) ? 1 : 0;
}

static inline void simple_lock_destroy(mxwl_simple_lock_data_t *l)
{
    pthread_mutex_destroy(&l->lock);
    pthread_mutexattr_destroy(&l->attr);
}

#endif /* MXWL_KAL_THREAD_H */

#ifdef __cplusplus
}
#endif

#endif /* MXWL_COMPAT_KERN_LOCK_H */
