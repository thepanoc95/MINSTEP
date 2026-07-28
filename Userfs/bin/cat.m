#objc
/*
 * cat.m - Concatenate and display files
 * Simple implementation for MINSTEP userland.
 */

#include "bsd.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#define CAT_BUF_SIZE 4096

int main(int argc, char *argv[])
{
    char buf[CAT_BUF_SIZE];
    ssize_t n;
    int i;
    int fd;

    if (argc == 1) {
        while ((n = read(STDIN_FILENO, buf, sizeof(buf))) > 0) {
            write(STDOUT_FILENO, buf, n);
        }
        return 0;
    }

    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-") == 0) {
            while ((n = read(STDIN_FILENO, buf, sizeof(buf))) > 0) {
                write(STDOUT_FILENO, buf, n);
            }
            continue;
        }

        fd = open(argv[i], O_RDONLY);
        if (fd < 0) {
            fprintf(stderr, "cat: %s: ", argv[i]);
            perror("");
            return 1;
        }

        while ((n = read(fd, buf, sizeof(buf))) > 0) {
            write(STDOUT_FILENO, buf, n);
        }

        close(fd);
    }

    return 0;
}