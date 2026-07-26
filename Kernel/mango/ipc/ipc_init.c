/*
 * mango/ipc/ipc_init.c
 *
 * IPC subsystem initialization for Mango.
 *
 * Defines the global variables needed by the ported Mach4 IPC
 * data structures (zones, hash tables, growth schedules, etc.)
 * and provides the ipc_init() entry point.
 *
 * This file replaces osfmk/kernel/ipc/ipc_init.c with a
 * usermode-compatible implementation.
 */

#include <stdlib.h>
#include <string.h>

#include <mach/boolean.h>
#include <kern/zalloc.h>
#include <kern/lock.h>
#include <ipc/ipc_object.h>
#include <ipc/ipc_entry.h>
#include <ipc/ipc_space.h>
#include <ipc/ipc_table.h>
#include <ipc/ipc_init.h>

/* ====================================================================
 *  Zone definitions
 * ==================================================================== */

/* Object zones: one per IPC object type (port, port set) */
zone_t ipc_object_zones[IOT_NUMBER];

/* Tree entry zone: for ipc_tree_entry_t allocations */
extern zone_t ipc_tree_entry_zone;

/* Space zone: for ipc_space_t allocations */
zone_t ipc_space_zone;

/* ====================================================================
 *  Table growth schedule
 *
 *  These define the progression of sizes as an IPC space's
 *  entry table grows.  Must end with a duplicate entry.
 * ==================================================================== */

static struct ipc_table_size _ipc_table_entries[] = {
    { 64 },     /* initial table size */
    { 128 },
    { 256 },
    { 512 },
    { 1024 },
    { 2048 },
    { 4096 },
    { 8192 },
    { 16384 },
    { 32768 },
    { 65536 },
    { 65536 },  /* duplicate = sentinel */
};

ipc_table_size_t ipc_table_entries = _ipc_table_entries;

/* Dead-name request table growth schedule */
static struct ipc_table_size _ipc_table_dnrequests[] = {
    { 4 },      /* initial dnrequest table size */
    { 8 },
    { 16 },
    { 32 },
    { 64 },
    { 0 },      /* sentinel: no more growth */
};

ipc_table_size_t ipc_table_dnrequests = _ipc_table_dnrequests;

/* ====================================================================
 *  Special spaces (kernel and reply)
 * ==================================================================== */

struct ipc_space *ipc_space_kernel = IS_NULL;
struct ipc_space *ipc_space_reply  = IS_NULL;

/* ====================================================================
 *  Tree entry max -- used by ipc_hash_init() to size the global table
 * ==================================================================== */

unsigned int ipc_tree_entry_max = 16384;

/* ====================================================================
 *  ipc_table_init
 *  Initialize the IPC table growth schedule (already done via statics).
 * ==================================================================== */

void
ipc_table_init(void)
{
    /* Growth schedules are statically initialized above.
     * This function exists for API compatibility. */
}

/* ====================================================================
 *  ipc_object_reference / ipc_object_release
 *  Basic reference counting for ipc_objects.
 * ==================================================================== */

void
ipc_object_reference(ipc_object_t object)
{
    io_reference(object);
}

void
ipc_object_release(ipc_object_t object)
{
    io_release(object);
    /* Note: in a real kernel, the object would be freed when
     * references hit zero.  For now, we leave that to the caller. */
}

/* ====================================================================
 *  ipc_space_reference / ipc_space_release
 * ==================================================================== */

void
ipc_space_reference(struct ipc_space *space)
{
    ipc_space_reference_macro(space);
}

void
ipc_space_release(struct ipc_space *space)
{
    ipc_space_release_macro(space);
}

/* ====================================================================
 *  ipc_init  -- main IPC subsystem initialization
 *
 *  Creates zones, initializes hash tables, and sets up the
 *  kernel and reply spaces.
 * ==================================================================== */

kern_return_t
ipc_init(void)
{
    /* Initialize table growth schedules */
    ipc_table_init();

    /* Create object zones.
     * Element sizes are generous to accommodate port structures
     * that embed ipc_object. */
    ipc_object_zones[IOT_PORT] = zinit(
        256,            /* element size (generous) */
        4096,           /* max elements */
        4096 * 256,     /* alloc size */
        "ipc port");

    ipc_object_zones[IOT_PORT_SET] = zinit(
        128,            /* element size */
        1024,           /* max elements */
        1024 * 128,     /* alloc size */
        "ipc port set");

    /* Create tree entry zone */
    ipc_tree_entry_zone = zinit(
        sizeof(struct ipc_tree_entry),
        ipc_tree_entry_max,
        ipc_tree_entry_max * sizeof(struct ipc_tree_entry),
        "ipc tree entry");

    /* Create space zone */
    ipc_space_zone = zinit(
        sizeof(struct ipc_space),
        1024,
        1024 * sizeof(struct ipc_space),
        "ipc space");

    /* Create the kernel and reply spaces */
    ipc_space_create_special(&ipc_space_kernel);
    ipc_space_create_special(&ipc_space_reply);

    return KERN_SUCCESS;
}

/* ====================================================================
 *  ipc_space_create_special
 *  Create a special (kernel or reply) space with no locking.
 * ==================================================================== */

kern_return_t
ipc_space_create_special(struct ipc_space **spacep)
{
    ipc_space_t space;

    space = is_alloc();
    if (space == IS_NULL)
        return KERN_FAILURE;

    memset(space, 0, sizeof(*space));

    is_ref_lock_init(space);
    is_lock_init(space);

    space->is_active   = TRUE;
    space->is_growing  = FALSE;
    space->is_references = 1;

    /* Allocate a minimal table (index 0 is always free) */
    space->is_table_next = ipc_table_entries;
    space->is_table_size = ipc_table_entries->its_size;
    space->is_table = (ipc_entry_t) it_entries_alloc(ipc_table_entries);

    if (space->is_table == IE_NULL) {
        is_free(space);
        return KERN_NO_SPACE;
    }

    memset(space->is_table, 0,
           space->is_table_size * sizeof(struct ipc_entry));

    /* Initialize the splay tree */
    ipc_splay_tree_init(&space->is_tree);
    space->is_tree_total = 0;
    space->is_tree_small = 0;
    space->is_tree_hash  = 0;

    /* Advance to the next growth step */
    if (ipc_table_entries->its_size != 0)
        space->is_table_next++;

    *spacep = space;
    return KERN_SUCCESS;
}

/* ====================================================================
 *  ipc_space_create / ipc_space_destroy
 *  Standard space lifecycle (simplified).
 * ==================================================================== */

kern_return_t
ipc_space_create(ipc_table_size_t table_entries, ipc_space_t *spacep)
{
    ipc_space_t space;

    space = is_alloc();
    if (space == IS_NULL)
        return KERN_FAILURE;

    memset(space, 0, sizeof(*space));

    is_ref_lock_init(space);
    is_lock_init(space);

    space->is_active    = TRUE;
    space->is_growing   = FALSE;
    space->is_references = 1;
    space->is_table_next = table_entries;
    space->is_table_size = table_entries->its_size;
    space->is_table = (ipc_entry_t) it_entries_alloc(table_entries);

    if (space->is_table == IE_NULL) {
        is_free(space);
        return KERN_NO_SPACE;
    }

    memset(space->is_table, 0,
           space->is_table_size * sizeof(struct ipc_entry));

    ipc_splay_tree_init(&space->is_tree);
    space->is_tree_total = 0;
    space->is_tree_small = 0;
    space->is_tree_hash  = 0;

    if (table_entries->its_size != 0)
        space->is_table_next++;

    *spacep = space;
    return KERN_SUCCESS;
}

void
ipc_space_destroy(struct ipc_space *space)
{
    if (space == IS_NULL)
        return;

    /* Free the entry table */
    if (space->is_table != IE_NULL)
        it_entries_free(space->is_table_next, (vm_offset_t)space->is_table);

    is_free(space);
}
