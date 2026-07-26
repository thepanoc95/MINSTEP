

#objc
#import <objc/Object.h>
#import <stdio.h>
#import <stdlib.h>
#import <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define BOOTSTRAPPER_DIR "/tmp/minstepbootstrap"

@interface UserfsDecompress : Object
   - (void)uncompress;
   - (void)chrootit;
   - (void)cleanup;
@end

@implementation UserfsDecompress

- (void)uncompress {
    char cmd_buf[256];

    snprintf(cmd_buf, sizeof(cmd_buf), "mkdir -p %s/root", BOOTSTRAPPER_DIR);
    system(cmd_buf);

    printf("uncompressing userfs");
    fflush(stdout);

    pid_t pid = fork();

    if (pid < 0) {
        printf(" [FAILED to fork]\n");
        return;
    } 
    else if (pid == 0) {
        char dest_dir[256];
        snprintf(dest_dir, sizeof(dest_dir), "%s/root", BOOTSTRAPPER_DIR);

        char *args[] = {"tar", "-xf", "userfs.tar", "-C", dest_dir, NULL};
        execvp("tar", args);
        
        exit(1);
    } 
    else {
        int status;

        while (waitpid(pid, &status, WNOHANG) == 0) {
            printf(".");
            fflush(stdout);
            usleep(200000);
        }

        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            printf(" [OK]\n");
            printf("osbootstrap.m: Now booting the kernel!\n");
        } else {
            printf(" [FAILED]\n");
	    NSLog(@"panic: Failed to uncompress the Userfs image!");
        }
    }
}

- (void)cleanup {
    char cmd_buf[256];
    snprintf(cmd_buf, sizeof(cmd_buf), "rm -rf %s", BOOTSTRAPPER_DIR);
    system(cmd_buf);
    printf("osbootstrap.m: Cleaning up Userfs:0:%s\n", BOOTSTRAPPER_DIR);
}

- (void)chrootit {
    char cmd_buf[256];
    snprintf(cmd_buf, sizeof(cmd_buf), "chroot %s/root /boot1 -kernel /mach", BOOTSTRAPPER_DIR);
    system(cmd_buf);
}
@end

int main(int argc, char *argv[]) {
    UserfsDecompress *bootstrapper = [UserfsDecompress alloc];
    bootstrapper = [bootstrapper init];
    
    [bootstrapper uncompress];
    [bootstrapper chrootit];

    [bootstrapper free]; 
    return 0;
}

