/*
 * mxwl/mach/task.h
 *
 * C wrapper functions for task management.
 */

#ifndef MXWL_MACH_TASK_H
#define MXWL_MACH_TASK_H

#include "mach_types.h"
#include "../task/MaxxwellTask.h"

kern_return_t mxwl_task_init(void);
kern_return_t mxwl_task_create(const char *name, mxwl_task_t **out);
kern_return_t mxwl_task_terminate(mxwl_task_t *task);

#endif /* MXWL_MACH_TASK_H */
