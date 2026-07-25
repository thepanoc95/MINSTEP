/*
 * mango/mach/task.c
 *
 * C wrapper functions for task management.
 * The actual implementation is in MangoTask.m (Objective-C).
 */

#include "task.h"
#include "../task/MangoTask.h"

kern_return_t mango_task_init(void)
{
    /* Task initialization is handled by MangoTask Objective-C class */
    return KERN_SUCCESS;
}

kern_return_t mango_task_create(const char *name, mango_task_t **out)
{
    /* Task creation is handled by MangoTask Objective-C class */
    /* This stub is used by mach_loader.c before ObjC runtime is available */
    (void)name;
    if (out) *out = NULL;
    return KERN_FAILURE;
}

kern_return_t mango_task_terminate(mango_task_t *task)
{
    /* Task termination is handled by MangoTask Objective-C class */
    /* This stub is used by mach_loader.c before ObjC runtime is available */
    (void)task;
    return KERN_FAILURE;
}
