/*
 * mango/task/task.c
 *
 * Task and thread management for the Mango nanokernel.
 */

#include "task.h"
#include "../ipc/ipc.h"
#include "../mach/mach_port.h"
#include "../mach/klog.h"
#include "../kal/kal.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern mango_task_t  mango_task_table[TASK_MAX];
extern int           mango_task_count;
extern mango_task_t *mango_current_task;

kern_return_t mango_task_init(void)
{
    memset(mango_task_table, 0, sizeof(mango_task_table));
    mango_task_count = 0;
    mango_current_task = NULL;
    return mango_kernel_task_init();
}

kern_return_t mango_kernel_task_init(void)
{
    mango_task_t *task = &mango_task_table[0];

    memset(task, 0, sizeof(mango_task_t));
    task->id        = 0;
    task->host_pid  = kal_getpid();
    task->in_use    = TRUE;
    task->running   = TRUE;
    task->terminated = FALSE;
    strncpy(task->name, "kernel", TASK_NAME_MAX - 1);

    task->bootstrap_port = ipc_bootstrap_port;

    mango_task_count = 1;
    mango_current_task = task;

    klog_info("Task table initialized, kernel task (pid %d, bootstrap port %d).\n",
              task->host_pid, task->bootstrap_port);
    return KERN_SUCCESS;
}

kern_return_t mango_task_create(mango_task_t **out_task, const char *name)
{
    if (mango_task_count >= TASK_MAX) {
        klog_err("task table full\n");
        return KERN_NO_SPACE;
    }

    mango_task_t *task = NULL;
    for (int i = 0; i < TASK_MAX; i++) {
        if (!mango_task_table[i].in_use) {
            task = &mango_task_table[i];
            break;
        }
    }

    if (!task) return KERN_NO_SPACE;

    memset(task, 0, sizeof(mango_task_t));
    task->id = (int)(task - mango_task_table);
    task->host_pid = -1;
    task->in_use = TRUE;
    task->running = FALSE;
    task->terminated = FALSE;

    if (name) {
        strncpy(task->name, name, TASK_NAME_MAX - 1);
        task->name[TASK_NAME_MAX - 1] = '\0';
    } else {
        snprintf(task->name, TASK_NAME_MAX, "task%d", task->id);
    }

    task->bootstrap_port = mach_port_allocate(MACH_PORT_RIGHT_RECEIVE);
    if (task->bootstrap_port == MACH_PORT_NULL) {
        task->in_use = FALSE;
        return KERN_FAILURE;
    }

    mango_task_count++;

    klog_sub_info("task", "created: %s (id %d, bootstrap port %d)\n",
                   task->name, task->id, task->bootstrap_port);

    if (out_task) *out_task = task;
    return KERN_SUCCESS;
}

kern_return_t mango_task_terminate(mango_task_t *task)
{
    if (!task || !task->in_use) return KERN_INVALID_TASK;

    if (task->host_pid > 0) {
        kal_process_kill(task->host_pid, KAL_SIGTERM);
        kal_process_wait(task->host_pid, NULL);
    }

    for (int i = 0; i < task->port_count; i++) {
        mach_port_deallocate(task->port_objects[i]);
    }

    if (task->bootstrap_port != MACH_PORT_NULL) {
        mach_port_destroy(task->bootstrap_port);
    }

    task->running = FALSE;
    task->terminated = TRUE;
    task->in_use = FALSE;
    mango_task_count--;

    klog_sub_info("task", "terminated: %s (id %d)\n", task->name, task->id);
    return KERN_SUCCESS;
}

mango_task_t *mango_task_lookup(mach_task_t task_id)
{
    if (task_id < 0 || task_id >= TASK_MAX) return NULL;
    mango_task_t *task = &mango_task_table[task_id];
    if (!task->in_use) return NULL;
    return task;
}

kern_return_t mango_task_insert_port(mango_task_t *task,
                                     mach_port_name_t name,
                                     mach_port_t port)
{
    if (!task || task->port_count >= TASK_PORT_MAX) {
        return KERN_NO_SPACE;
    }
    task->ports[task->port_count] = name;
    task->port_objects[task->port_count] = port;
    task->port_count++;
    return KERN_SUCCESS;
}

kern_return_t mango_task_remove_port(mango_task_t *task,
                                     mach_port_name_t name)
{
    if (!task) return KERN_INVALID_TASK;
    for (int i = 0; i < task->port_count; i++) {
        if (task->ports[i] == name) {
            for (int j = i; j < task->port_count - 1; j++) {
                task->ports[j] = task->ports[j + 1];
                task->port_objects[j] = task->port_objects[j + 1];
            }
            task->port_count--;
            return KERN_SUCCESS;
        }
    }
    return KERN_INVALID_NAME;
}

mach_port_t mango_task_get_bootstrap_port(mango_task_t *task)
{
    if (!task) return MACH_PORT_NULL;
    return task->bootstrap_port;
}

kern_return_t mango_thread_create(mango_task_t *task,
                                  mango_thread_t **out_thread)
{
    if (!task || task->thread_count >= THREAD_MAX) {
        return KERN_NO_SPACE;
    }
    mango_thread_t *thread = kal_calloc(1, sizeof(mango_thread_t));
    if (!thread) return KERN_FAILURE;
    thread->id = task->thread_count;
    thread->host_tid = -1;
    thread->active = TRUE;
    thread->suspended = FALSE;
    task->threads[task->thread_count] = thread;
    task->thread_count++;
    if (out_thread) *out_thread = thread;
    return KERN_SUCCESS;
}

kern_return_t mango_thread_terminate(mango_thread_t *thread)
{
    if (!thread) return KERN_INVALID_ARGUMENT;
    thread->active = FALSE;
    kal_free(thread);
    return KERN_SUCCESS;
}

kern_return_t mango_thread_suspend(mango_thread_t *thread)
{
    if (!thread) return KERN_INVALID_ARGUMENT;
    thread->suspended = TRUE;
    return KERN_SUCCESS;
}

kern_return_t mango_thread_resume(mango_thread_t *thread)
{
    if (!thread) return KERN_INVALID_ARGUMENT;
    thread->suspended = FALSE;
    return KERN_SUCCESS;
}

kern_return_t mango_launch_init(const char *userfs_root, const char *init_path)
{
    if (!userfs_root) return KERN_INVALID_ARGUMENT;

    char resolved_path[1024];

    if (init_path && init_path[0]) {
        strncpy(resolved_path, init_path, sizeof(resolved_path) - 1);
        resolved_path[sizeof(resolved_path) - 1] = '\0';
        klog_info("using init: %s\n", resolved_path);
    } else {
        snprintf(resolved_path, sizeof(resolved_path), "%s/private/init", userfs_root);
        klog_info("searching for init: %s\n", resolved_path);

        if (!kal_can_exec(resolved_path)) {
            klog_warn("%s not found or not executable\n", resolved_path);
            klog_info("falling back to /sbin/init\n");
            strncpy(resolved_path, "/sbin/init", sizeof(resolved_path) - 1);
        }
    }

    mango_task_t *init_task = NULL;
    kern_return_t kr = mango_task_create(&init_task, "init");
    if (kr != KERN_SUCCESS) {
        klog_err("could not create init task (%d)\n", kr);
        return kr;
    }

    strncpy(init_task->binary_path, resolved_path,
            sizeof(init_task->binary_path) - 1);
    strncpy(init_task->userfs_root, userfs_root,
            sizeof(init_task->userfs_root) - 1);

    /* Build argv for the init process */
    const char *argv[] = { resolved_path, NULL };

    char port_str[32];
    snprintf(port_str, sizeof(port_str), "%d", init_task->bootstrap_port);

    /* Set environment for the child process to inherit */
    kal_env_set("USERFSROOT", userfs_root, 1);
    kal_env_set("MANGO_BOOTSTRAP_PORT", port_str, 1);

    kal_spawn_args_t spawn = {
        .exec_path = resolved_path,
        .argv = argv,
        .envp = NULL   /* inherit current environment */
    };

    kal_pid_t pid;
    if (kal_process_spawn(&spawn, &pid) < 0) {
        klog_err("fork failed\n");
        mango_task_terminate(init_task);
        return KERN_FAILURE;
    }

    init_task->host_pid = pid;
    init_task->running = TRUE;

    klog_info("init started (pid %d, task id %d)\n", pid, init_task->id);

    return KERN_SUCCESS;
}
