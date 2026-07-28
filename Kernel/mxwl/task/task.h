/*
 * mxwl/task/task.h
 *
 * Task and thread abstraction for the Maxxwell nanokernel.
 *
 * In real Mach, a task is an address space with port rights and
 * an list of threads.  In Maxxwell's usermode environment, a task
 * maps to a host process (fork/exec).  The kernel tracks tasks
 * and their port rights, and provides the illusion of Mach-style
 * task management on top of POSIX processes.
 */

#ifndef MXWL_TASK_TASK_H
#define MXWL_TASK_TASK_H

#include "../mach/mach_types.h"
#include "../mach/mach_port.h"

#include <sys/types.h>
#include <unistd.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -----------------------------------------------------------------------
 *  Limits
 * ----------------------------------------------------------------------- */

#define TASK_MAX            128
#define TASK_PORT_MAX       64
#define TASK_NAME_MAX       64

/* -----------------------------------------------------------------------
 *  Thread object
 * ----------------------------------------------------------------------- */

typedef struct mxwl_thread {
    mach_thread_t       id;                 /* Thread ID                    */
    pid_t               host_tid;           /* Host thread ID (if mapped)   */
    BOOL                active;             /* Thread is running            */
    BOOL                suspended;          /* Thread is suspended           */
} mxwl_thread_t;

#define THREAD_MAX  256

/* -----------------------------------------------------------------------
 *  Task object
 * ----------------------------------------------------------------------- */

typedef struct mxwl_task {
    mach_task_t         id;                 /* Task ID (index in table)     */
    char                name[TASK_NAME_MAX];/* Human-readable name          */
    pid_t               host_pid;           /* Host process ID              */
    BOOL                in_use;             /* Slot is allocated            */
    BOOL                running;            /* Task has been started        */
    BOOL                terminated;         /* Task has exited              */

    /* Port rights table: maps Mach port names to port objects.
     * ports[i] is the name, port_objects[i] is the port object. */
    mach_port_name_t    ports[TASK_PORT_MAX];
    mach_port_t         port_objects[TASK_PORT_MAX];
    int                 port_count;

    /* Bootstrap port -- every task gets one */
    mach_port_t         bootstrap_port;

    /* Threads belonging to this task */
    mxwl_thread_t     *threads[THREAD_MAX];
    int                 thread_count;

    /* Binary path (for .mach loader) */
    char                binary_path[512];

    /* User filesystem root */
    char                userfs_root[512];
} mxwl_task_t;

/* -----------------------------------------------------------------------
 *  Task table
 * ----------------------------------------------------------------------- */

extern mxwl_task_t mxwl_task_table[TASK_MAX];
extern int          mxwl_task_count;
extern mxwl_task_t *mxwl_current_task;

/* -----------------------------------------------------------------------
 *  Task API
 * ----------------------------------------------------------------------- */

/* Create a new task */
kern_return_t mxwl_task_create(mxwl_task_t **out_task, const char *name);

/* Terminate a task */
kern_return_t mxwl_task_terminate(mxwl_task_t *task);

/* Look up a task by ID */
mxwl_task_t *mxwl_task_lookup(mach_task_t task_id);

/* Insert a port right into a task's port space */
kern_return_t mxwl_task_insert_port(mxwl_task_t *task,
                                     mach_port_name_t name,
                                     mach_port_t port);

/* Remove a port right from a task */
kern_return_t mxwl_task_remove_port(mxwl_task_t *task,
                                     mach_port_name_t name);

/* Get a task's bootstrap port */
mach_port_t mxwl_task_get_bootstrap_port(mxwl_task_t *task);

/* -----------------------------------------------------------------------
 *  Thread API
 * ----------------------------------------------------------------------- */

/* Create a thread in a task */
kern_return_t mxwl_thread_create(mxwl_task_t *task,
                                  mxwl_thread_t **out_thread);

/* Terminate a thread */
kern_return_t mxwl_thread_terminate(mxwl_thread_t *thread);

/* Suspend a thread */
kern_return_t mxwl_thread_suspend(mxwl_thread_t *thread);

/* Resume a thread */
kern_return_t mxwl_thread_resume(mxwl_thread_t *thread);

/* -----------------------------------------------------------------------
 *  Kernel bootstrap sequence
 * ----------------------------------------------------------------------- */

/* Initialize the task subsystem */
kern_return_t mxwl_task_init(void);

/* Create the kernel task (task 0 -- the kernel itself) */
kern_return_t mxwl_kernel_task_init(void);

/* -----------------------------------------------------------------------
 *  Init system
 * ----------------------------------------------------------------------- */

/* Launch the init process.
 * If init_path is non-NULL, use it directly.
 * Otherwise search $USERFSROOT/private/init, then /sbin/init. */
kern_return_t mxwl_launch_init(const char *userfs_root, const char *init_path);

#ifdef __cplusplus
}
#endif

#endif /* MXWL_TASK_TASK_H */
