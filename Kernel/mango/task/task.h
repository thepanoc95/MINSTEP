/*
 * mango/task/task.h
 *
 * Task and thread abstraction for the Mango nanokernel.
 *
 * In real Mach, a task is an address space with port rights and
 * an list of threads.  In Mango's usermode environment, a task
 * maps to a host process (fork/exec).  The kernel tracks tasks
 * and their port rights, and provides the illusion of Mach-style
 * task management on top of POSIX processes.
 */

#ifndef MANGO_TASK_TASK_H
#define MANGO_TASK_TASK_H

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

typedef struct mango_thread {
    mach_thread_t       id;                 /* Thread ID                    */
    pid_t               host_tid;           /* Host thread ID (if mapped)   */
    BOOL                active;             /* Thread is running            */
    BOOL                suspended;          /* Thread is suspended           */
} mango_thread_t;

#define THREAD_MAX  256

/* -----------------------------------------------------------------------
 *  Task object
 * ----------------------------------------------------------------------- */

typedef struct mango_task {
    mach_task_t         id;                 /* Task ID (index in table)     */
    char                name[TASK_NAME_MAX];/* Human-readable name          */
    pid_t               host_pid;           /* Host process ID              */
    BOOL                in_use;             /* Slot is allocated            */
    BOOL                running;            /* Task has been started        */
    BOOL                terminated;         /* Task has exited              }

    /* Port rights table, maps Mach port names to port objects.
     * ports[i] is the name, port_objects[i] is the port object. */
    mach_port_name_t    ports[TASK_PORT_MAX];
    mach_port_t         port_objects[TASK_PORT_MAX];
    int                 port_count;

    /* Bootstrap port -- every task gets one */
    mach_port_t         bootstrap_port;

    /* Threads belonging to this task */
    mango_thread_t     *threads[THREAD_MAX];
    int                 thread_count;

    /* Binary path (for .mach loader) */
    char                binary_path[512];

    /* User filesystem root */
    char                userfs_root[512];
} mango_task_t;

/* -----------------------------------------------------------------------
 *  Task table
 * ----------------------------------------------------------------------- */

extern mango_task_t _mango_task_table[TASK_MAX];
extern int          _mango_task_count;
extern mango_task_t *_mango_current_task;

/* -----------------------------------------------------------------------
 *  Task API
 * ----------------------------------------------------------------------- */

/* Create a new task */
kern_return_t mango_task_create(mango_task_t **out_task, const char *name);

/* Terminate a task */
kern_return_t mango_task_terminate(mango_task_t *task);

/* Look up a task by ID */
mango_task_t *mango_task_lookup(mach_task_t task_id);

/* Insert a port right into a task's port space */
kern_return_t mango_task_insert_port(mango_task_t *task,
                                     mach_port_name_t name,
                                     mach_port_t port);

/* Remove a port right from a task */
kern_return_t mango_task_remove_port(mango_task_t *task,
                                     mach_port_name_t name);

/* Get a task's bootstrap port */
mach_port_t mango_task_get_bootstrap_port(mango_task_t *task);

/* -----------------------------------------------------------------------
 *  Thread API
 * ----------------------------------------------------------------------- */

/* Create a thread in a task */
kern_return_t mango_thread_create(mango_task_t *task,
                                  mango_thread_t **out_thread);

/* Terminate a thread */
kern_return_t mango_thread_terminate(mango_thread_t *thread);

/* Suspend a thread */
kern_return_t mango_thread_suspend(mango_thread_t *thread);

/* Resume a thread */
kern_return_t mango_thread_resume(mango_thread_t *thread);

/* -----------------------------------------------------------------------
 *  Kernel bootstrap sequence
 * ----------------------------------------------------------------------- */

/* Initialize the task subsystem */
kern_return_t mango_task_init(void);

/* Create the kernel task (task 0 -- the kernel itself) */
kern_return_t mango_kernel_task_init(void);

/* -----------------------------------------------------------------------
 *  Init system
 * ----------------------------------------------------------------------- */

/* Launch the init process from $USERFSROOT/private/init */
kern_return_t mango_launch_init(const char *userfs_root);

#ifdef __cplusplus
}
#endif

#endif /* MANGO_TASK_TASK_H */
