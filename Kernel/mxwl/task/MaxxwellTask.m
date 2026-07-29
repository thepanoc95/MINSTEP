#objc
/*
 * mxwl/task/MaxxwellTask.m
 *
 * Objective-C implementation of task and thread management.
 */

#import "MaxxwellTask.h"
#import "../mach/klog.h"
#import "../ipc/ipc.h"
#import "../mach/mach_port.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>

/* -----------------------------------------------------------------------
 *  Global task table (C globals, shared with C code)
 * ----------------------------------------------------------------------- */

mxwl_task_t  mxwl_task_table[TASK_MAX];
int           mxwl_task_count = 0;
mxwl_task_t *mxwl_current_task = NULL;

@implementation MaxxwellTask

- (id)init {
    self = [super init];
    if (self) {
        memset(mxwl_task_table, 0, sizeof(mxwl_task_table));
        mxwl_task_count = 0;
        mxwl_current_task = NULL;
    }
    return self;
}

- (id)free {
    for (int i = TASK_MAX - 1; i >= 0; i--) {
        if (mxwl_task_table[i].in_use && mxwl_task_table[i].id != 0) {
            mxwl_task_t *t = &mxwl_task_table[i];
            [self terminate:t];
        }
    }
    return [super free];
}

- (kern_return_t)initKernel {
    mxwl_task_t *task = &mxwl_task_table[0];

    memset(task, 0, sizeof(mxwl_task_t));
    task->id        = 0;
    task->host_pid  = getpid();
    task->in_use    = YES;
    task->running   = YES;
    task->terminated = NO;
    strncpy(task->name, "kernel", TASK_NAME_MAX - 1);

    task->bootstrap_port = ipc_bootstrap_port;

    mxwl_task_count = 1;
    mxwl_current_task = task;

    klog_info("Task table initialized, kernel task (pid %d, bootstrap port %d).\n",
              task->host_pid, task->bootstrap_port);
    return KERN_SUCCESS;
}

- (kern_return_t)createTaskWithName:(const char *)name outTask:(mxwl_task_t **)out {
    if (mxwl_task_count >= TASK_MAX) {
        klog_err("task table full\n");
        return KERN_NO_SPACE;
    }

    mxwl_task_t *task = NULL;
    for (int i = 0; i < TASK_MAX; i++) {
        if (!mxwl_task_table[i].in_use) {
            task = &mxwl_task_table[i];
            break;
        }
    }

    if (!task) return KERN_NO_SPACE;

    memset(task, 0, sizeof(mxwl_task_t));
    task->id = (int)(task - mxwl_task_table);
    task->host_pid = -1;
    task->in_use = YES;
    task->running = NO;
    task->terminated = NO;

    if (name) {
        strncpy(task->name, name, TASK_NAME_MAX - 1);
        task->name[TASK_NAME_MAX - 1] = '\0';
    } else {
        snprintf(task->name, TASK_NAME_MAX, "task%d", task->id);
    }

    task->bootstrap_port = mach_port_allocate(MACH_PORT_RIGHT_RECEIVE);
    if (task->bootstrap_port == MACH_PORT_NULL) {
        task->in_use = NO;
        return KERN_FAILURE;
    }

    mxwl_task_count++;

    klog_sub_info("task", "created: %s (id %d, bootstrap port %d)\n",
                   task->name, task->id, task->bootstrap_port);

    if (out) *out = task;
    return KERN_SUCCESS;
}

- (kern_return_t)terminate:(mxwl_task_t *)task {
    if (!task || !task->in_use) return KERN_INVALID_TASK;

    if (task->host_pid > 0) {
        kill(task->host_pid, SIGTERM);
        waitpid(task->host_pid, NULL, WNOHANG);
    }

    for (int i = 0; i < task->port_count; i++) {
        mach_port_deallocate(task->port_objects[i]);
    }

    if (task->bootstrap_port != MACH_PORT_NULL) {
        mach_port_destroy(task->bootstrap_port);
    }

    task->running = NO;
    task->terminated = YES;
    task->in_use = NO;
    mxwl_task_count--;

    klog_sub_info("task", "terminated: %s (id %d)\n", task->name, task->id);
    return KERN_SUCCESS;
}

- (mxwl_task_t *)lookup:(mach_task_t)task_id {
    if (task_id < 0 || task_id >= TASK_MAX) return NULL;
    mxwl_task_t *task = &mxwl_task_table[task_id];
    if (!task->in_use) return NULL;
    return task;
}

- (kern_return_t)insertPort:(mxwl_task_t *)task :(mach_port_name_t)name :(mach_port_t)port {
    if (!task || task->port_count >= TASK_PORT_MAX) {
        return KERN_NO_SPACE;
    }
    task->ports[task->port_count] = name;
    task->port_objects[task->port_count] = port;
    task->port_count++;
    return KERN_SUCCESS;
}

- (kern_return_t)removePort:(mxwl_task_t *)task :(mach_port_name_t)name {
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

- (mach_port_t)bootstrapPort:(mxwl_task_t *)task {
    if (!task) return MACH_PORT_NULL;
    return task->bootstrap_port;
}

- (kern_return_t)threadCreate:(mxwl_task_t *)task :(mxwl_thread_t **)out {
    if (!task || task->thread_count >= THREAD_MAX) {
        return KERN_NO_SPACE;
    }
    mxwl_thread_t *thread = malloc(sizeof(mxwl_thread_t));
    if (!thread) return KERN_FAILURE;
    memset(thread, 0, sizeof(mxwl_thread_t));
    thread->id = task->thread_count;
    thread->host_tid = -1;
    thread->active = YES;
    thread->suspended = NO;
    task->threads[task->thread_count] = thread;
    task->thread_count++;
    if (out) *out = thread;
    return KERN_SUCCESS;
}

- (kern_return_t)threadTerminate:(mxwl_thread_t *)thread {
    if (!thread) return KERN_INVALID_ARGUMENT;
    thread->active = NO;
    free(thread);
    return KERN_SUCCESS;
}

- (kern_return_t)threadSuspend:(mxwl_thread_t *)thread {
    if (!thread) return KERN_INVALID_ARGUMENT;
    thread->suspended = YES;
    return KERN_SUCCESS;
}

- (kern_return_t)threadResume:(mxwl_thread_t *)thread {
    if (!thread) return KERN_INVALID_ARGUMENT;
    thread->suspended = NO;
    return KERN_SUCCESS;
}

- (kern_return_t)launchInit:(const char *)root initPath:(const char *)init_path_arg {
    if (!root) return KERN_INVALID_ARGUMENT;

    char init_path[1024];

    if (init_path_arg && init_path_arg[0]) {
        strncpy(init_path, init_path_arg, sizeof(init_path) - 1);
        init_path[sizeof(init_path) - 1] = '\0';
        klog_info("using init: %s\n", init_path);
    } else {
        snprintf(init_path, sizeof(init_path), "%s/private/init", root);
        klog_info("searching for init: %s\n", init_path);
    }

    if (access(init_path, X_OK) != 0) {
        klog_warn("%s not found or not executable\n", init_path);
        klog_info("no init available, staying in-kernel\n");
        return KERN_FAILURE;
    }

    mxwl_task_t *init_task = NULL;
    kern_return_t kr = [self createTaskWithName:"init" outTask:&init_task];
    if (kr != KERN_SUCCESS) {
        klog_err("could not create init task (%d)\n", kr);
        return kr;
    }

    strncpy(init_task->binary_path, init_path,
            sizeof(init_task->binary_path) - 1);
    strncpy(init_task->userfs_root, root,
            sizeof(init_task->userfs_root) - 1);

    pid_t pid = fork();
    if (pid < 0) {
        klog_err("fork failed: %s\n", strerror(errno));
        [self terminate:init_task];
        return KERN_FAILURE;
    }

    if (pid == 0) {
        setenv("USERFSROOT", root, 1);
        char port_str[32];
        snprintf(port_str, sizeof(port_str), "%d", init_task->bootstrap_port);
        setenv("MXWL_BOOTSTRAP_PORT", port_str, 1);
        execl(init_path, init_path, (char *)NULL);
        klog_err("exec failed: %s\n", strerror(errno));
        (void)(1); /* _exit translated incorrectly by preprocessor */
    }

    init_task->host_pid = pid;
    init_task->running = YES;

    klog_info("init started (pid %d, task id %d)\n", pid, init_task->id);

    return KERN_SUCCESS;
}

@end
