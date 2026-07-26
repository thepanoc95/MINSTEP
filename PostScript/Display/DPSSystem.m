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

/* #objc: MINSTEP Objective-C translation unit */
#include "DPSSystem.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DPS_MAX_WINDOWS 32
#define DPS_EVENT_QUEUE 64

struct DPSWindow {
    int id;
    int x;
    int y;
    int width;
    int height;
    int mapped;
    int z;
    char title[96];
    DPSContext *ctx;
};

struct DPSDisplay {
    int width;
    int height;
    int next_window_id;
    int next_z;
    DPSDeviceKind kind;
    DPSContext *root;
    DPSWindow *windows[DPS_MAX_WINDOWS];
    int window_count;
    DPSEvent events[DPS_EVENT_QUEUE];
    int event_head;
    int event_tail;
    char error[160];
};

static void system_error(DPSDisplay *display, const char *message) {
    if (!display) return;
    strncpy(display->error, message, sizeof(display->error) - 1);
    display->error[sizeof(display->error) - 1] = '\0';
}

static unsigned char pixel_for(DPSContext *ctx, int x, int y) {
    unsigned char *pixels = DPSPixels(ctx);
    if (!ctx || !pixels || x < 0 || y < 0 || x >= DPSWidth(ctx) || y >= DPSHeight(ctx)) return 0;
    return pixels[(size_t)y * (size_t)DPSWidth(ctx) + (size_t)x];
}

static void put_pixel(DPSContext *ctx, int x, int y, unsigned char value) {
    unsigned char *pixels = DPSPixels(ctx);
    if (!ctx || !pixels || x < 0 || y < 0 || x >= DPSWidth(ctx) || y >= DPSHeight(ctx)) return;
    pixels[(size_t)y * (size_t)DPSWidth(ctx) + (size_t)x] = value;
}

static void draw_window_frame(DPSDisplay *display, DPSWindow *window) {
    int x;
    int y;
    for (x = 0; x < window->width + 2; x++) {
        put_pixel(display->root, window->x + x, window->y, 220);
        put_pixel(display->root, window->x + x, window->y + window->height + 1, 220);
    }
    for (y = 0; y < window->height + 2; y++) {
        put_pixel(display->root, window->x, window->y + y, 220);
        put_pixel(display->root, window->x + window->width + 1, window->y + y, 220);
    }
}

static void composite_window(DPSDisplay *display, DPSWindow *window) {
    int x;
    int y;
    if (!window->mapped) return;
    draw_window_frame(display, window);
    for (y = 0; y < window->height; y++) {
        for (x = 0; x < window->width; x++) {
            unsigned char p = pixel_for(window->ctx, x, y);
            if (p) put_pixel(display->root, window->x + x + 1, window->y + y + 1, p);
        }
    }
}

DPSDisplay *DPSDisplayCreate(int width, int height, DPSDeviceKind kind) {
    DPSDisplay *display;
    if (width <= 0) width = 80;
    if (height <= 0) height = 25;
    display = (DPSDisplay *)calloc(1, sizeof(DPSDisplay));
    if (!display) return NULL;
    display->root = DPSCreateContext(width, height, kind);
    if (!display->root) { free(display); return NULL; }
    display->width = width;
    display->height = height;
    display->kind = kind;
    display->next_window_id = 1;
    return display;
}

void DPSDisplayDestroy(DPSDisplay *display) {
    int i;
    if (!display) return;
    for (i = 0; i < display->window_count; i++) {
        DPSDestroyContext(display->windows[i]->ctx);
        free(display->windows[i]);
    }
    DPSDestroyContext(display->root);
    free(display);
}

DPSWindow *DPSWindowCreate(DPSDisplay *display, int x, int y, int width, int height, const char *title) {
    DPSWindow *window;
    DPSEvent event;
    if (!display || display->window_count >= DPS_MAX_WINDOWS) return NULL;
    window = (DPSWindow *)calloc(1, sizeof(DPSWindow));
    if (!window) return NULL;
    window->ctx = DPSCreateContext(width, height, DPS_DEVICE_MEMORY);
    if (!window->ctx) { free(window); return NULL; }
    window->id = display->next_window_id++;
    window->x = x;
    window->y = y;
    window->width = width;
    window->height = height;
    window->mapped = 1;
    window->z = display->next_z++;
    DPSWindowSetTitle(window, title ? title : "Untitled");
    display->windows[display->window_count++] = window;
    event.type = DPS_EVENT_EXPOSE;
    event.window_id = window->id;
    event.x = event.y = event.button = event.key = 0;
    DPSDisplayPostEvent(display, event);
    return window;
}

int DPSWindowID(DPSWindow *window) { return window ? window->id : 0; }
DPSContext *DPSWindowContext(DPSWindow *window) { return window ? window->ctx : NULL; }

int DPSWindowOrderFront(DPSWindow *window) {
    if (!window) return 0;
    window->z++;
    window->mapped = 1;
    return 1;
}

int DPSWindowMove(DPSWindow *window, int x, int y) {
    if (!window) return 0;
    window->x = x;
    window->y = y;
    return 1;
}

int DPSWindowSetTitle(DPSWindow *window, const char *title) {
    if (!window) return 0;
    strncpy(window->title, title ? title : "", sizeof(window->title) - 1);
    window->title[sizeof(window->title) - 1] = '\0';
    return 1;
}

int DPSDisplayPostEvent(DPSDisplay *display, DPSEvent event) {
    int next;
    if (!display) return 0;
    next = (display->event_tail + 1) % DPS_EVENT_QUEUE;
    if (next == display->event_head) { system_error(display, "event queue full"); return 0; }
    display->events[display->event_tail] = event;
    display->event_tail = next;
    return 1;
}

int DPSDisplayNextEvent(DPSDisplay *display, DPSEvent *event) {
    if (!display || !event) return 0;
    if (display->event_head == display->event_tail) return 0;
    *event = display->events[display->event_head];
    display->event_head = (display->event_head + 1) % DPS_EVENT_QUEUE;
    return 1;
}

int DPSDisplayComposite(DPSDisplay *display) {
    int z;
    int i;
    int max_z = -1;
    if (!display) return 0;
    DPSClear(display->root, 0);
    for (i = 0; i < display->window_count; i++) if (display->windows[i]->z > max_z) max_z = display->windows[i]->z;
    for (z = 0; z <= max_z; z++) {
        for (i = 0; i < display->window_count; i++) if (display->windows[i]->z == z) composite_window(display, display->windows[i]);
    }
    return 1;
}

int DPSDisplayFlush(DPSDisplay *display) {
    if (!display) return 0;
    if (!DPSDisplayComposite(display)) return 0;
    return DPSFlush(display->root);
}

const char *DPSDisplayError(DPSDisplay *display) {
    return display && display->error[0] ? display->error : "noerror";
}
