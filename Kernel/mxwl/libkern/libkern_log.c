#include "libkern_log.h"

#include <stdio.h>
#include <stdarg.h>

static int _log_level = LOG_INFO;

static const char *_level_names[] = {
    "EMERG", "CRIT", "ERR", "WARN", "NOTICE", "INFO", "DEBUG"
};

void libkern_log(int level, const char *fmt, ...)
{
    if (level > _log_level)
        return;

    va_list args;
    va_start(args, fmt);

    if (level >= 0 && level <= LOG_DEBUG) {
        fprintf(stderr, "[%s] ", _level_names[level]);
    }

    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");
    va_end(args);
}

void libkern_log_set_level(int level)
{
    _log_level = level;
}

int libkern_log_get_level(void)
{
    return _log_level;
}
