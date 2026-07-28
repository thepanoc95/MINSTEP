/*
 * mxwl/kal/kal_thread.h
 *
 * Kernel Abstraction Layer -- threading and synchronization.
 *
 * Provides mutexes, condition variables, and thread yield
 * so the compat/ scheduler primitives can be ported easily.
 */

#ifndef MXWL_KAL_THREAD_H
#define MXWL_KAL_THREAD_H

#include "kal_platform.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -----------------------------------------------------------------------
 *  Mutex
 * ----------------------------------------------------------------------- */

typedef struct kal_mutex {
    void *_impl;    /* Backend-specific (e.g. pthread_mutex_t*) */
} kal_mutex_t;

/* Initialize a mutex. */
int kal_mutex_init(kal_mutex_t *m);

/* Destroy a mutex. */
int kal_mutex_destroy(kal_mutex_t *m);

/* Lock a mutex. */
int kal_mutex_lock(kal_mutex_t *m);

/* Try to lock a mutex.  Returns 0 if locked, -1 if not. */
int kal_mutex_trylock(kal_mutex_t *m);

/* Unlock a mutex. */
int kal_mutex_unlock(kal_mutex_t *m);

/* -----------------------------------------------------------------------
 *  Condition variable
 * ----------------------------------------------------------------------- */

typedef struct kal_cond {
    void *_impl;    /* Backend-specific (e.g. pthread_cond_t*) */
} kal_cond_t;

/* Initialize a condition variable. */
int kal_cond_init(kal_cond_t *c);

/* Destroy a condition variable. */
int kal_cond_destroy(kal_cond_t *c);

/* Wait on a condition variable (must hold associated mutex). */
int kal_cond_wait(kal_cond_t *c, kal_mutex_t *m);

/* Signal one waiter. */
int kal_cond_signal(kal_cond_t *c);

/* Wake all waiters. */
int kal_cond_broadcast(kal_cond_t *c);

/* -----------------------------------------------------------------------
 *  Thread yield
 * ----------------------------------------------------------------------- */

/* Yield the current thread's time slice. */
void kal_yield(void);

#ifdef __cplusplus
}
#endif

#endif /* MXWL_KAL_THREAD_H */
