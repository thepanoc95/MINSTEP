/*
 * compat/kern/macro_help.h
 *
 * Usermode compatibility shim for Mach kernel macro helpers.
 *
 * MACRO_BEGIN / MACRO_END are used throughout osfmk/ipc/ to wrap
 * multi-statement macros so they behave correctly in if/else chains.
 */

#ifndef MXWL_COMPAT_KERN_MACRO_HELP_H
#define MXWL_COMPAT_KERN_MACRO_HELP_H

#define MACRO_BEGIN  do {
#define MACRO_END    } while (0)

#endif /* MXWL_COMPAT_KERN_MACRO_HELP_H */
