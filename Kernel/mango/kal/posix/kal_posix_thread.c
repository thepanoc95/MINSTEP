/*
 * mango/kal/posix/kal_posix_thread.c
 *
 * POSIX backend for KAL threading and synchronization.
 */

#include "../kal_thread.h"

#include <stdlib.h>
#include <pthread.h>
#include <sched.h>
#include <errno.h>

/* -----------------------------------------------------------------------
 *  Mutex
 * ----------------------------------------------------------------------- */

int kal_mutex_init(kal_mutex_t *m)
{
    pthread_mutex_t *impl = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
    if (!impl) return -1;

    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
#ifdef MANGO_DEBUG_LOCKS
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);
#else
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_NORMAL);
#endif

    int ret = pthread_mutex_init(impl, &attr);
    pthread_mutexattr_destroy(&attr);

    if (ret != 0) {
        free(impl);
        return -1;
    }

    m->_impl = impl;
    return 0;
}

int kal_mutex_destroy(kal_mutex_t *m)
{
    if (!m || !m->_impl) return -1;
    pthread_mutex_destroy((pthread_mutex_t *)m->_impl);
    free(m->_impl);
    m->_impl = NULL;
    return 0;
}

int kal_mutex_lock(kal_mutex_t *m)
{
    if (!m || !m->_impl) return -1;
    return pthread_mutex_lock((pthread_mutex_t *)m->_impl);
}

int kal_mutex_trylock(kal_mutex_t *m)
{
    if (!m || !m->_impl) return -1;
    return (pthread_mutex_trylock((pthread_mutex_t *)m->_impl) == 0) ? 0 : -1;
}

int kal_mutex_unlock(kal_mutex_t *m)
{
    if (!m || !m->_impl) return -1;
    return pthread_mutex_unlock((pthread_mutex_t *)m->_impl);
}

/* -----------------------------------------------------------------------
 *  Condition variable
 * ----------------------------------------------------------------------- */

int kal_cond_init(kal_cond_t *c)
{
    pthread_cond_t *impl = (pthread_cond_t *)malloc(sizeof(pthread_cond_t));
    if (!impl) return -1;

    if (pthread_cond_init(impl, NULL) != 0) {
        free(impl);
        return -1;
    }

    c->_impl = impl;
    return 0;
}

int kal_cond_destroy(kal_cond_t *c)
{
    if (!c || !c->_impl) return -1;
    pthread_cond_destroy((pthread_cond_t *)c->_impl);
    free(c->_impl);
    c->_impl = NULL;
    return 0;
}

int kal_cond_wait(kal_cond_t *c, kal_mutex_t *m)
{
    if (!c || !c->_impl || !m || !m->_impl) return -1;
    return pthread_cond_wait((pthread_cond_t *)c->_impl,
                             (pthread_mutex_t *)m->_impl);
}

int kal_cond_signal(kal_cond_t *c)
{
    if (!c || !c->_impl) return -1;
    return pthread_cond_signal((pthread_cond_t *)c->_impl);
}

int kal_cond_broadcast(kal_cond_t *c)
{
    if (!c || !c->_impl) return -1;
    return pthread_cond_broadcast((pthread_cond_t *)c->_impl);
}

/* -----------------------------------------------------------------------
 *  Yield
 * ----------------------------------------------------------------------- */

void kal_yield(void)
{
    sched_yield();
}
