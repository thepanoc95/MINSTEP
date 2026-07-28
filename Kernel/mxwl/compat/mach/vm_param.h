/*
 * compat/mach/vm_param.h
 *
 * Usermode compatibility shim for Mach VM parameter constants.
 * Provides PAGE_SIZE and related constants used by ipc_table.h.
 */

#ifndef MXWL_COMPAT_MACH_VM_PARAM_H
#define MXWL_COMPAT_MACH_VM_PARAM_H

#include <mach/vm_types.h>

#ifndef PAGE_SIZE
#define PAGE_SIZE       4096
#endif

#define PAGE_SHIFT      12
#define PAGE_MASK       (PAGE_SIZE - 1)

#define VM_PAGE_SIZE    PAGE_SIZE

#endif /* MXWL_COMPAT_MACH_VM_PARAM_H */
