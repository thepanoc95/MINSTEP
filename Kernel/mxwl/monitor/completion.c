#include "monitor.h"

int mx_monitor_complete(const char *prefix, char *buf, size_t n)
{
    if (!prefix || !buf || n == 0)
        return 0;

    buf[0] = '\0';
    const char *best = NULL;
    size_t plen = libkern_strlen(prefix);

    mx_cmd_t *cmd = mx_monitor_first_command();
    while (cmd) {
        if (libkern_strncmp(cmd->name, prefix, plen) == 0) {
            if (!best) {
                best = cmd->name;
            } else {
                size_t i;
                for (i = 0; best[i] && cmd->name[i] &&
                            best[i] == cmd->name[i]; i++)
                    ;
                if (i < libkern_strlen(best)) {
                    char tmp[MX_MONITOR_MAX_LINE];
                    libkern_strncpy(tmp, best, sizeof(tmp));
                    tmp[i] = '\0';
                    libkern_strncpy(buf, tmp, n);
                    return 1;
                }
            }
        }
        cmd = mx_monitor_next_command(cmd);
    }

    if (best) {
        libkern_strncpy(buf, best, n);
        return 1;
    }

    return 0;
}

void mx_monitor_list_completions(const char *prefix,
                                  char *out, size_t out_size)
{
    if (!prefix || !out || out_size == 0)
        return;

    out[0] = '\0';
    size_t plen = libkern_strlen(prefix);
    size_t pos = 0;

    mx_cmd_t *cmd = mx_monitor_first_command();
    while (cmd) {
        if (libkern_strncmp(cmd->name, prefix, plen) == 0) {
            size_t clen = libkern_strlen(cmd->name);
            if (pos + clen + 2 < out_size) {
                if (pos > 0)
                    out[pos++] = ' ';
                libkern_memcpy(out + pos, cmd->name, clen);
                pos += clen;
            }
        }
        cmd = mx_monitor_next_command(cmd);
    }
    out[pos] = '\0';
}
