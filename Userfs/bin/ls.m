#objc
/*
 * ls.m - List directory contents
 * Simple implementation for MINSTEP userland.
 */

#include <bsd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    const char *path = ".";
    int i;

    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-l") == 0) {
            continue;
        }
        if (argv[i][0] != '-') {
            path = argv[i];
        }
    }

    DIR *dir = opendir(path);
    if (!dir) {
        fprintf(stderr, "ls: %s: ", path);
        perror("");
        return 1;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        printf("%s\n", entry->d_name);
    }

    closedir(dir);
    return 0;
}