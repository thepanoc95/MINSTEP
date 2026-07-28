#include "../monitor.h"
#include "help.h"

static kern_return_t _help_handler(int argc, char **argv,
                                    char *out, size_t out_size)
{
    size_t pos = 0;

    if (argc == 1) {
        pos += libkern_snprintf(out + pos, out_size - pos,
                                "Commands:\n");

        mx_cmd_t *cmd = mx_monitor_first_command();
        while (cmd && pos < out_size) {
            pos += libkern_snprintf(out + pos, out_size - pos,
                                    "  %-15s  %s\n",
                                    cmd->name,
                                    cmd->help_short ? cmd->help_short : "");
            cmd = mx_monitor_next_command(cmd);
        }
    } else {
        for (int i = 1; i < argc && pos < out_size; i++) {
            mx_cmd_t *cmd = mx_monitor_find_command(argv[i]);
            if (!cmd) {
                pos += libkern_snprintf(out + pos, out_size - pos,
                                        "No help for: %s\n", argv[i]);
            } else {
                pos += libkern_snprintf(out + pos, out_size - pos,
                                        "%s - %s\n",
                                        cmd->name,
                                        cmd->help_short ? cmd->help_short : "");
                if (cmd->help_long) {
                    pos += libkern_snprintf(out + pos, out_size - pos,
                                            "\n%s\n", cmd->help_long);
                }
            }
        }
    }

    out[out_size - 1] = '\0';
    return KERN_SUCCESS;
}

mx_cmd_t _help_cmd = {
    .name       = "help",
    .help_short = "Display available commands or detailed help",
    .help_long  = "Usage: help [command...]\n"
                  "If called without arguments, lists all commands.\n"
                  "If given command names, shows detailed help for each.",
    .handler    = _help_handler,
};
