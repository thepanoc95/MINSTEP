#objc
/*
 * pwd.m - Print working directory
 * Simple implementation for MINSTEP userland.
 */

#include "bsd.h"
#include <stdio.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    char buf[1024];
    if (getcwd(buf, sizeof(buf)) != NULL) {
        printf("%s\n", buf);
        return 0;
    }

    perror("pwd");
    return 1;
}