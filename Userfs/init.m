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
