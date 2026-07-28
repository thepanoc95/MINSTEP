#include "monitor.h"

static mx_cmd_t *_cmd_list = NULL;

kern_return_t mx_monitor_register_command(mx_cmd_t *cmd)
{
    if (!cmd || !cmd->name || !cmd->handler)
        return KERN_INVALID_ARGUMENT;

    cmd->next = _cmd_list;
    _cmd_list = cmd;
    return KERN_SUCCESS;
}

mx_cmd_t *mx_monitor_find_command(const char *name)
{
    mx_cmd_t *p = _cmd_list;
    while (p) {
        if (libkern_strcmp(p->name, name) == 0)
            return p;
        p = p->next;
    }
    return NULL;
}

mx_cmd_t *mx_monitor_first_command(void)
{
    return _cmd_list;
}

mx_cmd_t *mx_monitor_next_command(mx_cmd_t *cmd)
{
    return cmd ? cmd->next : NULL;
}
