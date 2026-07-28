/*
 * compat/kern/sched_prim.h
 *
 * Usermode compatibility shim for Mach scheduler primitives.
 *
 * The original Mach kernel provides assert_wait, thread_block,
 * and thread_wakeup for thread synchronization.  In the Maxxwell
 * usermode nanokernel, we implement these with KAL condition
 * variables (falling back to pthreads if KAL is unavailable).
 *
 * assert_wait(event, interruptible)  -- mark current thread as waiting
 * thread_block(cleanup)              -- block the current thread
 * thread_wakeup(event)               -- wake all threads waiting on event
 *
 * Original: osfmk/kern/sched_prim.h (partial)
 */

#ifndef MXWL_COMPAT_KERN_SCHED_PRIM_H
#define MXWL_COMPAT_KERN_SCHED_PRIM_H

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

#ifdef MXWL_KAL_THREAD_H
/* KAL threading is available */

#include "../kal/kal_thread.h"

typedef struct mxwl_wait_channel {
    event_t                 mw_event;
    kal_mutex_t             mw_mutex;
    kal_cond_t              mw_cond;
    boolean_t               mw_woken;
    struct mxwl_wait_channel *mw_next;
} mxwl_wait_channel_t;

extern mxwl_wait_channel_t *mxwl_wait_channels;

extern mxwl_wait_channel_t *mxwl_wait_find(event_t event);
extern void mxwl_assert_wait(event_t event, boolean_t interruptible);
extern void mxwl_thread_block(void (*cleanup)(void));
extern void mxwl_thread_wakeup(event_t event);

#else
/* Fallback to pthreads */

#include <pthread.h>

typedef struct mxwl_wait_channel {
    event_t                 mw_event;
    pthread_mutex_t         mw_mutex;
    pthread_cond_t          mw_cond;
    boolean_t               mw_woken;
    struct mxwl_wait_channel *mw_next;
} mxwl_wait_channel_t;

extern mxwl_wait_channel_t *mxwl_wait_channels;

extern mxwl_wait_channel_t *mxwl_wait_find(event_t event);
extern void mxwl_assert_wait(event_t event, boolean_t interruptible);
extern void mxwl_thread_block(void (*cleanup)(void));
extern void mxwl_thread_wakeup(event_t event);

#endif /* MXWL_KAL_THREAD_H */

#define assert_wait(event, intr)    mxwl_assert_wait((event), (intr))
#define thread_block(cleanup)       mxwl_thread_block((cleanup))
#define thread_wakeup(event)        mxwl_thread_wakeup((event))

/*
 * thread_go - mark a thread as runnable.
 * In the usermode nanokernel, this is a no-op since threads
 * are managed by the host POSIX scheduler.  The ith_state
 * field is checked by the caller to determine the result.
 */
struct mxwl_ipc_thread;
extern void thread_go(struct mxwl_ipc_thread *thread);

#ifdef __cplusplus
}
#endif

#endif /* MXWL_COMPAT_KERN_SCHED_PRIM_H */
