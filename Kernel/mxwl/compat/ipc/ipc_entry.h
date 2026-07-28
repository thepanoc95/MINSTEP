/*
 * compat/ipc/ipc_entry.h
 *
 * Usermode compatibility shim for Mach IPC entry tables.
 *
 * Provides ipc_entry (table entry) and ipc_tree_entry (splay tree
 * entry) structures.  These represent task capabilities for ports
 * and port sets.
 *
 * PORT_GENERATIONS is disabled (IE_BITS_GEN_MASK == 0).
 * Port names are just raw table indices.
 *
 * Original: osfmk/kernel/ipc/ipc_entry.h
 */

#ifndef MXWL_COMPAT_IPC_IPC_ENTRY_H
#define MXWL_COMPAT_IPC_IPC_ENTRY_H

#include <mach/port.h>
#include <mach/kern_return.h>
#include <kern/zalloc.h>
#include <ipc/ipc_types.h>
#include <ipc/ipc_table.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -----------------------------------------------------------------------
 *  Forward declarations
 * ----------------------------------------------------------------------- */

struct ipc_space;

/* -----------------------------------------------------------------------
 *  ipc_entry_bits_t -- capability bit fields
 * ----------------------------------------------------------------------- */

typedef unsigned int ipc_entry_bits_t;
typedef ipc_table_elems_t ipc_entry_num_t;   /* number of entries */

/* -----------------------------------------------------------------------
 *  ipc_entry -- table entry (small names)
 *
 *  The first entry (index 0) is always free; it holds the free list head.
 * ----------------------------------------------------------------------- */

typedef struct ipc_entry {
    ipc_entry_bits_t ie_bits;
    struct ipc_object *ie_object;
    union {
        mach_port_index_t next;     /* free list: next free index */
        unsigned int request;       /* dead-name request index */
    } index;
    union {
        mach_port_index_t table;    /* reverse hash: table index */
        struct ipc_tree_entry *tree; /* reverse hash: tree entry */
    } hash;
} *ipc_entry_t;

#define IE_NULL     ((ipc_entry_t) 0)

#define ie_request   index.request
#define ie_next      index.next
#define ie_index     hash.table

/* -----------------------------------------------------------------------
 *  ie_bits field layout
 * ----------------------------------------------------------------------- */

#define IE_BITS_UREFS_MASK   0x0000ffff     /* 16 bits of user-reference */
#define IE_BITS_UREFS(bits)  ((bits) & IE_BITS_UREFS_MASK)

#define IE_BITS_TYPE_MASK    0x001f0000     /* 5 bits of capability type */
#define IE_BITS_TYPE(bits)   ((bits) & IE_BITS_TYPE_MASK)

#define IE_BITS_MAREQUEST    0x00200000     /* 1 bit for msg-accepted */

#define IE_BITS_COMPAT       0x00400000     /* 1 bit for compatibility */

#define IE_BITS_COLLISION    0x00800000     /* 1 bit for collisions */
#define IE_BITS_RIGHT_MASK   0x007fffff     /* relevant to the right */

/* Generation bits -- disabled in Maxxwell (no port generations) */
#define IE_BITS_GEN_MASK     0
#define IE_BITS_GEN(bits)    0
#define IE_BITS_GEN_ONE      0

/* MACH_PORT_MAKEB: constructs a port name from an index and entry bits.
 * Without PORT_GENERATIONS, this simplifies to just the index. */
#ifndef MACH_PORT_MAKEB
#define MACH_PORT_MAKEB(index, bits) \
    ((mach_port_t)(index))
#endif

/* -----------------------------------------------------------------------
 *  ipc_tree_entry -- splay tree entry (large names)
 *
 *  This extends ipc_entry with the splay tree link fields and a
 *  back-pointer to the owning space.
 * ----------------------------------------------------------------------- */

typedef struct ipc_tree_entry {
    struct ipc_entry ite_entry;     /* base entry */
    mach_port_t ite_name;           /* port name for this entry */
    struct ipc_space *ite_space;    /* owning space */
    struct ipc_tree_entry *ite_lchild;  /* left child in splay tree */
    struct ipc_tree_entry *ite_rchild;  /* right child in splay tree */
} *ipc_tree_entry_t;

#define ITE_NULL    ((ipc_tree_entry_t) 0)

/* Convenience macros to access embedded ipc_entry fields */
#define ite_bits    ite_entry.ie_bits
#define ite_object  ite_entry.ie_object
#define ite_request ite_entry.ie_request
#define ite_next    ite_entry.hash.tree

/* -----------------------------------------------------------------------
 *  Zone-backed allocation for tree entries
 * ----------------------------------------------------------------------- */

extern zone_t ipc_tree_entry_zone;

#define ite_alloc()     ((ipc_tree_entry_t) zalloc(ipc_tree_entry_zone))
#define ite_free(ite)   zfree(ipc_tree_entry_zone, (vm_offset_t)(ite))

/* -----------------------------------------------------------------------
 *  Function declarations
 * ----------------------------------------------------------------------- */

extern ipc_entry_t
ipc_entry_lookup(ipc_space_t space, mach_port_t name);

extern kern_return_t
ipc_entry_get(ipc_space_t space, mach_port_t *namep, ipc_entry_t *entryp);

extern kern_return_t
ipc_entry_alloc(ipc_space_t space, mach_port_t *namep, ipc_entry_t *entryp);

extern kern_return_t
ipc_entry_alloc_name(ipc_space_t space, mach_port_t name,
                     ipc_entry_t *entryp);

extern void
ipc_entry_dealloc(ipc_space_t space, mach_port_t name, ipc_entry_t entry);

extern kern_return_t
ipc_entry_grow_table(ipc_space_t space);

#ifdef __cplusplus
}
#endif

#endif /* MXWL_COMPAT_IPC_IPC_ENTRY_H */
