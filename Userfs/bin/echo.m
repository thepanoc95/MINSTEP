#objc
/*
 * echo.m - Display a line of text
 * Simple implementation for MINSTEP userland.
 */

#include "bsd.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[])
{
    int i;
    int first = 1;

    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-n") == 0) {
            continue;
        }

        if (!first) {
            putchar(' ');
        }
        first = 0;

        fputs(argv[i], stdout);
    }

    putchar('\n');
    return 0;
}