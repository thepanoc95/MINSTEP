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
#import <X11/Xlib.h>
#import <stdio.h>
#import <stdlib.h>

typedef XEvent PureCEvent;

@interface XWindowServer : Object
- (void)runWM;
- (void)handleMapRequest:(id)e display:(id)d;
- (void)handleConfigureRequest:(id)e display:(id)d;
@end

@implementation XWindowServer

- (void)runWM {
    Display *display = (Display *)XOpenDisplay(NULL);
    if (!display) {
        fprintf(stderr, "Failed to open display\n");
        return;
    }

    Window root = DefaultRootWindow(display);
    XSelectInput(display, root, SubstructureRedirectMask | SubstructureNotifyMask);
    XSync(display, False);

    PureCEvent ev;

    while (!XNextEvent(display, &ev)) {
        if (ev.type == MapRequest) {
            [self handleMapRequest:&ev.xmaprequest display:display];
        }
        else if (ev.type == ConfigureRequest) {
            [self handleConfigureRequest:&ev.xconfigurerequest display:display];
        }
    }

    XCloseDisplay(display);
}

- (void)handleMapRequest:(id)e display:(id)d {
    XMapRequestEvent *req = (XMapRequestEvent *)e;
    Display *display = (Display *)d;

    XMapWindow(display, req->window);
    XSetInputFocus(display, req->window, RevertToParent, CurrentTime);
}

- (void)handleConfigureRequest:(id)e display:(id)d {
    XConfigureRequestEvent *req = (XConfigureRequestEvent *)e;
    Display *display = (Display *)d;

    XWindowChanges changes;
    changes.x = req->x;
    changes.y = req->y;
    changes.width = req->width;
    changes.height = req->height;
    changes.border_width = req->border_width;
    changes.sibling = req->above;
    changes.stack_mode = req->detail;

    XConfigureWindow(display, req->window, req->value_mask, &changes);
}

@end

int main() {
    id server = [[XWindowServer alloc] init];
    [server runWM];
    return 0;
}
