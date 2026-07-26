#objc
#import <objc/Object.h>
#import <stdio.h>
#import <stdlib.h>
#import <string.h>
#import <unistd.h>
#import <sys/utsname.h>

#define INIT_VERSION "0.1"

@interface SystemInit : Object
- (void)banner;
- (void)setupEnv;
@end

@implementation SystemInit

- (void)banner {
    struct utsname uts;
    const char *machine = "unknown";

    if (uname(&uts) == 0)
        machine = uts.machine;

    fprintf(stderr, "MINSTEP %s [%s] - /private/init starting...\n",
            INIT_VERSION, machine);
}

- (void)setupEnv {
    setenv("PATH", "/bin:/sbin:/usr/bin:/usr/sbin", 1);
    setenv("HOME", "/me", 1);
    setenv("SHELL", "/bin/sh", 1);
    setenv("MINSTEP_VERSION", INIT_VERSION, 1);
    setenv("USERFSROOT", "/", 1);

    struct utsname uts;
    if (uname(&uts) == 0)
        sethostname(uts.nodename, strlen(uts.nodename));
}

@end

int main(int argc, char *argv[]) {
    SystemInit *init = [SystemInit new];

    [init banner];
    [init setupEnv];

    [init free];

    fprintf(stderr, "MINSTEP %s - launching /bin/sh\n", INIT_VERSION);

    execl("/bin/sh", "/bin/sh", (char *)NULL);
    fprintf(stderr, "init: exec /bin/sh failed\n");

    ((void(*)(int))_exit)(1);
}
