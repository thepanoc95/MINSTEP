/*
 * compat/kern/sched_prim.h
 *
 * Usermode compatibility shim for Mach scheduler primitives.
 *
 * The original Mach kernel provides assert_wait, thread_block,
 * and thread_wakeup for thread synchronization.  In the Mango
 * usermode nanokernel, we implement these with KAL condition
 * variables (falling back to pthreads if KAL is unavailable).
 *
 * assert_wait(event, interruptible)  -- mark current thread as waiting
 * thread_block(cleanup)              -- block the current thread
 * thread_wakeup(event)               -- wake all threads waiting on event
 *
 * Original: osfmk/kern/sched_prim.h (partial)
 */

#ifndef MANGO_COMPAT_KERN_SCHED_PRIM_H
#define MANGO_COMPAT_KERN_SCHED_PRIM_H

#include <mach/boolean.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -----------------------------------------------------------------------
 *  event_t -- opaque event handle (pointer-sized)
 * ----------------------------------------------------------------------- */

typedef void *event_t;

/* -----------------------------------------------------------------------
 *  Wait channel -- one per unique event value
 *
 *  We maintain a simple linked list of wait channels, one for each
 *  distinct event pointer.  For the initial usermode port this is
 *  sufficient; the IPC subsystem uses very few distinct wait events
 *  (typically just task/space pointers).
 * ----------------------------------------------------------------------- */

#ifdef MANGO_KAL_THREAD_H
/* KAL threading is available */

#include "../kal/kal_thread.h"

typedef struct mango_wait_channel {
    event_t                 mw_event;
    kal_mutex_t             mw_mutex;
    kal_cond_t              mw_cond;
    boolean_t               mw_woken;
    struct mango_wait_channel *mw_next;
} mango_wait_channel_t;

extern mango_wait_channel_t *mango_wait_channels;

extern mango_wait_channel_t *mango_wait_find(event_t event);
extern void mango_assert_wait(event_t event, boolean_t interruptible);
extern void mango_thread_block(void (*cleanup)(void));
extern void mango_thread_wakeup(event_t event);

#else
/* Fallback to pthreads */

#include <pthread.h>

typedef struct mango_wait_channel {
    event_t                 mw_event;
    pthread_mutex_t         mw_mutex;
    pthread_cond_t          mw_cond;
    boolean_t               mw_woken;
    struct mango_wait_channel *mw_next;
} mango_wait_channel_t;

extern mango_wait_channel_t *mango_wait_channels;

extern mango_wait_channel_t *mango_wait_find(event_t event);
extern void mango_assert_wait(event_t event, boolean_t interruptible);
extern void mango_thread_block(void (*cleanup)(void));
extern void mango_thread_wakeup(event_t event);

#endif /* MANGO_KAL_THREAD_H */

#define assert_wait(event, intr)    mango_assert_wait((event), (intr))
#define thread_block(cleanup)       mango_thread_block((cleanup))
#define thread_wakeup(event)        mango_thread_wakeup((event))

/*
 * thread_go - mark a thread as runnable.
 * In the usermode nanokernel, this is a no-op since threads
 * are managed by the host POSIX scheduler.  The ith_state
 * field is checked by the caller to determine the result.
 */
struct mango_ipc_thread;
extern void thread_go(struct mango_ipc_thread *thread);

#ifdef __cplusplus
}
#endif

#endif /* MANGO_COMPAT_KERN_SCHED_PRIM_H */
