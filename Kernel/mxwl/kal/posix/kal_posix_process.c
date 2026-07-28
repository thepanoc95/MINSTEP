/*
 * mxwl/kal/posix/kal_posix_process.c
 *
 * POSIX backend for KAL process management.
 */

#include "../kal_process.h"

#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

extern char **environ;

kal_pid_t kal_getpid(void)
{
    return (kal_pid_t)getpid();
}

int kal_process_spawn(kal_spawn_args_t *args, kal_pid_t *out_pid)
{
    if (!args || !args->exec_path) return -1;

    pid_t pid = fork();
    if (pid < 0) return -1;

    if (pid == 0) {
        /* Child */
        if (args->envp) {
            /* Clear existing env and set provided env */
            environ = (char **)args->envp;
        }

        if (args->argv) {
            execvp(args->exec_path, (char *const *)args->argv);
        } else {
            execl(args->exec_path, args->exec_path, (char *)NULL);
        }

        _exit(127);
    }

    if (out_pid) *out_pid = (kal_pid_t)pid;
    return 0;
}

int kal_process_kill(kal_pid_t pid, int sig)
{
    return kill((pid_t)pid, sig);
}

void kal_process_exit(int code)
{
    _exit(code);
}

kal_pid_t kal_process_reap(int *out_status)
{
    int status = 0;
    pid_t pid = waitpid(-1, &status, WNOHANG);
    if (pid > 0 && out_status) *out_status = status;
    return (kal_pid_t)pid;
}

kal_pid_t kal_process_wait(kal_pid_t pid, int *out_status)
{
    int status = 0;
    pid_t result = waitpid((pid_t)pid, &status, 0);
    if (result > 0 && out_status) *out_status = status;
    return (kal_pid_t)result;
}

const char *kal_env_get(const char *name)
{
    return getenv(name);
}

int kal_env_set(const char *name, const char *value, int overwrite)
{
    return setenv(name, value, overwrite);
}

int kal_can_exec(const char *path)
{
    return (access(path, X_OK) == 0) ? 1 : 0;
}
