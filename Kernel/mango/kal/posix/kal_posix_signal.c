/*
 * mango/kal/posix/kal_posix_signal.c
 *
 * POSIX backend for KAL signal handling.
 */

#include "../kal_signal.h"

#include <signal.h>

kal_sighandler_t kal_signal(int sig, kal_sighandler_t handler)
{
    return (kal_sighandler_t)signal(sig, (void (*)(int))handler);
}
