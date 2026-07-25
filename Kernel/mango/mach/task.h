/*
 * mango/mach/task.h
 *
 * C wrapper functions for task management.
 */

#ifndef MANGO_MACH_TASK_H
#define MANGO_MACH_TASK_H

#include "mach_types.h"
#include "../task/MangoTask.h"

kern_return_t mango_task_init(void);
kern_return_t mango_task_create(const char *name, mango_task_t **out);
kern_return_t mango_task_terminate(mango_task_t *task);

#endif /* MANGO_MACH_TASK_H */
