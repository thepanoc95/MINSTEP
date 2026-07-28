#include "libkern_panic.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

void libkern_panic(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    fprintf(stderr, "*** KERNEL PANIC *** ");
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");
    va_end(args);

    fflush(stderr);
    abort();
}

void libkern_panic_dump(void)
{
}
