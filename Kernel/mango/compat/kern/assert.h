/*
 * compat/kern/assert.h
 *
 * Usermode compatibility shim for Mach kernel assert.
 *
 * Maps to standard C assert() unless NDEBUG is defined.
 */

#ifndef MANGO_COMPAT_KERN_ASSERT_H
#define MANGO_COMPAT_KERN_ASSERT_H

#include <assert.h>

/* The osfmk/ipc/ code uses assert() from <kern/assert.h>.
 * We simply redirect to the standard C assert. */

#ifndef MACH_ASSERT
#define MACH_ASSERT 1
#endif

#ifndef MACH_ASSERT_TRACING
#define MACH_ASSERT_TRACING 0
#endif

#endif /* MANGO_COMPAT_KERN_ASSERT_H */
