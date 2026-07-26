/*
 * compat/kern/lock.h
 *
 * Usermode compatibility shim for Mach kernel locks.
 *
 * Maps Mach simple_lock operations to POSIX pthread_mutex_t.
 */

#ifndef MANGO_COMPAT_KERN_LOCK_H
#define MANGO_COMPAT_KERN_LOCK_H

#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -----------------------------------------------------------------------
 *  simple_lock_data
 * ----------------------------------------------------------------------- */

typedef struct {
    pthread_mutex_t lock;
    pthread_mutexattr_t attr;
} mango_simple_lock_data_t;

/*
 * In the real kernel, decl_simple_lock_data is used as:
 *   decl_simple_lock_data(, is_ref_lock_data)
 * The macro expands to include a type + name declaration.
 * The real macro definition ends with a semicolon:
 *   #define decl_simple_lock_data(class, name) class simple_lock_data_t name;
 * We follow the same convention.
 */

#define decl_simple_lock_data(var, name) \
    var mango_simple_lock_data_t name;

/* -----------------------------------------------------------------------
 *  simple_lock_init / simple_lock / simple_unlock / simple_lock_try
 * ----------------------------------------------------------------------- */

static inline void simple_lock_init(mango_simple_lock_data_t *l)
{
    pthread_mutexattr_init(&l->attr);
#ifdef MANGO_DEBUG_LOCKS
    pthread_mutexattr_settype(&l->attr, PTHREAD_MUTEX_ERRORCHECK);
#else
    pthread_mutexattr_settype(&l->attr, PTHREAD_MUTEX_NORMAL);
#endif
    pthread_mutex_init(&l->lock, &l->attr);
}

static inline void simple_lock(mango_simple_lock_data_t *l)
{
    pthread_mutex_lock(&l->lock);
}

static inline void simple_unlock(mango_simple_lock_data_t *l)
{
    pthread_mutex_unlock(&l->lock);
}

static inline int simple_lock_try(mango_simple_lock_data_t *l)
{
    return (pthread_mutex_trylock(&l->lock) == 0) ? 1 : 0;
}

static inline void simple_lock_destroy(mango_simple_lock_data_t *l)
{
    pthread_mutex_destroy(&l->lock);
    pthread_mutexattr_destroy(&l->attr);
}

#ifdef __cplusplus
}
#endif

#endif /* MANGO_COMPAT_KERN_LOCK_H */
