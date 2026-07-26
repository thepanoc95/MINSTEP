/*
 * @BSD_LICENSE_HEADER BEGIN
 * Copyright (c) 2026, thepanoc95 All rights reserved.

  * Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
  *  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  *  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  *  * All advertising materials mentioning features or use of this software must display the following acknowledgement: This product includes software developed by thepanoc95.
  *  * Neither the name of thepanoc95 nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THEPANOC95 AS IS AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THEPANOC95 BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * THIS SOFTWARE IS PROVIDED BY THEPANOC95 AS IS AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THEPANOC95 BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.[6]
 *
 * @BSD_LICENSE_HEADER END
 */

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

    // execvp("clear", NULL);

    [bootstrapper uncompress];
    [bootstrapper chrootit];

    [bootstrapper free];
    return 0;
}
