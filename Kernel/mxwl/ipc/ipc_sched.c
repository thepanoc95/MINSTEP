/*
 * mxwl/ipc/ipc_sched.c
 *
 * Usermode implementation of Mach scheduler primitives.
 *
 * Provides: assert_wait, thread_block, thread_wakeup, thread_go
 *
 * In the real Mach kernel, these block/wake kernel threads.
 * In Maxxwell's usermode nanokernel, we use KAL synchronization
 * primitives (falling back to pthreads if KAL is unavailable).
 */

#include <stdlib.h>
#include <mach/boolean.h>
#include <kern/sched_prim.h>
#include <ipc/ipc_thread.h>

#ifdef MXWL_KAL_THREAD_H
#include <kal/kal.h>
#endif

/* -----------------------------------------------------------------------
 *  Global wait channel list
 * ----------------------------------------------------------------------- */

mxwl_wait_channel_t *mxwl_wait_channels = NULL;

/* Thread-local storage for the current wait channel */
static __thread mxwl_wait_channel_t *mxwl_current_wait = NULL;

/* -----------------------------------------------------------------------
 *  mxwl_wait_find -- find or create a wait channel for an event
 * ----------------------------------------------------------------------- */

mxwl_wait_channel_t *
mxwl_wait_find(event_t event)
{
	mxwl_wait_channel_t *ch;

	/* Search existing channels */
	for (ch = mxwl_wait_channels; ch != NULL; ch = ch->mw_next) {
		if (ch->mw_event == event)
			return ch;
	}

	/* Allocate a new channel */
#ifdef MXWL_KAL_THREAD_H
	ch = kal_calloc(1, sizeof(*ch));
#else
	ch = calloc(1, sizeof(*ch));
#endif
	if (ch == NULL)
		return NULL;

	ch->mw_event = event;
#ifdef MXWL_KAL_THREAD_H
	kal_mutex_init(&ch->mw_mutex);
	kal_cond_init(&ch->mw_cond);
#else
	pthread_mutex_init(&ch->mw_mutex, NULL);
	pthread_cond_init(&ch->mw_cond, NULL);
#endif
	ch->mw_woken = FALSE;
	ch->mw_next = mxwl_wait_channels;
	mxwl_wait_channels = ch;

	return ch;
}

/* -----------------------------------------------------------------------
 *  mxwl_assert_wait -- mark current thread as waiting on an event
 * ----------------------------------------------------------------------- */

void
mxwl_assert_wait(event_t event, boolean_t interruptible)
{
	mxwl_wait_channel_t *ch;

	(void)interruptible;

	ch = mxwl_wait_find(event);
	if (ch == NULL)
		return;

#ifdef MXWL_KAL_THREAD_H
	kal_mutex_lock(&ch->mw_mutex);
#else
	pthread_mutex_lock(&ch->mw_mutex);
#endif
	ch->mw_woken = FALSE;
	mxwl_current_wait = ch;
	/* Don't unlock -- thread_block will wait and then unlock */
}

/* -----------------------------------------------------------------------
 *  mxwl_thread_block -- block until woken or timeout
 * ----------------------------------------------------------------------- */

void
mxwl_thread_block(void (*cleanup)(void))
{
	mxwl_wait_channel_t *ch = mxwl_current_wait;

	(void)cleanup;

	if (ch == NULL) {
		/* Nothing to wait on -- just yield */
#ifdef MXWL_KAL_THREAD_H
		kal_yield();
#else
		sched_yield();
#endif
		return;
	}

	/* Wait for the event to be signaled */
#ifdef MXWL_KAL_THREAD_H
	while (!ch->mw_woken)
		kal_cond_wait(&ch->mw_cond, &ch->mw_mutex);
	kal_mutex_unlock(&ch->mw_mutex);
#else
	while (!ch->mw_woken)
		pthread_cond_wait(&ch->mw_cond, &ch->mw_mutex);
	pthread_mutex_unlock(&ch->mw_mutex);
#endif
	mxwl_current_wait = NULL;
}

/* -----------------------------------------------------------------------
 *  mxwl_thread_wakeup -- wake all threads waiting on an event
 * ----------------------------------------------------------------------- */

void
mxwl_thread_wakeup(event_t event)
{
	mxwl_wait_channel_t *ch;

	ch = mxwl_wait_find(event);
	if (ch == NULL)
		return;

#ifdef MXWL_KAL_THREAD_H
	kal_mutex_lock(&ch->mw_mutex);
	ch->mw_woken = TRUE;
	kal_cond_broadcast(&ch->mw_cond);
	kal_mutex_unlock(&ch->mw_mutex);
#else
	pthread_mutex_lock(&ch->mw_mutex);
	ch->mw_woken = TRUE;
	pthread_cond_broadcast(&ch->mw_cond);
	pthread_mutex_unlock(&ch->mw_mutex);
#endif
}

/* -----------------------------------------------------------------------
 *  thread_go -- mark a thread as runnable (no-op in usermode)
 *
 *  In the real Mach kernel, this puts a thread on the run queue.
 *  In usermode, the thread is managed by the host scheduler.
 *  The caller sets ith_state before calling this, so the blocked
 *  thread will see the updated state when it checks.
 * ----------------------------------------------------------------------- */

void
thread_go(struct mxwl_ipc_thread *thread)
{
	(void)thread;
	/* No-op in usermode -- the thread will check ith_state
	 * when it next runs.  In a more complete implementation,
	 * we could signal a per-thread condition variable here. */
}
