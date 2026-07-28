/*
 * compat/ipc/ipc_mqueue.h
 *
 * Usermode compatibility shim for Mach message queues.
 *
 * Original: osfmk/kernel/ipc/ipc_mqueue.h
 */

#ifndef MXWL_COMPAT_IPC_IPC_MQUEUE_H
#define MXWL_COMPAT_IPC_IPC_MQUEUE_H

#include <mach/message.h>
#include <kern/assert.h>
#include <kern/lock.h>
#include <kern/macro_help.h>
#include <ipc/ipc_kmsg.h>
#include <ipc/ipc_thread.h>

typedef struct ipc_mqueue {
	decl_simple_lock_data(, imq_lock_data)
	struct ipc_kmsg_queue imq_messages;
	struct ipc_thread_queue imq_threads;
} *ipc_mqueue_t;

#define	IMQ_NULL		((ipc_mqueue_t) 0)

#define	imq_lock_init(mq)	simple_lock_init(&(mq)->imq_lock_data)
#define	imq_lock(mq)		simple_lock(&(mq)->imq_lock_data)
#define	imq_lock_try(mq)	simple_lock_try(&(mq)->imq_lock_data)
#define	imq_unlock(mq)		simple_unlock(&(mq)->imq_lock_data)

extern void
ipc_mqueue_init(ipc_mqueue_t mqueue);

extern void
ipc_mqueue_move(ipc_mqueue_t dest, ipc_mqueue_t src, ipc_port_t port);

extern void
ipc_mqueue_changed(ipc_mqueue_t mqueue, mach_msg_return_t result);

extern mach_msg_return_t
ipc_mqueue_send(ipc_kmsg_t kmsg, mach_msg_option_t option,
		mach_msg_timeout_t time_out);

#define	IMQ_NULL_CONTINUE	((void (*)()) 0)

extern mach_msg_return_t
ipc_mqueue_receive(ipc_mqueue_t mqueue, mach_msg_option_t option,
		   mach_msg_size_t max_size, mach_msg_timeout_t time_out,
		   boolean_t resume, void (*continuation)(void),
		   ipc_kmsg_t *kmsgp, mach_port_seqno_t *seqnop);

#include <kern/assert.h>

#if	MACH_ASSERT

#define	ipc_mqueue_send_always(kmsg)					\
MACRO_BEGIN								\
	mach_msg_return_t mr;						\
									\
	mr = ipc_mqueue_send((kmsg), MACH_SEND_ALWAYS,			\
			     MACH_MSG_TIMEOUT_NONE);			\
	assert(mr == MACH_MSG_SUCCESS);					\
MACRO_END

#else	/* !MACH_ASSERT */

#define	ipc_mqueue_send_always(kmsg)					\
MACRO_BEGIN								\
	(void) ipc_mqueue_send((kmsg), MACH_SEND_ALWAYS,		\
			       MACH_MSG_TIMEOUT_NONE);			\
MACRO_END

#endif	/* MACH_ASSERT */

#endif	/* MXWL_COMPAT_IPC_IPC_MQUEUE_H */
