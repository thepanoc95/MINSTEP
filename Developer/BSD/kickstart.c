/*
 * Developer/BSD/kickstart.c
 *
 * Userland kickstarter program for MinSTEP BSD subsystem.
 * Provides a command prompt ("----> ") when no shell is auto-started.
 *
 * Copyright (c) 2026 MinSTEP Project
 * Licensed under the MIT License.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <poll.h>
#include <errno.h>
#include <sys/wait.h>

#define KICKSTART_PROMPT "----> "
#define KICKSTART_VERSION "0.1.0"
#define MAX_CMDLINE 1024

static int kickstart_bootstrap_port = -1;

static void kickstart_init(void)
{
    const char *port_str = getenv("MANGO_BOOTSTRAP_PORT");
    if (port_str) {
        kickstart_bootstrap_port = atoi(port_str);
    }
}

static void kickstart_execute_command(const char *cmd)
{
    if (!cmd || cmd[0] == '\0') return;

    if (strcmp(cmd, "exit") == 0) {
        exit(0);
    }

    if (strcmp(cmd, "help") == 0 || strcmp(cmd, "?") == 0) {
        fprintf(stderr, "MinSTEP Kickstart commands:\n");
        fprintf(stderr, "  help     Show this help\n");
        fprintf(stderr, "  exit     Exit kickstart\n");
        fprintf(stderr, "  #cmd     Execute command via system()\n");
        fprintf(stderr, "\nCommands starting with '#' are shell commands.\n");
        fprintf(stderr, "Other commands are executed if executable.\n");
        return;
    }

    if (cmd[0] == '#') {
        const char *shell_cmd = cmd + 1;
        while (*shell_cmd == ' ') shell_cmd++;
        if (*shell_cmd) {
            int result = system(shell_cmd);
            if (result != 0) {
                fprintf(stderr, "[kickstart] command failed (status %d)\n", result);
            }
        }
        return;
    }

    if (access(cmd, X_OK) == 0) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork failed");
        } else if (pid == 0) {
            execl(cmd, cmd, (char *)NULL);
            perror("exec failed");
            _exit(1);
        } else {
            int status;
            waitpid(pid, &status, 0);
        }
    } else {
        fprintf(stderr, "[kickstart] not found or not executable: %s\n", cmd);
    }
}

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    char buffer[MAX_CMDLINE];
    int buf_idx = 0;
    struct pollfd pfd;

    kickstart_init();

    fprintf(stderr, "MinSTEP Kickstart v%s\n", KICKSTART_VERSION);

    if (kickstart_bootstrap_port < 0) {
        fprintf(stderr, "[kickstart] warning: no bootstrap port available\n");
    }

    pfd.fd = STDIN_FILENO;
    pfd.events = POLLIN;

    fprintf(stderr, "%s", KICKSTART_PROMPT);
    fflush(stderr);

    while (1) {
        int ret = poll(&pfd, 1, 1000);

        if (ret < 0) {
            if (errno == EINTR) continue;
            break;
        }

        if (ret > 0 && (pfd.revents & POLLIN)) {
            char c;
            ssize_t n = read(STDIN_FILENO, &c, 1);
            if (n <= 0) break;

            if (c == '\n' || c == '\r') {
                buffer[buf_idx] = '\0';
                fprintf(stderr, "\n");

                if (buf_idx > 0) {
                    kickstart_execute_command(buffer);
                }

                buf_idx = 0;
                memset(buffer, 0, sizeof(buffer));

                fprintf(stderr, "%s", KICKSTART_PROMPT);
                fflush(stderr);
            } else if (c == 127 || c == 8) {
                if (buf_idx > 0) {
                    buf_idx--;
                    buffer[buf_idx] = '\0';
                    fprintf(stderr, "\b \b");
                    fflush(stderr);
                }
            } else if (c >= 32 && c <= 126 && buf_idx < (MAX_CMDLINE - 1)) {
                buffer[buf_idx++] = c;
                fputc(c, stderr);
                fflush(stderr);
            }
        }
    }

    fprintf(stderr, "\n");
    return 0;
}