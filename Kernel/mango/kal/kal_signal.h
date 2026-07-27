/*
 * mango/kal/kal_signal.h
 *
 * Kernel Abstraction Layer -- signal handling.
 *
 * Abstracts signal registration and delivery so the kernel
 * can handle shutdown signals portably.
 */

#ifndef MANGO_KAL_SIGNAL_H
#define MANGO_KAL_SIGNAL_H

#include "kal_platform.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Signal numbers (host-independent values where possible). */
#define KAL_SIGTERM     15
#define KAL_SIGINT       2
#define KAL_SIGCHLD     17
#define KAL_SIG_IGN     ((kal_sighandler_t)1)

typedef void (*kal_sighandler_t)(int sig);

/* Install a signal handler.  Returns the previous handler. */
kal_sighandler_t kal_signal(int sig, kal_sighandler_t handler);

#ifdef __cplusplus
}
#endif

#endif /* MANGO_KAL_SIGNAL_H */
