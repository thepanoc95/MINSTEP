/*
 * mango/kal/kal_terminal.h
 *
 * Kernel Abstraction Layer -- terminal I/O.
 *
 * Abstracts raw-mode terminal manipulation for the boot prompt
 * and kernel console.
 */

#ifndef MANGO_KAL_TERMINAL_H
#define MANGO_KAL_TERMINAL_H

#include "kal_platform.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -----------------------------------------------------------------------
 *  Terminal handle
 * ----------------------------------------------------------------------- */

typedef struct kal_terminal {
    void *_impl;    /* Backend-specific state */
} kal_terminal_t;

/* -----------------------------------------------------------------------
 *  Operations
 * ----------------------------------------------------------------------- */

/* Save the current terminal state. */
int kal_terminal_save(kal_terminal_t *t);

/* Restore a previously saved terminal state. */
int kal_terminal_restore(kal_terminal_t *t);

/* Put the terminal into raw (non-canonical) mode. */
int kal_terminal_raw(kal_terminal_t *t);

#ifdef __cplusplus
}
#endif

#endif /* MANGO_KAL_TERMINAL_H */
