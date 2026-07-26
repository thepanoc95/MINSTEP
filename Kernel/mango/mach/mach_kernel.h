/*
 * mango/mach/mach_kernel.h
 *
 * Kernel boot and lifecycle management.
 * This header defines the entry points for the Mango nanokernel.
 */

#ifndef MANGO_MACH_MACH_KERNEL_H
#define MANGO_MACH_MACH_KERNEL_H

#include "mach_types.h"
#include <sys/types.h>
#include <objc/objc.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -----------------------------------------------------------------------
 *  Kernel version
 * ----------------------------------------------------------------------- */

#define MANGO_KERNEL_VERSION    "0.1.0"
#define MANGO_KERNEL_NAME       "Mango"

/* -----------------------------------------------------------------------
 *  Kernel boot flags
 * ----------------------------------------------------------------------- */

#define MANGO_BOOT_VERBOSE      0x0001
#define MANGO_BOOT_SINGLE_USER  0x0002
#define MANGO_BOOT_NO_INIT      0x0004
#define MANGO_BOOT_DEBUG        0x0008

/* -----------------------------------------------------------------------
 *  Kernel state
 * ----------------------------------------------------------------------- */

typedef struct mango_kernel_state {
    BOOL                initialized;
    BOOL                running;
    uint32_t            boot_flags;
    char                userfs_root[512];   /* $USERFSROOT                  */
    pid_t               kernel_pid;         /* Our own PID                  */
    pid_t               init_pid;           /* PID of the init process      */
    mach_port_t         host_port;          /* Host port                    */
    mach_port_t         host_priv_port;     /* Host privileged port         */
    mach_port_t         kernel_port;        /* Kernel port                  */
} mango_kernel_state_t;

extern mango_kernel_state_t _mango_kernel;

/* -----------------------------------------------------------------------
 *  Kernel entry points
 * ----------------------------------------------------------------------- */

/* Main kernel entry (called by boot1's jump2mach).
 * init_path may be NULL to use default init search. */
void mango_kernel_main(const char *init_path);

/* Kernel initialization (sets up IPC, tasks, loader) */
kern_return_t mango_kernel_init(uint32_t boot_flags);

/* Kernel main loop (dispatches IPC, manages tasks) */
void mango_kernel_loop(void);

/* Kernel shutdown */
void mango_kernel_shutdown(void);

/* Print kernel banner and version */
void mango_kernel_banner(void);

/* -----------------------------------------------------------------------
 *  Userfs helpers
 * ----------------------------------------------------------------------- */

/* Get the userfs root path ($USERFSROOT or default) */
const char *mango_get_userfs_root(void);

/* Set the userfs root path */
void mango_set_userfs_root(const char *path);

#ifdef __cplusplus
}
#endif

#endif /* MANGO_MACH_MACH_KERNEL_H */
