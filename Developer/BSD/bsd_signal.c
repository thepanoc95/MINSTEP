/*
 * Developer/BSD/bsd_signal.c
 *
 * Signal handling for the BSD server.
 * Manages per-process signal actions, pending signals,
 * and signal delivery.
 */

#include "bsd.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <errno.h>

/* -----------------------------------------------------------------------
 *  Signal table initialization
 * ----------------------------------------------------------------------- */

kern_return_t bsd_signal_init(bsd_process_t *proc)
{
    if (!proc) return KERN_INVALID_TASK;

    proc->signal_pending = 0;
    proc->signal_mask = 0;
    proc->signal_actions = 0;

    return KERN_SUCCESS;
}

/* -----------------------------------------------------------------------
 *  kill (host-level signal delivery)
 * ----------------------------------------------------------------------- */

int bsd_signal_kill(pid_t pid, int sig)
{
    if (pid <= 0) {
        return kill(pid, sig);
    }

    return kill(pid, sig);
}

/* -----------------------------------------------------------------------
 *  Signal delivery to process
 * ----------------------------------------------------------------------- */

void bsd_signal_deliver(bsd_process_t *proc)
{
    if (!proc || !proc->running) return;

    unsigned long pending = proc->signal_pending & ~proc->signal_mask;
    if (pending == 0) return;

    for (int sig = 1; sig < BSD_MAX_SIGNALS; sig++) {
        if (!(pending & (1UL << sig))) continue;

        proc->signal_pending &= ~(1UL << sig);

        switch (sig) {
        case SIGCHLD:
        case SIGURG:
        case SIGWINCH:
            break;
        default:
            fprintf(stderr, "[bsd] process %d killed by signal %d\n",
                    proc->pid, sig);
            proc->running = NO;
            proc->exit_signal = sig;
            break;
        }
    }
}