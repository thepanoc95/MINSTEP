/*
 * compat/mach/kern_return.h
 *
 * Usermode compatibility shim for Mach kern_return_t.
 *
 * Provides the type and all KERN_* return codes used by the IPC subsystem.
 */

#ifndef MANGO_COMPAT_MACH_KERN_RETURN_H
#define MANGO_COMPAT_MACH_KERN_RETURN_H

typedef int kern_return_t;

#define KERN_SUCCESS                0
#define KERN_INVALID_ARGUMENT       4
#define KERN_FAILURE                5
#define KERN_INVALID_HOST           7
#define KERN_INVALID_RIGHT          8
#define KERN_INVALID_OBJECT         9
#define KERN_INVALID_NAME           10
#define KERN_NOT_RECEIVER           14
#define KERN_NO_SPACE               29
#define KERN_INVALID_CAPABILITY     30
#define KERN_TERMINATED             46
#define KERN_MSG_TOO_LARGE          52
#define KERN_INVALID_TASK           73
#define KERN_NOT_IN_SET             77
#define KERN_NAME_EXISTS            78
#define KERN_RESOURCE_SHORTAGE      85
#define KERN_ABORTED                90
#define KERN_INVALID_MEMORY_CONTROL 94
#define KERN_SET_NOT_SET            79
#define KERN_NO_RIGHTS              80
#define KERN_UREFS_OVERFLOW         100
#define KERN_RIGHT_EXISTS           110

#endif /* MANGO_COMPAT_MACH_KERN_RETURN_H */
