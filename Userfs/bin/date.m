#objc
/*
 * date.m - Print date and time
 * Simple implementation for MINSTEP userland.
 */

#include "bsd.h"
#include <stdio.h>
#include <time.h>

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    time_t now = time(NULL);
    if (now != (time_t)-1) {
        struct tm *tm = localtime(&now);
        if (tm) {
            char buf[128];
            if (strftime(buf, sizeof(buf), "%a %b %d %H:%M:%S %Z %Y", tm) > 0) {
                printf("%s\n", buf);
                return 0;
            }
        }
    }

    perror("date");
    return 1;
}