/*
 * compat/mach/boolean.h
 *
 * Usermode compatibility shim for Mach boolean types.
 */

#ifndef MANGO_COMPAT_MACH_BOOLEAN_H
#define MANGO_COMPAT_MACH_BOOLEAN_H

typedef int boolean_t;

#ifndef TRUE
#define TRUE  1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#define BOOL boolean_t

#endif /* MANGO_COMPAT_MACH_BOOLEAN_H */
