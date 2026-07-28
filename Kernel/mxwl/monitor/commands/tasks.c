#include "../monitor.h"
#include "tasks.h"
#include "../../task/MaxxwellTask.h"

static kern_return_t _tasks_handler(int argc, char **argv,
                                     char *out, size_t out_size)
{
    (void)argc; (void)argv;

    size_t pos = 0;
    pos += libkern_snprintf(out + pos, out_size - pos,
                            "Tasks (%d total):\n", mxwl_task_count);

    for (int i = 0; i < mxwl_task_count; i++) {
        mxwl_task_t *t = &mxwl_task_table[i];
        if (t->name[0]) {
            pos += libkern_snprintf(out + pos, out_size - pos,
                                    "  %-12s  id=%d  pid=%d  running=%d  threads=%d\n",
                                    t->name, t->id, t->host_pid,
                                    t->running, t->thread_count);
        }
    }

    return KERN_SUCCESS;
}

static kern_return_t _threads_handler(int argc, char **argv,
                                       char *out, size_t out_size)
{
    (void)argc; (void)argv;

    libkern_snprintf(out, out_size, "Thread listing not yet implemented\n");
    return KERN_SUCCESS;
}

mx_cmd_t _tasks_cmd = {
    .name       = "tasks",
    .help_short = "List running tasks",
    .help_long  = "Usage: tasks\n"
                  "Show all tasks and their basic information.",
    .handler    = _tasks_handler,
};

mx_cmd_t _threads_cmd = {
    .name       = "threads",
    .help_short = "List threads in a task",
    .help_long  = "Usage: threads [task]\n"
                  "Show thread information.",
    .handler    = _threads_handler,
};
