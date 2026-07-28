/*
 * mxwl/devicekit/DeviceX11Framebuffer.c
 *
 * X11 Framebuffer Driver for MINSTEP.
 * Provides display output via X11.
 *
 * Copyright (c) 2026 MinSTEP Project
 */

#include "Headers/DeviceKit.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

/* X11 includes (if available) */
#ifdef HAVE_X11
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#endif

/* ========================================================================
 * X11 Framebuffer Driver State
 * ======================================================================== */

typedef struct {
    DKitFramebufferDriver base;
    
    /* X11 state */
#ifdef HAVE_X11
    Display *display;
    int screen;
    Window window;
    Visual *visual;
    GC gc;
    XImage *ximage;
#endif
    
    /* Display settings */
    uint32_t width;
    uint32_t height;
    uint32_t bitsPerPixel;
    bool fullscreen;
    bool hasFocus;
    
    /* Double buffering */
    uint32_t *backBuffer;
    size_t backBufferSize;
    
    /* Cursor state */
    int cursorX;
    int cursorY;
    bool cursorVisible;
    
} X11FramebufferDriver;

/* ========================================================================
 * Driver Instance
 * ======================================================================== */

static X11FramebufferDriver *_x11_driver = NULL;

/* ========================================================================
 * Color Conversion
 * ======================================================================== */

static uint32_t x11_rgb_to_pixel(X11FramebufferDriver *fb, uint8_t r, uint8_t g, uint8_t b)
{
    return DKIT_RGB(r, g, b);
}

static void x11_pixel_to_rgb(X11FramebufferDriver *fb, uint32_t pixel, uint8_t *r, uint8_t *g, uint8_t *b)
{
    *r = DKIT_GET_R(pixel);
    *g = DKIT_GET_G(pixel);
    *b = DKIT_GET_B(pixel);
}

/* ========================================================================
 * Drawing Operations
 * ======================================================================== */

static void x11_fill_rect(X11FramebufferDriver *fb, DKitRect rect, uint32_t color)
{
    uint8_t r, g, b;
    x11_pixel_to_rgb(fb, color, &r, &g, &b);
    
#ifdef HAVE_X11
    if (fb->display && fb->gc) {
        XSetForeground(fb->display, fb->gc, 
                      (r << 16) | (g << 8) | b);
        XFillRectangle(fb->display, fb->window, fb->gc,
                       rect.x, rect.y, rect.width, rect.height);
    }
#endif
}

static void x11_copy_rect(X11FramebufferDriver *fb, DKitRect srcRect, int dx, int dy)
{
#ifdef HAVE_X11
    if (fb->display && fb->gc) {
        XCopyArea(fb->display, fb->window, fb->window, fb->gc,
                  srcRect.x, srcRect.y, srcRect.width, srcRect.height,
                  srcRect.x + dx, srcRect.y + dy);
    }
#endif
}

static void x11_flush(X11FramebufferDriver *fb, int x, int y, int width, int height)
{
#ifdef HAVE_X11
    if (fb->display) {
        XFlush(fb->display);
    }
#endif
}

static void x11_flush_rect(X11FramebufferDriver *fb, DKitRect rect)
{
    x11_flush(fb, rect.x, rect.y, rect.width, rect.height);
}

static int x11_initialize(X11FramebufferDriver *fb)
{
    if (!fb) return DKIT_ERROR_GENERAL;
    
#ifdef HAVE_X11
    /* Open X11 display */
    const char *display_name = getenv("DISPLAY");
    if (!display_name) display_name = ":0";
    
    fb->display = XOpenDisplay(display_name);
    if (!fb->display) {
        fprintf(stderr, "X11FB: Cannot open display '%s'\n", display_name);
        return DKIT_ERROR_GENERAL;
    }
    
    fb->screen = DefaultScreen(fb->display);
    fb->visual = DefaultVisual(fb->display, fb->screen);
    
    /* Get default depth */
    int depth = DefaultDepth(fb->display, fb->screen);
    
    /* Set initial resolution */
    fb->width = 800;
    fb->height = 600;
    fb->bitsPerPixel = (depth >= 24) ? 32 : 16;
    
    /* Create window */
    fb->window = XCreateSimpleWindow(
        fb->display,
        RootWindow(fb->display, fb->screen),
        0, 0, fb->width, fb->height,
        1,
        BlackPixel(fb->display, fb->screen),
        BlackPixel(fb->display, fb->screen)
    );
    
    /* Set window title */
    XStoreName(fb->display, fb->window, "MINSTEP");
    
    /* Create graphics context */
    fb->gc = XCreateGC(fb->display, fb->window, 0, NULL);
    
    /* Select input events */
    XSelectInput(fb->display, fb->window,
                 KeyPressMask | KeyReleaseMask |
                 ButtonPressMask | ButtonReleaseMask |
                 EnterWindowMask | LeaveWindowMask |
                 ExposureMask | StructureNotifyMask);
    
    /* Map window */
    XMapWindow(fb->display, fb->window);
    XFlush(fb->display);
    
    /* Allocate back buffer */
    fb->backBufferSize = fb->width * fb->height * 4;
    fb->backBuffer = (uint32_t *)malloc(fb->backBufferSize);
    if (!fb->backBuffer) {
        fprintf(stderr, "X11FB: Cannot allocate back buffer\n");
        return DKIT_ERROR_NO_MEMORY;
    }
    memset(fb->backBuffer, 0, fb->backBufferSize);
    
    /* Create XImage for back buffer */
    fb->ximage = XCreateImage(
        fb->display, fb->visual, depth, ZPixmap, 0,
        (char *)fb->backBuffer,
        fb->width, fb->height, 32, fb->width * 4
    );
    
    printf("X11FB: Display opened (%dx%d, %d bpp)\n",
           fb->width, fb->height, fb->bitsPerPixel);
    
#else
    fprintf(stderr, "X11FB: X11 support not compiled in\n");
    return DKIT_ERROR_GENERAL;
#endif
    
    return DKIT_SUCCESS;
}

static int x11_shutdown(X11FramebufferDriver *fb)
{
    if (!fb) return DKIT_ERROR_GENERAL;
    
#ifdef HAVE_X11
    if (fb->ximage) {
        XDestroyImage(fb->ximage);
        fb->ximage = NULL;
    }
    
    if (fb->backBuffer) {
        free(fb->backBuffer);
        fb->backBuffer = NULL;
    }
    
    if (fb->gc) {
        XFreeGC(fb->display, fb->gc);
        fb->gc = NULL;
    }
    
    if (fb->window) {
        XDestroyWindow(fb->display, fb->window);
        fb->window = 0;
    }
    
    if (fb->display) {
        XCloseDisplay(fb->display);
        fb->display = NULL;
    }
#endif
    
    printf("X11FB: Display closed\n");
    return DKIT_SUCCESS;
}

static int x11_set_mode(X11FramebufferDriver *fb, uint32_t width, uint32_t height, uint32_t bpp)
{
    fb->width = width;
    fb->height = height;
    fb->bitsPerPixel = bpp;
    
#ifdef HAVE_X11
    if (fb->display && fb->window) {
        XResizeWindow(fb->display, fb->window, width, height);
        
        /* Reallocate back buffer */
        size_t newSize = width * height * 4;
        fb->backBuffer = (uint32_t *)realloc(fb->backBuffer, newSize);
        if (fb->backBuffer) {
            fb->backBufferSize = newSize;
            memset(fb->backBuffer, 0, newSize);
            
            if (fb->ximage) {
                XDestroyImage(fb->ximage);
            }
            int depth = DefaultDepth(fb->display, fb->screen);
            fb->ximage = XCreateImage(
                fb->display, fb->visual, depth, ZPixmap, 0,
                (char *)fb->backBuffer,
                width, height, 32, width * 4
            );
        }
        
        XFlush(fb->display);
    }
#endif
    
    return DKIT_SUCCESS;
}

static int x11_set_display_mode(X11FramebufferDriver *fb, const DKitDisplayMode *mode)
{
    if (!mode) return DKIT_ERROR_GENERAL;
    return x11_set_mode(fb, mode->width, mode->height, mode->bitsPerPixel);
}

static int x11_get_display_modes(X11FramebufferDriver *fb, DKitDisplayMode **modes, uint32_t *count)
{
    if (!modes || !count) return DKIT_ERROR_GENERAL;
    
    /* Return standard display modes */
    static DKitDisplayMode standard_modes[] = {
        { 640, 480, 32, 60, true, false },
        { 800, 600, 32, 60, false, false },
        { 1024, 768, 32, 60, false, false },
        { 1280, 720, 32, 60, false, false },
        { 1280, 800, 32, 60, false, false },
        { 1920, 1080, 32, 60, false, false },
    };
    
    *modes = standard_modes;
    *count = sizeof(standard_modes) / sizeof(standard_modes[0]);
    
    return DKIT_SUCCESS;
}

/* ========================================================================
 * Cursor Operations
 * ======================================================================== */

static int x11_set_cursor(X11FramebufferDriver *fb, const void *cursorData, 
                          int width, int height, int hotX, int hotY)
{
    /* Cursor setting would be implemented here */
    return DKIT_SUCCESS;
}

static int x11_show_cursor(X11FramebufferDriver *fb, bool show)
{
    fb->cursorVisible = show;
    return DKIT_SUCCESS;
}

static int x11_move_cursor(X11FramebufferDriver *fb, int x, int y)
{
    fb->cursorX = x;
    fb->cursorY = y;
    
#ifdef HAVE_X11
    if (fb->display && fb->window) {
        XWarpPointer(fb->display, None, fb->window, 0, 0, 0, 0, x, y);
        XFlush(fb->display);
    }
#endif
    
    return DKIT_SUCCESS;
}

/* ========================================================================
 * Synchronization
 * ======================================================================== */

static int x11_wait_for_vsync(X11FramebufferDriver *fb)
{
    /* Simple VSync using usleep - would be better with DRM/KMS */
    usleep(16667); /* ~60fps */
    return DKIT_SUCCESS;
}

static int x11_lock_buffer(X11FramebufferDriver *fb)
{
    /* Double-buffering: lock the back buffer */
    return DKIT_SUCCESS;
}

static int x11_unlock_buffer(X11FramebufferDriver *fb)
{
    /* Flip buffers or copy to screen */
#ifdef HAVE_X11
    if (fb->display && fb->ximage) {
        GC gc = XCreateGC(fb->display, fb->window, 0, NULL);
        XPutImage(fb->display, fb->window, gc, fb->ximage,
                  0, 0, 0, 0, fb->width, fb->height);
        XFreeGC(fb->display, gc);
        XFlush(fb->display);
    }
#endif
    return DKIT_SUCCESS;
}

/* ========================================================================
 * Driver Probe and Start
 * ======================================================================== */

static int x11_probe(DKitDriverRef driver)
{
    /* Check if X11 is available */
#ifdef HAVE_X11
    const char *display = getenv("DISPLAY");
    if (!display) {
        /* Try to open display anyway */
        Display *test = XOpenDisplay(NULL);
        if (!test) {
            printf("X11FB: No X11 display available\n");
            return -1;
        }
        XCloseDisplay(test);
    }
    printf("X11FB: X11 display detected\n");
    return 100; /* High priority if X11 available */
#else
    printf("X11FB: X11 support not compiled in\n");
    return -1;
#endif
}

static int x11_start(DKitDriverRef driver)
{
    X11FramebufferDriver *fb = (X11FramebufferDriver *)driver;
    return x11_initialize(fb);
}

static int x11_stop(DKitDriverRef driver)
{
    X11FramebufferDriver *fb = (X11FramebufferDriver *)driver;
    return x11_shutdown(fb);
}

/* ========================================================================
 * Driver Entry Point
 * ======================================================================== */

DKitDriverRef DKitCreateDriver_X11Framebuffer(void)
{
    X11FramebufferDriver *fb = (X11FramebufferDriver *)DKitCreateDriver(
        "X11Framebuffer", DKIT_DRIVER_TYPE_FRAMEBUFFER);
    
    if (!fb) return NULL;
    
    /* Set up driver interface */
    fb->base.probe = (DKitDriverProbeFunc)x11_probe;
    fb->base.start = (DKitDriverStartFunc)x11_start;
    fb->base.stop = (DKitDriverStopFunc)x11_stop;
    
    /* Framebuffer interface */
    fb->initialize = (void *)x11_initialize;
    fb->setMode = (void *)x11_set_mode;
    fb->getDisplayModes = (void *)x11_get_display_modes;
    fb->setDisplayMode = (void *)x11_set_display_mode;
    fb->flush = (void *)x11_flush;
    fb->flushRect = (void *)x11_flush_rect;
    fb->fillRect = (void *)x11_fill_rect;
    fb->copyRect = (void *)x11_copy_rect;
    fb->rgbToPixel = (void *)x11_rgb_to_pixel;
    fb->pixelToRgb = (void *)x11_pixel_to_rgb;
    fb->setCursor = (void *)x11_set_cursor;
    fb->showCursor = (void *)x11_show_cursor;
    fb->moveCursor = (void *)x11_move_cursor;
    fb->waitForVSync = (void *)x11_wait_for_vsync;
    fb->lockBuffer = (void *)x11_lock_buffer;
    fb->unlockBuffer = (void *)x11_unlock_buffer;
    fb->doubleBuffered = true;
    
    _x11_driver = fb;
    
    return (DKitDriverRef)fb;
}

/* ========================================================================
 * Direct Access Functions
 * ======================================================================== */

#ifdef HAVE_X11
Display *X11FramebufferGetDisplay(void)
{
    return _x11_driver ? _x11_driver->display : NULL;
}

Window X11FramebufferGetWindow(void)
{
    return _x11_driver ? _x11_driver->window : 0;
}

GC X11FramebufferGetGC(void)
{
    return _x11_driver ? _x11_driver->gc : NULL;
}
#endif

uint32_t *X11FramebufferGetBackBuffer(void)
{
    return _x11_driver ? _x11_driver->backBuffer : NULL;
}

void X11FramebufferFlip(X11FramebufferDriver *fb)
{
    if (!fb) return;
    x11_unlock_buffer(fb);
}