/*
 * mxwl/devicekit/Headers/DeviceKit.h
 *
 * DriverKit-like framework for mxwl kernel.
 * Inspired by NeXTSTEP DriverKit and Apple IOKit.
 *
 * Provides:
 * - Driver class hierarchy
 * - Framebuffer drivers (X11, no-display)
 * - Platform family detection
 * - Driver registration and matching
 *
 * Copyright (c) 2026 MinSTEP Project
 */

#ifndef MXWL_DEVICEKIT_H
#define MXWL_DEVICEKIT_H

#include <sys/types.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __OBJC__
#include <objc/Object.h>
#endif

/* ========================================================================
 * DriverKit Version
 * ======================================================================== */

#define DKIT_VERSION_MAJOR 1
#define DKIT_VERSION_MINOR 0
#define DKIT_VERSION_STRING "1.0.0"

/* ========================================================================
 * Driver IOKit-like Constants
 * ======================================================================== */

/* Driver types */
#define DKIT_DRIVER_TYPE_FRAMEBUFFER  0x0001
#define DKIT_DRIVER_TYPE_NETWORK     0x0002
#define DKIT_DRIVER_TYPE_STORAGE     0x0003
#define DKIT_DRIVER_TYPE_INPUT       0x0004
#define DKIT_DRIVER_TYPE_AUDIO       0x0005
#define DKIT_DRIVER_TYPE_SERIAL      0x0006
#define DKIT_DRIVER_TYPE_PARALLEL    0x0007
#define DKIT_DRIVER_TYPE_GRAPHICS    0x0008
#define DKIT_DRIVER_TYPE_POWER       0x0009

/* Framebuffer pixel formats */
#define DKIT_PIXEL_FORMAT_GS        0x0001  /* Grayscale */
#define DKIT_PIXEL_FORMAT_RGB       0x0002  /* RGB */
#define DKIT_PIXEL_FORMAT_RGBA      0x0003  /* RGBA */
#define DKIT_PIXEL_FORMAT_INDEX8    0x0004  /* 8-bit indexed */
#define DKIT_PIXEL_FORMAT_INDEX16   0x0005  /* 16-bit indexed */
#define DKIT_PIXEL_FORMAT_BGR       0x0006  /* BGR */

/* Framebuffer capabilities */
#define DKIT_CAPABILITY_DOUBLEBUFFER    0x0001
#define DKIT_CAPABILITY_HARDWARE_CURSOR 0x0002
#define DKIT_CAPABILITY_ACCELERATED      0x0004
#define DKIT_CAPABILITY_FULLSCREEN       0x0008
#define DKIT_CAPABILITY_RESIZABLE        0x0010
#define DKIT_CAPABILITY_COLOR_CURSOR     0x0020

/* Driver states */
#define DKIT_STATE_NOT_LOADED      0
#define DKIT_STATE_LOADING         1
#define DKIT_STATE_LOADED          2
#define DKIT_STATE_INITIALIZING    3
#define DKIT_STATE_READY           4
#define DKIT_STATE_SUSPENDED       5
#define DKIT_STATE_ERROR           6
#define DKIT_STATE_UNLOADING       7

/* Error codes */
#define DKIT_SUCCESS               0
#define DKIT_ERROR_GENERAL        (-1)
#define DKIT_ERROR_NO_MEMORY      (-2)
#define DKIT_ERROR_NOT_FOUND      (-3)
#define DKIT_ERROR_ALREADY_EXISTS  (-4)
#define DKIT_ERROR_INVALID_STATE  (-5)
#define DKIT_ERROR_TIMEOUT         (-6)
#define DKIT_ERROR_PERMISSION      (-7)

/* ========================================================================
 * Framebuffer Device Structure
 * ======================================================================== */

typedef struct {
    uint32_t width;
    uint32_t height;
    uint32_t pitch;
    uint32_t bitsPerPixel;
    uint32_t pixelFormat;
    uint32_t redMask;
    uint32_t greenMask;
    uint32_t blueMask;
    uint32_t alphaMask;
    void *framebuffer;
    size_t framebufferSize;
} DKitFramebufferInfo;

/* ========================================================================
 * Display Mode Structure
 * ======================================================================== */

typedef struct {
    uint32_t width;
    uint32_t height;
    uint32_t bitsPerPixel;
    uint32_t refreshRate;
    bool isPreferred;
    bool isInterlaced;
} DKitDisplayMode;

/* ========================================================================
 * Rectangle Type (for drawing)
 * ======================================================================== */

typedef struct {
    int x;
    int y;
    int width;
    int height;
} DKitRect;

typedef struct {
    int x;
    int y;
} DKitPoint;

/* ========================================================================
 * Driver Registry Entry
 * ======================================================================== */

typedef struct {
    const char *name;
    const char *className;
    const char *bundleIdentifier;
    uint32_t driverType;
    uint32_t priority;
    void *driverInstance;
    void *probeResult;
} DKitDriverEntry;

/* ========================================================================
 * Driver Base Class (pure C for kernel compatibility)
 * ======================================================================== */

struct _DKitDriver_class_t;

typedef struct DKitDriver {
    struct _DKitDriver_class_t *isa;
    
    /* Instance variables */
    const char *name;
    const char *bundlePath;
    uint32_t state;
    uint32_t driverType;
    void *platform;
    void *userData;
    
    /* Framebuffer interface */
    int (*probe)(struct DKitDriver *self);
    int (*start)(struct DKitDriver *self);
    int (*stop)(struct DKitDriver *self);
    int (*suspend)(struct DKitDriver *self);
    int (*resume)(struct DKitDriver *self);
} DKitDriver;

typedef DKitDriver *DKitDriverRef;

/* Driver methods (C function pointers) */
typedef int (*DKitDriverProbeFunc)(DKitDriverRef);
typedef int (*DKitDriverStartFunc)(DKitDriverRef);
typedef int (*DKitDriverStopFunc)(DKitDriverRef);

/* ========================================================================
 * Framebuffer Driver Interface
 * ======================================================================== */

typedef struct DKitFramebufferDriver {
    DKitDriver base;
    
    /* Framebuffer info */
    DKitFramebufferInfo framebufferInfo;
    
    /* Framebuffer operations */
    int (*initialize)(DKitDriverRef self);
    int (*setMode)(DKitDriverRef self, uint32_t width, uint32_t height, uint32_t bpp);
    int (*getDisplayModes)(DKitDriverRef self, DKitDisplayMode **modes, uint32_t *count);
    int (*setDisplayMode)(DKitDriverRef self, const DKitDisplayMode *mode);
    
    /* Drawing operations */
    void (*flush)(DKitDriverRef self, int x, int y, int width, int height);
    void (*flushRect)(DKitDriverRef self, DKitRect rect);
    void (*fillRect)(DKitDriverRef self, DKitRect rect, uint32_t color);
    void (*copyRect)(DKitDriverRef self, DKitRect srcRect, int dx, int dy);
    
    /* Color operations */
    uint32_t (*rgbToPixel)(DKitDriverRef self, uint8_t r, uint8_t g, uint8_t b);
    void (*pixelToRgb)(DKitDriverRef self, uint32_t pixel, uint8_t *r, uint8_t *g, uint8_t *b);
    
    /* Cursor operations */
    int (*setCursor)(DKitDriverRef self, const void *cursorData, int width, int height, int hotX, int hotY);
    int (*showCursor)(DKitDriverRef self, bool show);
    int (*moveCursor)(DKitDriverRef self, int x, int y);
    
    /* Palette operations (for indexed modes) */
    int (*setPalette)(DKitDriverRef self, const uint32_t *colors, uint32_t first, uint32_t count);
    
    /* Synchronization */
    int (*waitForVSync)(DKitDriverRef self);
    int (*lockBuffer)(DKitDriverRef self);
    int (*unlockBuffer)(DKitDriverRef self);
    bool doubleBuffered;
    
} DKitFramebufferDriver;

typedef DKitFramebufferDriver *DKitFramebufferDriverRef;

/* ========================================================================
 * Color Utilities
 * ======================================================================== */

#define DKIT_RGB(r, g, b) ((uint32_t)((((r) & 0xFF) << 16) | (((g) & 0xFF) << 8) | ((b) & 0xFF)))
#define DKIT_RGBA(r, g, b, a) ((uint32_t)((((r) & 0xFF) << 24) | (((g) & 0xFF) << 16) | (((b) & 0xFF) << 8) | ((a) & 0xFF)))

#define DKIT_GET_R(color) (((color) >> 16) & 0xFF)
#define DKIT_GET_G(color) (((color) >> 8) & 0xFF)
#define DKIT_GET_B(color) ((color) & 0xFF)
#define DKIT_GET_A(color) (((color) >> 24) & 0xFF)

/* ========================================================================
 * Driver Manager (C API)
 * ======================================================================== */

int DKitInitialize(void);
int DKitShutdown(void);

DKitDriverRef DKitCreateDriver(const char *className, uint32_t driverType);
int DKitDestroyDriver(DKitDriverRef driver);
int DKitRegisterDriver(DKitDriverRef driver);
int DKitUnregisterDriver(DKitDriverRef driver);
DKitDriverRef DKitFindDriver(const char *name);
DKitDriverRef DKitFindDriverByType(uint32_t driverType);

DKitDriverRef DKitGetFramebufferDriver(void);
int DKitSetFramebufferDriver(DKitDriverRef driver);

/* Driver lifecycle */
int DKitProbeDrivers(void);
int DKitStartDrivers(void);
int DKitStopDrivers(void);

/* ========================================================================
 * Platform Family Detection
 * ======================================================================== */

const char *DKitGetPlatformFamily(void);
uint32_t DKitGetPlatformCPUType(void);
bool DKitPlatformHasFeature(const char *feature);

/* ========================================================================
 * Driver Registration Macros
 * ======================================================================== */

/* Declare a driver entry point */
#define DKIT_DRIVER_ENTRY(className) \
    DKitDriverRef DKitCreateDriver_##className(void) __attribute__((visibility("default")))

/* Register a framebuffer driver */
#define DKIT_REGISTER_FRAMEBUFFER_DRIVER(className, probeFunc, startFunc) \
    DKIT_DRIVER_ENTRY(className) { \
        DKitFramebufferDriver *drv = (DKitFramebufferDriver *)DKitCreateDriver(#className, DKIT_DRIVER_TYPE_FRAMEBUFFER); \
        if (drv) { \
            drv->probe = (DKitDriverProbeFunc)probeFunc; \
            drv->start = (DKitDriverStartFunc)startFunc; \
        } \
        return (DKitDriverRef)drv; \
    }

/* ========================================================================
 * Framebuffer Driver Utilities
 * ======================================================================== */

/* Calculate framebuffer pitch */
static inline uint32_t DKitCalculatePitch(uint32_t width, uint32_t bitsPerPixel) {
    return ((width * bitsPerPixel + 31) / 32) * 4;
}

/* Calculate framebuffer size */
static inline size_t DKitCalculateFramebufferSize(uint32_t width, uint32_t height, uint32_t bitsPerPixel) {
    return (size_t)height * DKitCalculatePitch(width, bitsPerPixel);
}

/* Create a DKitRect */
static inline DKitRect DKitMakeRect(int x, int y, int w, int h) {
    DKitRect r; r.x = x; r.y = y; r.width = w; r.height = h; return r;
}

/* Create a DKitPoint */
static inline DKitPoint DKitMakePoint(int x, int y) {
    DKitPoint p; p.x = x; p.y = y; return p;
}

/* Check if point is in rect */
static inline bool DKitPointInRect(DKitPoint p, DKitRect r) {
    return (p.x >= r.x && p.x < r.x + r.width && p.y >= r.y && p.y < r.y + r.height);
}

/* Check if rects intersect */
static inline bool DKitRectsIntersect(DKitRect a, DKitRect b) {
    return !(a.x + a.width <= b.x || b.x + b.width <= a.x ||
             a.y + a.height <= b.y || b.y + b.height <= a.y);
}

/* Intersection of two rects */
static inline DKitRect DKitRectIntersection(DKitRect a, DKitRect b) {
    DKitRect r;
    r.x = (a.x > b.x) ? a.x : b.x;
    r.y = (a.y > b.y) ? a.y : b.y;
    r.width = ((a.x + a.width < b.x + b.width) ? a.x + a.width : b.x + b.width) - r.x;
    r.height = ((a.y + a.height < b.y + b.height) ? a.y + a.height : b.y + b.height) - r.y;
    if (r.width < 0) r.width = 0;
    if (r.height < 0) r.height = 0;
    return r;
}

#endif /* MXWL_DEVICEKIT_H */