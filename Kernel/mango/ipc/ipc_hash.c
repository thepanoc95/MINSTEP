/*
 * mango/ipc/ipc_hash.c
 *
 * Entry hash table operations.
 *
 * Ported from osfmk/kernel/ipc/ipc_hash.c (Rich Draves, 1989).
 * Adapted for the Mango usermode nanokernel.
 *
 * The hash table provides the reverse mapping:
 *     (ipc_space, ipc_object) -> (mach_port_name, ipc_entry)
 *
 * This is used for operations like ipc_object_copyout where we
 * have an object pointer and need to find the name a task knows
 * it by.
 *
 * Two levels of hash tables:
 *   - Local: open-addressing table embedded in the space's entry table.
 *            Used for entries with small names (in the flat table).
 *   - Global: chained hash table with per-bucket locks.
 *            Used for entries with large names (in the splay tree).
 */

#include <mach/boolean.h>
#include <mach/port.h>
#include <kern/lock.h>
#include <kern/kalloc.h>
#include <kern/assert.h>
#include <ipc/port.h>
#include <ipc/ipc_space.h>
#include <ipc/ipc_object.h>
#include <ipc/ipc_entry.h>
#include <ipc/ipc_hash.h>
#include <ipc/ipc_init.h>

#include <mach_ipc_debug.h>
#if MACH_IPC_DEBUG
#include <mach/kern_return.h>
#endif

/*
 * Routine:	ipc_hash_lookup
 * Purpose:
 *		Converts (space, obj) -> (name, entry).
 *		Returns TRUE if an entry was found.
 * Conditions:
 *		The space must be locked (read or write) throughout.
 */

boolean_t
ipc_hash_lookup(ipc_space_t space, ipc_object_t obj, mach_port_t *namep, ipc_entry_t *entryp)
{
	return (ipc_hash_local_lookup(space, obj, namep, entryp) ||
		((space->is_tree_hash > 0) &&
		 ipc_hash_global_lookup(space, obj, namep,
					(ipc_tree_entry_t *) entryp)));
}

/*
 * Routine:	ipc_hash_insert
 * Purpose:
 *		Inserts an entry into the appropriate reverse hash table,
 *		so that ipc_hash_lookup will find it.
 * Conditions:
 *		The space must be write-locked.
 */

void
ipc_hash_insert(
	ipc_space_t	space,
	ipc_object_t	obj,
	mach_port_t	name,
	ipc_entry_t	entry)
{
	mach_port_index_t index;

	index = MACH_PORT_INDEX(name);
	if ((index < space->is_table_size) &&
	    (entry == &space->is_table[index]))
		ipc_hash_local_insert(space, obj, index, entry);
	else
		ipc_hash_global_insert(space, obj, name,
				       (ipc_tree_entry_t) entry);
}

/*
 * Routine:	ipc_hash_delete
 * Purpose:
 *		Deletes an entry from the appropriate reverse hash table.
 * Conditions:
 *		The space must be write-locked.
 */

void
ipc_hash_delete(
	ipc_space_t	space,
	ipc_object_t	obj,
	mach_port_t	name,
	ipc_entry_t	entry)
{
	mach_port_index_t index;

	index = MACH_PORT_INDEX(name);
	if ((index < space->is_table_size) &&
	    (entry == &space->is_table[index]))
		ipc_hash_local_delete(space, obj, index, entry);
	else
		ipc_hash_global_delete(space, obj, name,
				       (ipc_tree_entry_t) entry);
}

/*
 * The global reverse hash table holds splay tree entries.
 * It is a simple open-chaining hash table with singly-linked buckets.
 * Each bucket is locked separately, with an exclusive lock.
 * Within each bucket, move-to-front is used.
 */

typedef natural_t ipc_hash_index_t;

ipc_hash_index_t ipc_hash_global_size;
ipc_hash_index_t ipc_hash_global_mask;

#define IH_GLOBAL_HASH(space, obj)					\
	(((((ipc_hash_index_t) ((vm_offset_t)space)) >> 4) +		\
	  (((ipc_hash_index_t) ((vm_offset_t)obj)) >> 6)) &		\
	 ipc_hash_global_mask)

typedef struct ipc_hash_global_bucket {
	decl_simple_lock_data(, ihgb_lock_data)
	ipc_tree_entry_t ihgb_head;
} *ipc_hash_global_bucket_t;

#define IHGB_NULL	((ipc_hash_global_bucket_t) 0)

#define	ihgb_lock_init(ihgb)	simple_lock_init(&(ihgb)->ihgb_lock_data)
#define	ihgb_lock(ihgb)		simple_lock(&(ihgb)->ihgb_lock_data)
#define	ihgb_unlock(ihgb)	simple_unlock(&(ihgb)->ihgb_lock_data)

ipc_hash_global_bucket_t ipc_hash_global_table;

/*
 * Routine:	ipc_hash_global_lookup
 * Purpose:
 *		Converts (space, obj) -> (name, entry).
 *		Looks in the global table, for splay tree entries.
 *		Returns TRUE if an entry was found.
 * Conditions:
 *		The space must be locked (read or write) throughout.
 */

boolean_t
ipc_hash_global_lookup(
	ipc_space_t		space,
	ipc_object_t		obj,
	mach_port_t		*namep,
	ipc_tree_entry_t	*entryp)
{
	ipc_hash_global_bucket_t bucket;
	ipc_tree_entry_t this, *last;

	assert(space != IS_NULL);
	assert(obj != IO_NULL);

	bucket = &ipc_hash_global_table[IH_GLOBAL_HASH(space, obj)];
	ihgb_lock(bucket);

	if ((this = bucket->ihgb_head) != ITE_NULL) {
		if ((this->ite_object == obj) &&
		    (this->ite_space == space)) {
			/* found it at front; no need to move */

			*namep = this->ite_name;
			*entryp = this;
		} else for (last = &this->ite_next;
			    (this = *last) != ITE_NULL;
			    last = &this->ite_next) {
			if ((this->ite_object == obj) &&
			    (this->ite_space == space)) {
				/* found it; move to front */

				*last = this->ite_next;
				this->ite_next = bucket->ihgb_head;
				bucket->ihgb_head = this;

				*namep = this->ite_name;
				*entryp = this;
				break;
			}
		}
	}

	ihgb_unlock(bucket);
	return this != ITE_NULL;
}

/*
 * Routine:	ipc_hash_global_insert
 * Purpose:
 *		Inserts an entry into the global reverse hash table.
 * Conditions:
 *		The space must be write-locked.
 */

void
ipc_hash_global_insert(
	ipc_space_t		space,
	ipc_object_t		obj,
	mach_port_t		name,
	ipc_tree_entry_t	entry)
{
	ipc_hash_global_bucket_t bucket;

	assert(entry->ite_name == name);
	assert(space != IS_NULL);
	assert(entry->ite_space == space);
	assert(obj != IO_NULL);
	assert(entry->ite_object == obj);

	space->is_tree_hash++;
	assert(space->is_tree_hash <= space->is_tree_total);

	bucket = &ipc_hash_global_table[IH_GLOBAL_HASH(space, obj)];
	ihgb_lock(bucket);

	/* insert at front of bucket */

	entry->ite_next = bucket->ihgb_head;
	bucket->ihgb_head = entry;

	ihgb_unlock(bucket);
}

/*
 * Routine:	ipc_hash_global_delete
 * Purpose:
 *		Deletes an entry from the global reverse hash table.
 * Conditions:
 *		The space must be write-locked.
 */

void
ipc_hash_global_delete(
	ipc_space_t		space,
	ipc_object_t		obj,
	mach_port_t		name,
	ipc_tree_entry_t	entry)
{
	ipc_hash_global_bucket_t bucket;
	ipc_tree_entry_t this, *last;

	assert(entry->ite_name == name);
	assert(space != IS_NULL);
	assert(entry->ite_space == space);
	assert(obj != IO_NULL);
	assert(entry->ite_object == obj);

	assert(space->is_tree_hash > 0);
	space->is_tree_hash--;

	bucket = &ipc_hash_global_table[IH_GLOBAL_HASH(space, obj)];
	ihgb_lock(bucket);

	for (last = &bucket->ihgb_head;
	     (this = *last) != ITE_NULL;
	     last = &this->ite_next) {
		if (this == entry) {
			/* found it; remove from bucket */

			*last = this->ite_next;
			break;
		}
	}
	assert(this != ITE_NULL);

	ihgb_unlock(bucket);
}

/*
 *	Each space has a local reverse hash table, which holds
 *	entries from the space's table.  In fact, the hash table
 *	just uses a field (ie_index) in the table itself.
 *
 *	The local hash table is an open-addressing hash table,
 *	which means that when a collision occurs, instead of
 *	throwing the entry into a bucket, the entry is rehashed
 *	to another position in the table.  In this case the rehash
 *	is very simple: linear probing (ie, just increment the position).
 */

#define	IH_LOCAL_HASH(obj, size)				\
		((((mach_port_index_t) (obj)) >> 6) % (size))

/*
 * Routine:	ipc_hash_local_lookup
 * Purpose:
 *		Converts (space, obj) -> (name, entry).
 *		Looks in the space's local table, for table entries.
 *		Returns TRUE if an entry was found.
 * Conditions:
 *		The space must be locked (read or write) throughout.
 */

boolean_t
ipc_hash_local_lookup(
	ipc_space_t	space,
	ipc_object_t	obj,
	mach_port_t	*namep,
	ipc_entry_t	*entryp)
{
	ipc_entry_t table;
	ipc_entry_num_t size;
	mach_port_index_t hindex, index;

	assert(space != IS_NULL);
	assert(obj != IO_NULL);

	table = space->is_table;
	size = space->is_table_size;
	hindex = IH_LOCAL_HASH(obj, size);

	while ((index = table[hindex].ie_index) != 0) {
		ipc_entry_t entry = &table[index];

		if (entry->ie_object == obj) {
			*namep = MACH_PORT_MAKEB(index, entry->ie_bits);
			*entryp = entry;
			return TRUE;
		}

		if (++hindex == size)
			hindex = 0;
	}

	return FALSE;
}

/*
 * Routine:	ipc_hash_local_insert
 * Purpose:
 *		Inserts an entry into the space's reverse hash table.
 * Conditions:
 *		The space must be write-locked.
 */

void
ipc_hash_local_insert(
	ipc_space_t		space,
	ipc_object_t		obj,
	mach_port_index_t	index,
	ipc_entry_t		entry)
{
	ipc_entry_t table;
	ipc_entry_num_t size;
	mach_port_index_t hindex;

	assert(index != 0);
	assert(space != IS_NULL);
	assert(obj != IO_NULL);

	table = space->is_table;
	size = space->is_table_size;
	hindex = IH_LOCAL_HASH(obj, size);

	assert(entry == &table[index]);
	assert(entry->ie_object == obj);

	while (table[hindex].ie_index != 0) {
		if (++hindex == size)
			hindex = 0;
	}

	table[hindex].ie_index = index;
}

/*
 * Routine:	ipc_hash_local_delete
 * Purpose:
 *		Deletes an entry from the space's reverse hash table.
 * Conditions:
 *		The space must be write-locked.
 */

void
ipc_hash_local_delete(
	ipc_space_t		space,
	ipc_object_t		obj,
	mach_port_index_t	index,
	ipc_entry_t		entry)
{
	ipc_entry_t table;
	ipc_entry_num_t size;
	mach_port_index_t hindex, dindex;

	assert(index != MACH_PORT_NULL);
	assert(space != IS_NULL);
	assert(obj != IO_NULL);

	table = space->is_table;
	size = space->is_table_size;
	hindex = IH_LOCAL_HASH(obj, size);

	assert(entry == &table[index]);
	assert(entry->ie_object == obj);

	while (table[hindex].ie_index != index) {
		if (table[hindex].ie_index == 0)
		{
			/* entry wasn't in hash table -- shouldn't happen */
			return;
		}
		if (++hindex == size)
			hindex = 0;
	}

	/*
	 * Now we want to set table[hindex].ie_index = 0.
	 * But if we aren't the last index in a clump,
	 * this might cause problems for lookups of objects
	 * farther along in the clump that are displaced
	 * due to collisions.  So we must check the clump
	 * after hindex for displaced objects and move them up.
	 */

	for (dindex = hindex; index != 0; hindex = dindex) {
		for (;;) {
			mach_port_index_t tindex;
			ipc_object_t tobj;

			if (++dindex == size)
				dindex = 0;
			assert(dindex != hindex);

			/* are we at the end of the clump? */

			index = table[dindex].ie_index;
			if (index == 0)
				break;

			/* is this a displaced object? */

			tobj = table[index].ie_object;
			assert(tobj != IO_NULL);
			tindex = IH_LOCAL_HASH(tobj, size);

			if ((dindex < hindex) ?
			    ((dindex < tindex) && (tindex <= hindex)) :
			    ((dindex < tindex) || (tindex <= hindex)))
				break;
		}

		table[hindex].ie_index = index;
	}
}

/*
 * Routine:	ipc_hash_init
 * Purpose:
 *		Initialize the reverse hash table implementation.
 */

void
ipc_hash_init(void)
{
	ipc_hash_index_t i;

	/* if not configured, initialize ipc_hash_global_size */

	if (ipc_hash_global_size == 0) {
		ipc_hash_global_size = ipc_tree_entry_max >> 8;
		if (ipc_hash_global_size < 32)
			ipc_hash_global_size = 32;
	}

	/* make sure it is a power of two */

	ipc_hash_global_mask = ipc_hash_global_size - 1;
	if ((ipc_hash_global_size & ipc_hash_global_mask) != 0) {
		natural_t bit;

		/* round up to closest power of two */

		for (bit = 1;; bit <<= 1) {
			ipc_hash_global_mask |= bit;
			ipc_hash_global_size = ipc_hash_global_mask + 1;

			if ((ipc_hash_global_size & ipc_hash_global_mask) == 0)
				break;
		}
	}

	/* allocate ipc_hash_global_table */

	ipc_hash_global_table = (ipc_hash_global_bucket_t)
		kalloc((vm_size_t) (ipc_hash_global_size *
				    sizeof(struct ipc_hash_global_bucket)));
	assert(ipc_hash_global_table != IHGB_NULL);

	/* and initialize it */

	for (i = 0; i < ipc_hash_global_size; i++) {
		ipc_hash_global_bucket_t bucket;

		bucket = &ipc_hash_global_table[i];
		ihgb_lock_init(bucket);
		bucket->ihgb_head = ITE_NULL;
	}
}

#if MACH_IPC_DEBUG
#include <mach_debug/hash_info.h>

/*
 * Routine:	ipc_hash_info
 * Purpose:
 *		Return information about the global reverse hash table.
 */

ipc_hash_index_t
ipc_hash_info(
	hash_info_bucket_t	*info,
	mach_msg_type_number_t count)
{
	ipc_hash_index_t i;

	if (ipc_hash_global_size < count)
		count = ipc_hash_global_size;

	for (i = 0; i < count; i++) {
		ipc_hash_global_bucket_t bucket = &ipc_hash_global_table[i];
		unsigned int bucket_count = 0;
		ipc_tree_entry_t entry;

		ihgb_lock(bucket);
		for (entry = bucket->ihgb_head;
		     entry != ITE_NULL;
		     entry = entry->ite_next)
			bucket_count++;
		ihgb_unlock(bucket);

		info[i].hib_count = bucket_count;
	}

	return ipc_hash_global_size;
}
#endif /* MACH_IPC_DEBUG */
