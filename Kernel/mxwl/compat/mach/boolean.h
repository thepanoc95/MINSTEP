/*
 * compat/mach/boolean.h
 *
 * Usermode compatibility shim for Mach boolean types.
 */

#ifndef MXWL_COMPAT_MACH_BOOLEAN_H
#define MXWL_COMPAT_MACH_BOOLEAN_H

typedef int boolean_t;

#ifndef TRUE
#define TRUE  1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#define BOOL boolean_t

#endif /* MXWL_COMPAT_MACH_BOOLEAN_H */
