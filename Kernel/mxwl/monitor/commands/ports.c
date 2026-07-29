#include "../monitor.h"
#include "ports.h"
#include "../../mach/mach_port.h"

static kern_return_t _ports_handler(int argc, char **argv,
                                     char *out, size_t out_size)
{
    (void)argc; (void)argv;

    size_t pos = 0;
    pos += libkern_snprintf(out + pos, out_size - pos,
                            "IPC Ports:\n");

    int shown = 0;
    for (int i = 0; i < mxwl_port_table_count; i++) {
        mach_port_object_t *p = &mxwl_port_table[i];
        if (p->in_use) {
            pos += libkern_snprintf(out + pos, out_size - pos,
                                    "  [%d] name=%d  refs=%d  fd=%d  queue=%d\n",
                                    i, p->name, p->ref_count,
                                    p->fd, p->queue_count);
            shown++;
        }
    }

    if (shown == 0) {
        pos += libkern_snprintf(out + pos, out_size - pos,
                                "  (no active ports)\n");
    }

    return KERN_SUCCESS;
}

mx_cmd_t _ports_cmd = {
    .name       = "ports",
    .help_short = "List IPC ports",
    .help_long  = "Usage: ports\n"
                  "Show all active IPC ports in the system.",
    .handler    = _ports_handler,
};
