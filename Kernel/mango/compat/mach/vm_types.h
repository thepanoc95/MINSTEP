/*
 * compat/mach/vm_types.h
 *
 * Usermode compatibility shim for Mach VM types.
 *
 * Provides vm_offset_t, vm_size_t, natural_t which are used
 * pervasively throughout the Mach4 IPC subsystem.
 */

#ifndef MANGO_COMPAT_MACH_VM_TYPES_H
#define MANGO_COMPAT_MACH_VM_TYPES_H

#include <stdint.h>
#include <stddef.h>

/* Natural-width unsigned integer (pointer-sized) */
typedef uintptr_t       natural_t;
typedef intptr_t        integer_t;

/* VM types -- these represent sizes and offsets in the address space */
typedef uintptr_t       vm_offset_t;
typedef uintptr_t       vm_size_t;
typedef vm_offset_t     vm_address_t;
typedef vm_offset_t     vm_object_offset_t;

/* Mach port index types used by ipc_table.h and ipc_entry.h */
typedef natural_t       mach_port_index_t;
typedef natural_t       mach_port_gen_t;
typedef natural_t       mach_port_mscount_t;
typedef natural_t       mach_port_rights_t;
typedef natural_t       mach_port_seqno_t;
typedef natural_t       mach_port_msgcount_t;
typedef natural_t       mach_port_type_t;
typedef natural_t       mach_port_urefs_t;

#endif /* MANGO_COMPAT_MACH_VM_TYPES_H */
