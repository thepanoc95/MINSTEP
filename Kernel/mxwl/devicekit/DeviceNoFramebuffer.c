/*
 * mxwl/devicekit/DeviceNoFramebuffer.c
 *
 * No-Framebuffer Driver for MINSTEP.
 * Provides a null display driver for headless/server mode.
 *
 * Copyright (c) 2026 MinSTEP Project
 */

#include "Headers/DeviceKit.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

/* ========================================================================
 * No-Framebuffer Driver State
 * ======================================================================== */

typedef struct {
    DKitFramebufferDriver base;
    
    /* Display settings (virtual) */
    uint32_t width;
    uint32_t height;
    uint32_t bitsPerPixel;
    
    /* Memory framebuffer (offscreen) */
    uint32_t *framebuffer;
    size_t framebufferSize;
    
} NoFramebufferDriver;

/* ========================================================================
 * Driver Instance
 * ======================================================================== */

static NoFramebufferDriver *_null_driver = NULL;

/* ========================================================================
 * Framebuffer Operations
 * ======================================================================== */

static int nofb_initialize(NoFramebufferDriver *fb)
{
    if (!fb) return DKIT_ERROR_GENERAL;
    
    /* Set default resolution */
    fb->width = 640;
    fb->height = 480;
    fb->bitsPerPixel = 32;
    
    /* Allocate offscreen framebuffer */
    fb->framebufferSize = fb->width * fb->height * 4;
    fb->framebuffer = (uint32_t *)malloc(fb->framebufferSize);
    if (!fb->framebuffer) {
        fprintf(stderr, "NoFB: Cannot allocate framebuffer\n");
        return DKIT_ERROR_NO_MEMORY;
    }
    
    /* Clear to black */
    memset(fb->framebuffer, 0, fb->framebufferSize);
    
    printf("NoFB: Null framebuffer initialized (%dx%d, %d bpp)\n",
           fb->width, fb->height, fb->bitsPerPixel);
    
    return DKIT_SUCCESS;
}

static int nofb_shutdown(NoFramebufferDriver *fb)
{
    if (!fb) return DKIT_ERROR_GENERAL;
    
    if (fb->framebuffer) {
        free(fb->framebuffer);
        fb->framebuffer = NULL;
    }
    
    printf("NoFB: Null framebuffer shutdown\n");
    return DKIT_SUCCESS;
}

static int nofb_set_mode(NoFramebufferDriver *fb, uint32_t width, uint32_t height, uint32_t bpp)
{
    if (!fb) return DKIT_ERROR_GENERAL;
    
    /* Reallocate framebuffer if needed */
    size_t newSize = width * height * 4;
    if (newSize != fb->framebufferSize) {
        uint32_t *newFB = (uint32_t *)realloc(fb->framebuffer, newSize);
        if (!newFB) {
            return DKIT_ERROR_NO_MEMORY;
        }
        fb->framebuffer = newFB;
        fb->framebufferSize = newSize;
    }
    
    fb->width = width;
    fb->height = height;
    fb->bitsPerPixel = bpp;
    
    /* Clear to black */
    memset(fb->framebuffer, 0, fb->framebufferSize);
    
    printf("NoFB: Resolution set to %dx%d, %d bpp\n", width, height, bpp);
    
    return DKIT_SUCCESS;
}

static int nofb_set_display_mode(NoFramebufferDriver *fb, const DKitDisplayMode *mode)
{
    if (!mode) return DKIT_ERROR_GENERAL;
    return nofb_set_mode(fb, mode->width, mode->height, mode->bitsPerPixel);
}

static int nofb_get_display_modes(NoFramebufferDriver *fb, DKitDisplayMode **modes, uint32_t *count)
{
    if (!modes || !count) return DKIT_ERROR_GENERAL;
    
    /* Virtual modes (all work in software rendering) */
    static DKitDisplayMode virtual_modes[] = {
        { 640, 480, 32, 60, true, false },
        { 800, 600, 32, 60, false, false },
        { 1024, 768, 32, 60, false, false },
        { 1280, 720, 32, 60, false, false },
        { 1280, 800, 32, 60, false, false },
        { 1920, 1080, 32, 60, false, false },
    };
    
    *modes = virtual_modes;
    *count = sizeof(virtual_modes) / sizeof(virtual_modes[0]);
    
    return DKIT_SUCCESS;
}

/* ========================================================================
 * Drawing Operations
 * ======================================================================== */

static void nofb_fill_rect(NoFramebufferDriver *fb, DKitRect rect, uint32_t color)
{
    if (!fb || !fb->framebuffer) return;
    
    int x, y;
    for (y = rect.y; y < rect.y + rect.height && y < (int)fb->height; y++) {
        for (x = rect.x; x < rect.x + rect.width && x < (int)fb->width; x++) {
            if (y >= 0 && x >= 0) {
                fb->framebuffer[y * fb->width + x] = color;
            }
        }
    }
}

static void nofb_copy_rect(NoFramebufferDriver *fb, DKitRect srcRect, int dx, int dy)
{
    if (!fb || !fb->framebuffer) return;
    
    DKitRect dstRect = DKitMakeRect(srcRect.x + dx, srcRect.y + dy, 
                                    srcRect.width, srcRect.height);
    
    /* Simple copy (could be optimized with memcpy per line) */
    int x, y;
    for (y = 0; y < srcRect.height; y++) {
        int srcY = srcRect.y + y;
        int dstY = dstRect.y + y;
        if (srcY < 0 || srcY >= (int)fb->height) continue;
        if (dstY < 0 || dstY >= (int)fb->height) continue;
        
        for (x = 0; x < srcRect.width; x++) {
            int srcX = srcRect.x + x;
            int dstX = dstRect.x + x;
            if (srcX < 0 || srcX >= (int)fb->width) continue;
            if (dstX < 0 || dstX >= (int)fb->width) continue;
            
            fb->framebuffer[dstY * fb->width + dstX] = 
                fb->framebuffer[srcY * fb->width + srcX];
        }
    }
}

static void nofb_flush(NoFramebufferDriver *fb, int x, int y, int width, int height)
{
    /* No-op for offscreen rendering */
    (void)fb;
    (void)x;
    (void)y;
    (void)width;
    (void)height;
}

static void nofb_flush_rect(NoFramebufferDriver *fb, DKitRect rect)
{
    nofb_flush(fb, rect.x, rect.y, rect.width, rect.height);
}

/* ========================================================================
 * Color Conversion
 * ======================================================================== */

static uint32_t nofb_rgb_to_pixel(NoFramebufferDriver *fb, uint8_t r, uint8_t g, uint8_t b)
{
    (void)fb;
    return DKIT_RGB(r, g, b);
}

static void nofb_pixel_to_rgb(NoFramebufferDriver *fb, uint32_t pixel, uint8_t *r, uint8_t *g, uint8_t *b)
{
    (void)fb;
    *r = DKIT_GET_R(pixel);
    *g = DKIT_GET_G(pixel);
    *b = DKIT_GET_B(pixel);
}

/* ========================================================================
 * Cursor Operations (not supported)
 * ======================================================================== */

static int nofb_set_cursor(NoFramebufferDriver *fb, const void *cursorData, 
                           int width, int height, int hotX, int hotY)
{
    (void)fb;
    (void)cursorData;
    (void)width;
    (void)height;
    (void)hotX;
    (void)hotY;
    return DKIT_SUCCESS;
}

static int nofb_show_cursor(NoFramebufferDriver *fb, bool show)
{
    (void)fb;
    (void)show;
    return DKIT_SUCCESS;
}

static int nofb_move_cursor(NoFramebufferDriver *fb, int x, int y)
{
    (void)fb;
    (void)x;
    (void)y;
    return DKIT_SUCCESS;
}

/* ========================================================================
 * Synchronization
 * ======================================================================== */

static int nofb_wait_for_vsync(NoFramebufferDriver *fb)
{
    (void)fb;
    /* No VSync in headless mode */
    return DKIT_SUCCESS;
}

static int nofb_lock_buffer(NoFramebufferDriver *fb)
{
    (void)fb;
    return DKIT_SUCCESS;
}

static int nofb_unlock_buffer(NoFramebufferDriver *fb)
{
    (void)fb;
    return DKIT_SUCCESS;
}

/* ========================================================================
 * Driver Probe and Start
 * ======================================================================== */

static int nofb_probe(DKitDriverRef driver)
{
    (void)driver;
    /* Always available - this is the fallback driver */
    printf("NoFB: Null framebuffer available\n");
    return 1;
}

static int nofb_start(DKitDriverRef driver)
{
    NoFramebufferDriver *fb = (NoFramebufferDriver *)driver;
    return nofb_initialize(fb);
}

static int nofb_stop(DKitDriverRef driver)
{
    NoFramebufferDriver *fb = (NoFramebufferDriver *)driver;
    return nofb_shutdown(fb);
}

/* ========================================================================
 * Driver Entry Point
 * ======================================================================== */

DKitDriverRef DKitCreateDriver_NoFramebuffer(void)
{
    NoFramebufferDriver *fb = (NoFramebufferDriver *)DKitCreateDriver(
        "NoFramebuffer", DKIT_DRIVER_TYPE_FRAMEBUFFER);
    
    if (!fb) return NULL;
    
    /* Set up driver interface */
    fb->base.probe = (DKitDriverProbeFunc)nofb_probe;
    fb->base.start = (DKitDriverStartFunc)nofb_start;
    fb->base.stop = (DKitDriverStopFunc)nofb_stop;
    
    /* Framebuffer interface */
    fb->initialize = (void *)nofb_initialize;
    fb->setMode = (void *)nofb_set_mode;
    fb->getDisplayModes = (void *)nofb_get_display_modes;
    fb->setDisplayMode = (void *)nofb_set_display_mode;
    fb->flush = (void *)nofb_flush;
    fb->flushRect = (void *)nofb_flush_rect;
    fb->fillRect = (void *)nofb_fill_rect;
    fb->copyRect = (void *)nofb_copy_rect;
    fb->rgbToPixel = (void *)nofb_rgb_to_pixel;
    fb->pixelToRgb = (void *)nofb_pixel_to_rgb;
    fb->setCursor = (void *)nofb_set_cursor;
    fb->showCursor = (void *)nofb_show_cursor;
    fb->moveCursor = (void *)nofb_move_cursor;
    fb->waitForVSync = (void *)nofb_wait_for_vsync;
    fb->lockBuffer = (void *)nofb_lock_buffer;
    fb->unlockBuffer = (void *)nofb_unlock_buffer;
    fb->doubleBuffered = false;
    
    _null_driver = fb;
    
    return (DKitDriverRef)fb;
}

/* ========================================================================
 * Direct Access Functions
 * ======================================================================== */

uint32_t *NoFramebufferGetBuffer(void)
{
    return _null_driver ? _null_driver->framebuffer : NULL;
}

uint32_t NoFramebufferGetWidth(void)
{
    return _null_driver ? _null_driver->width : 0;
}

uint32_t NoFramebufferGetHeight(void)
{
    return _null_driver ? _null_driver->height : 0;
}