#include "libkern_assert.h"
#include "libkern_panic.h"
#include "libkern_log.h"

#include <stdio.h>
#include <stdlib.h>

void libkern_assert_fail(const char *expr, const char *file, int line, const char *func)
{
    libkern_log(LOG_CRIT, "ASSERTION FAILED: %s (%s:%d in %s)", expr, file, line, func);
    libkern_panic("assertion failed: %s", expr);
}
