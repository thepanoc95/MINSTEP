#include "libkern_util.h"

#include <stdio.h>
#include <stdarg.h>

size_t libkern_hexdump(char *buf, size_t n, const void *data, size_t len)
{
    const unsigned char *bytes = (const unsigned char *)data;
    size_t written = 0;

    for (size_t i = 0; i < len && written < n; i += 16) {
        size_t rem = len - i;
        if (rem > 16) rem = 16;

        written += (size_t)snprintf(buf + written, n - written,
                                     "%08lx  ", (unsigned long)i);

        for (size_t j = 0; j < 16; j++) {
            if (j < rem)
                written += (size_t)snprintf(buf + written, n - written,
                                             "%02x ", bytes[i + j]);
            else
                written += (size_t)snprintf(buf + written, n - written,
                                             "   ");
            if (j == 7)
                written += (size_t)snprintf(buf + written, n - written, " ");
        }

        written += (size_t)snprintf(buf + written, n - written, " |");

        for (size_t j = 0; j < rem && written < n; j++) {
            unsigned char c = bytes[i + j];
            buf[written++] = (c >= 32 && c <= 126) ? c : '.';
        }

        if (written < n)
            buf[written++] = '|';
        if (written < n)
            buf[written++] = '\n';
    }

    if (written < n)
        buf[written] = '\0';
    else if (n > 0)
        buf[n - 1] = '\0';

    return written;
}

int libkern_snprintf(char *buf, size_t n, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    int ret = vsnprintf(buf, n, fmt, args);
    va_end(args);
    return ret;
}

int libkern_atoi(const char *s)
{
    if (!s) return 0;
    int neg = 0;
    while (*s == ' ' || *s == '\t') s++;
    if (*s == '-') { neg = 1; s++; }
    else if (*s == '+') s++;
    int val = 0;
    while (*s >= '0' && *s <= '9')
        val = val * 10 + (*s++ - '0');
    return neg ? -val : val;
}

long libkern_atol(const char *s)
{
    if (!s) return 0;
    int neg = 0;
    while (*s == ' ' || *s == '\t') s++;
    if (*s == '-') { neg = 1; s++; }
    else if (*s == '+') s++;
    long val = 0;
    while (*s >= '0' && *s <= '9')
        val = val * 10 + (*s++ - '0');
    return neg ? -val : val;
}
