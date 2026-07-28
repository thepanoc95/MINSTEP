/*
 * mxwl/ipc/ipc_stubs.c
 *
 * Stub implementations for Mach 4 IPC functions that are referenced
 * by ipc_port.c but not yet fully ported.  These will be replaced
 * with real implementations in later phases.
 *
 * Functions provided:
 *   - ipc_object_alloc / ipc_object_alloc_name / ipc_object_copyout
 *   - ipc_notify_* (all notification functions)
 *   - ipc_mqueue_init / ipc_mqueue_changed
 *   - ipc_target_init / ipc_target_terminate
 *   - ipc_pset_remove
 *   - ipc_kmsg_destroy / ipc_kmsg_free / ipc_kmsg_dequeue
 *   - ipc_kobject_destroy
 *   - ipc_thread_dequeue / ipc_thread_enqueue / ipc_thread_rmqueue
 *   - ipc_kmsg_enqueue / ipc_kmsg_rmqueue / ipc_kmsg_queue_next
 *   - ipc_marequest_* (message-accepted request stubs)
 */

#include <stdlib.h>
#include <string.h>
#include <mach_ipc_compat.h>
#include <norma_ipc.h>

#include <mach/boolean.h>
#include <mach/kern_return.h>
#include <mach/port.h>
#include <mach/message.h>
#include <kern/lock.h>
#include <kern/zalloc.h>
#include <kern/sched_prim.h>
#include <kern/ipc_kobject.h>
#include <ipc/ipc_types.h>
#include <ipc/ipc_object.h>
#include <ipc/ipc_table.h>
#include <ipc/ipc_entry.h>
#include <ipc/ipc_space.h>
#include <ipc/ipc_port.h>
#include <ipc/ipc_pset.h>
#include <ipc/ipc_thread.h>
#include <ipc/ipc_mqueue.h>
#include <ipc/ipc_kmsg.h>
#include <ipc/ipc_notify.h>
#include <ipc/ipc_target.h>
#include <ipc/ipc_kmsg_queue.h>

/* =======================================================================
 *  Thread queue operations
 * ======================================================================= */

ipc_thread_t
ipc_thread_dequeue(ipc_thread_queue_t queue)
{
	ipc_thread_t thread;

	thread = ipc_thread_queue_first(queue);
	if (thread == ITH_NULL)
		return ITH_NULL;

	ipc_thread_rmqueue_first_macro(queue, thread);
	return thread;
}

void
ipc_thread_enqueue(ipc_thread_queue_t queue, ipc_thread_t thread)
{
	ipc_thread_enqueue_macro(queue, thread);
}

void
ipc_thread_rmqueue(ipc_thread_queue_t queue, ipc_thread_t thread)
{
	ipc_thread_rmqueue_first_macro(queue, thread);
}

/* =======================================================================
 *  Kernel message queue operations
 * ======================================================================= */

ipc_kmsg_t
ipc_kmsg_dequeue(ipc_kmsg_queue_t queue)
{
	ipc_kmsg_t kmsg;

	if (ipc_kmsg_queue_empty(queue))
		return IKM_NULL;

	kmsg = ipc_kmsg_queue_first(queue);
	ipc_kmsg_rmqueue_first_macro(queue, kmsg);
	return kmsg;
}

void
ipc_kmsg_enqueue(ipc_kmsg_queue_t queue, ipc_kmsg_t kmsg)
{
	ipc_kmsg_enqueue_macro(queue, kmsg);
}

void
ipc_kmsg_rmqueue(ipc_kmsg_queue_t queue, ipc_kmsg_t kmsg)
{
	ipc_kmsg_rmqueue_first_macro(queue, kmsg);
}

ipc_kmsg_t
ipc_kmsg_queue_next(ipc_kmsg_queue_t queue, ipc_kmsg_t kmsg)
{
	ipc_kmsg_t next;

	assert(kmsg != IKM_NULL);
	next = kmsg->ikm_next;
	if (next == ipc_kmsg_queue_first(queue))
		return IKM_NULL;
	return next;
}

void
ipc_kmsg_destroy(ipc_kmsg_t kmsg)
{
	if (kmsg == IKM_NULL)
		return;

	/* Release any msg-accepted request */
	if (kmsg->ikm_marequest != IMAR_NULL) {
		ipc_marequest_destroy(kmsg->ikm_marequest);
		kmsg->ikm_marequest = IMAR_NULL;
	}

	ikm_free(kmsg);
}

void
ipc_kmsg_clean(ipc_kmsg_t kmsg)
{
	(void)kmsg;
}

void
ipc_kmsg_free(ipc_kmsg_t kmsg)
{
	ikm_free(kmsg);
}

/* =======================================================================
 *  IPC object operations
 * ======================================================================= */

kern_return_t
ipc_object_alloc(
	ipc_space_t		space,
	ipc_object_type_t	otype,
	mach_port_type_t	obj_type,
	mach_port_nsrequest_t	nsreq,
	mach_port_t		*namep,
	ipc_object_t		*objp)
{
	ipc_object_t object;
	ipc_entry_t entry;
	mach_port_t name;
	kern_return_t kr;

	(void)nsreq;

	object = io_alloc(otype);
	if (object == IO_NULL)
		return KERN_RESOURCE_SHORTAGE;

	io_lock_init(object);
	object->io_references = 1;
	object->io_bits = io_makebits(TRUE, otype, 0);

	kr = ipc_entry_alloc(space, &name, &entry);
	if (kr != KERN_SUCCESS) {
		io_free(otype, object);
		return kr;
	}

	entry->ie_object = object;
	entry->ie_bits |= obj_type;

	is_write_unlock(space);

	*namep = name;
	*objp = object;
	return KERN_SUCCESS;
}

kern_return_t
ipc_object_alloc_name(
	ipc_space_t		space,
	ipc_object_type_t	otype,
	mach_port_type_t	obj_type,
	mach_port_nsrequest_t	nsreq,
	mach_port_t		name,
	ipc_object_t		*objp)
{
	ipc_object_t object;
	ipc_entry_t entry;
	kern_return_t kr;

	(void)nsreq;

	object = io_alloc(otype);
	if (object == IO_NULL)
		return KERN_RESOURCE_SHORTAGE;

	io_lock_init(object);
	object->io_references = 1;
	object->io_bits = io_makebits(TRUE, otype, 0);

	kr = ipc_entry_alloc_name(space, name, &entry);
	if (kr != KERN_SUCCESS) {
		io_free(otype, object);
		return kr;
	}

	entry->ie_object = object;
	entry->ie_bits |= obj_type;

	is_write_unlock(space);

	*objp = object;
	return KERN_SUCCESS;
}

kern_return_t
ipc_object_copyout(
	ipc_space_t		space,
	ipc_object_t		obj,
	mach_msg_type_name_t	msgt_name,
	boolean_t		copy,
	mach_port_t		*namep)
{
	ipc_entry_t entry;
	mach_port_t name;
	kern_return_t kr;

	(void)copy;

	kr = ipc_entry_get(space, &name, &entry);
	if (kr != KERN_SUCCESS)
		return kr;

	entry->ie_object = obj;
	entry->ie_bits |= MACH_PORT_TYPE_SEND;

	if (msgt_name != MACH_MSG_TYPE_COPY_SEND)
		entry->ie_bits |= MACH_PORT_TYPE_SEND_ONCE;

	is_write_unlock(space);
	*namep = name;
	return KERN_SUCCESS;
}

kern_return_t
ipc_object_translate(
	ipc_space_t		space,
	mach_port_t		name,
	mach_port_right_t	right,
	ipc_object_t		*objp)
{
	ipc_entry_t entry;

	(void)right;

	entry = ipc_entry_lookup(space, name);
	if (entry == IE_NULL)
		return KERN_INVALID_NAME;

	if (entry->ie_object == IO_NULL)
		return KERN_INVALID_RIGHT;

	*objp = entry->ie_object;
	return KERN_SUCCESS;
}

/* =======================================================================
 *  Notification stubs
 * ======================================================================= */

void
ipc_notify_init(void)
{
	/* Nothing to initialize */
}

void
ipc_notify_port_deleted(ipc_port_t port, mach_port_t name)
{
	(void)port;
	(void)name;
}

void
ipc_notify_msg_accepted(ipc_port_t port, mach_port_t name)
{
	(void)port;
	(void)name;
}

void
ipc_notify_port_destroyed(ipc_port_t port, ipc_port_t sright)
{
	(void)port;
	(void)sright;
}

void
ipc_notify_no_senders(ipc_port_t port, mach_port_mscount_t mscount)
{
	(void)port;
	(void)mscount;
}

void
ipc_notify_send_once(ipc_port_t port)
{
	(void)port;
}

void
ipc_notify_dead_name(ipc_port_t port, mach_port_t name)
{
	(void)port;
	(void)name;
}

/* =======================================================================
 *  Message queue stubs
 * ======================================================================= */

void
ipc_mqueue_init(ipc_mqueue_t mqueue)
{
	imq_lock_init(mqueue);
	ipc_kmsg_queue_init(&mqueue->imq_messages);
	ipc_thread_queue_init(&mqueue->imq_threads);
}

void
ipc_mqueue_move(ipc_mqueue_t from, ipc_mqueue_t to, ipc_port_t port)
{
	(void)from;
	(void)to;
	(void)port;
}

void
ipc_mqueue_changed(ipc_mqueue_t mqueue, mach_msg_return_t reason)
{
	ipc_thread_t thread;

	(void)reason;

	/* Wake up all waiting receivers with an error */
	while ((thread = ipc_thread_dequeue(&mqueue->imq_threads)) != ITH_NULL) {
		thread->ith_state = reason;
		thread_go(thread);
	}
}

/* =======================================================================
 *  IPC target stubs
 * ======================================================================= */

void
ipc_target_init(struct ipc_target *ipt, mach_port_t name)
{
	io_lock_init(&ipt->ipt_object);
	ipt->ipt_object.io_references = 1;
	ipt->ipt_object.io_bits = io_makebits(TRUE, IOT_PORT, 0);
	ipt->ipt_name = name;
	ipc_mqueue_init(&ipt->ipt_messages);
}

void
ipc_target_terminate(struct ipc_target *ipt)
{
	(void)ipt;
}

/* =======================================================================
 *  Port set stubs
 * ======================================================================= */

void
ipc_pset_remove(ipc_pset_t pset, ipc_port_t port)
{
	(void)pset;
	(void)port;
}

/* =======================================================================
 *  Kobject stub
 * ======================================================================= */

void
ipc_kobject_destroy(ipc_port_t port)
{
	(void)port;
}

/* =======================================================================
 *  Marequest stubs
 * ======================================================================= */

void
ipc_marequest_init(void)
{
	/* Nothing to initialize */
}

mach_msg_return_t
ipc_marequest_create(
	ipc_space_t		space,
	mach_port_t		name,
	ipc_port_t		soright,
	ipc_marequest_t		*marequestp)
{
	(void)space;
	(void)name;
	(void)soright;
	*marequestp = IMAR_NULL;
	return KERN_SUCCESS;
}

void
ipc_marequest_cancel(ipc_space_t space, mach_port_t name)
{
	(void)space;
	(void)name;
}

void
ipc_marequest_rename(
	ipc_space_t		space,
	mach_port_t		oname,
	mach_port_t		nname)
{
	(void)space;
	(void)oname;
	(void)nname;
}

void
ipc_marequest_destroy(ipc_marequest_t marequest)
{
	(void)marequest;
}
