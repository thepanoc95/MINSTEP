/*
 * mxwl/kal/kal_process.h
 *
 * Kernel Abstraction Layer -- process management.
 *
 * Abstracts process creation, termination, signalling, and
 * wait/reap operations so the kernel can run on any supported OS.
 */

#ifndef MXWL_KAL_PROCESS_H
#define MXWL_KAL_PROCESS_H

#include "kal_platform.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -----------------------------------------------------------------------
 *  Types
 * ----------------------------------------------------------------------- */

typedef int kal_pid_t;

/* -----------------------------------------------------------------------
 *  Process identity
 * ----------------------------------------------------------------------- */

/* Get the PID of the calling process. */
kal_pid_t kal_getpid(void);

/* -----------------------------------------------------------------------
 *  Process creation
 *
 *  kal_process_spawn() combines fork+exec into a single call.
 *  The child runs exec_path with the given argv and envp.
 *  On success, *out_pid is set to the child PID.
 *  If envp is NULL, the child inherits the parent's environment.
 * ----------------------------------------------------------------------- */

typedef struct kal_spawn_args {
    const char     *exec_path;     /* Path to executable               */
    const char    **argv;          /* Argument vector (NULL-terminated)*/
    const char    **envp;          /* Environment (NULL-terminated), or NULL */
} kal_spawn_args_t;

int kal_process_spawn(kal_spawn_args_t *args, kal_pid_t *out_pid);

/* -----------------------------------------------------------------------
 *  Process termination
 * ----------------------------------------------------------------------- */

/* Terminate a process with the given signal. */
int kal_process_kill(kal_pid_t pid, int sig);

/* Exit the current process with the given code. */
void kal_process_exit(int code) __attribute__((noreturn));

/* -----------------------------------------------------------------------
 *  Process reaping (wait)
 * ----------------------------------------------------------------------- */

/* Reap a child process.  Returns child PID, 0 if none available. */
kal_pid_t kal_process_reap(int *out_status);

/* Wait for a specific child.  Returns child PID. */
kal_pid_t kal_process_wait(kal_pid_t pid, int *out_status);

/* -----------------------------------------------------------------------
 *  Environment
 * ----------------------------------------------------------------------- */

/* Get an environment variable.  Returns NULL if not set. */
const char *kal_env_get(const char *name);

/* Set an environment variable.  Returns 0 on success. */
int kal_env_set(const char *name, const char *value, int overwrite);

/* -----------------------------------------------------------------------
 *  File existence / executability check
 * ----------------------------------------------------------------------- */

/* Check if a file exists and is executable.  Returns 1 if yes. */
int kal_can_exec(const char *path);

#ifdef __cplusplus
}
#endif

#endif /* MXWL_KAL_PROCESS_H */
