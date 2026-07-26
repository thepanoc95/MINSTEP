/*
 * compat/ipc/ipc_table.h
 *
 * Usermode compatibility shim for Mach IPC table definitions.
 *
 * Provides the ipc_table_size structures and the table alloc/realloc/free
 * primitives.  In Mango, we back these with malloc rather than the
 * kernel VM system.
 *
 * Original: osfmk/kernel/ipc/ipc_table.h
 */

#ifndef MANGO_COMPAT_IPC_IPC_TABLE_H
#define MANGO_COMPAT_IPC_IPC_TABLE_H

#include <mach/boolean.h>
#include <mach/vm_param.h>
#include <mach/vm_types.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -----------------------------------------------------------------------
 *  Table index and size types
 * ----------------------------------------------------------------------- */

typedef unsigned int ipc_table_index_t;    /* index into tables */
typedef unsigned int ipc_table_elems_t;    /* size of tables */

typedef struct ipc_table_size {
    ipc_table_elems_t its_size;        /* number of elements in table */
} *ipc_table_size_t;

#define ITS_NULL    ((ipc_table_size_t) 0)

/* -----------------------------------------------------------------------
 *  Growth schedule -- these arrays are defined in ipc_init.c
 * ----------------------------------------------------------------------- */

extern ipc_table_size_t ipc_table_entries;
extern ipc_table_size_t ipc_table_dnrequests;

extern void ipc_table_init(void);

/* -----------------------------------------------------------------------
 *  Table allocation -- backed by malloc in usermode
 *
 *  Note: In the real kernel, ipc_table_realloc uses vm remap
 *  (not memcpy).  We use realloc here because we are in usermode.
 * ----------------------------------------------------------------------- */

static inline vm_offset_t
ipc_table_alloc(vm_size_t size)
{
    void *p = calloc(1, size);
    return (vm_offset_t) p;
}

static inline vm_offset_t
ipc_table_realloc(vm_size_t old_size, vm_offset_t old_table,
                  vm_size_t new_size)
{
    (void)old_size;
    void *p = realloc((void *)old_table, new_size);
    if (p && new_size > old_size) {
        /* Zero the newly allocated portion */
        memset((char *)p + old_size, 0, new_size - old_size);
    }
    return (vm_offset_t) p;
}

static inline void
ipc_table_free(vm_size_t size, vm_offset_t table)
{
    (void)size;
    free((void *)table);
}

/* -----------------------------------------------------------------------
 *  Entry table convenience macros
 * ----------------------------------------------------------------------- */

/* Forward declaration -- ipc_entry_t is defined in ipc_entry.h */
struct ipc_entry;

#define it_entries_alloc(its)                                           \
    ((struct ipc_entry *)                                               \
     ipc_table_alloc((its)->its_size * sizeof(struct ipc_entry)))

#define it_entries_reallocable(its)                                     \
    (((its)->its_size * sizeof(struct ipc_entry)) >= PAGE_SIZE)

#define it_entries_realloc(its, table, nits)                            \
    ((struct ipc_entry *)                                               \
     ipc_table_realloc((its)->its_size * sizeof(struct ipc_entry),      \
                       (vm_offset_t)(table),                            \
                       (nits)->its_size * sizeof(struct ipc_entry)))

#define it_entries_free(its, table)                                     \
    ipc_table_free((its)->its_size * sizeof(struct ipc_entry),          \
                   (vm_offset_t)(table))

/* -----------------------------------------------------------------------
 *  Dead-name request table convenience macros
 * ----------------------------------------------------------------------- */

struct ipc_port_request;

#define it_dnrequests_alloc(its)                                        \
    ((struct ipc_port_request *)                                        \
     ipc_table_alloc((its)->its_size *                                  \
                     sizeof(struct ipc_port_request)))

#define it_dnrequests_free(its, table)                                  \
    ipc_table_free((its)->its_size *                                    \
                   sizeof(struct ipc_port_request),                     \
                   (vm_offset_t)(table))

#ifdef __cplusplus
}
#endif

#endif /* MANGO_COMPAT_IPC_IPC_TABLE_H */
