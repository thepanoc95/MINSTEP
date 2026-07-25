#objc
#include "Headers/DeviceKit.h"
#include <appkit/appkit.h>

@interface NXFrameBuffer : Object
{
    DKitFramebufferDriverRef driver;
    NXRect screenRect;
    int depth;
    BOOL isActive;
}

+ (NXFrameBuffer *)sharedFrameBuffer;
+ (BOOL)initializeFrameBuffer;
+ (void)shutdown;

- (BOOL)isActive;
- (NXRect)bounds;
- (int)depth;
- (void *)framebufferAddress;

- (void)setPixel:(int)x y:(int)y color:(uint32_t)color;
- (uint32_t)getPixel:(int)x y:(int)y;
- (void)fillRect:(NXRect)rect color:(uint32_t)color;
- (void)copyRect:(NXRect)src toPoint:(int)dx :(int)dy;
- (void)flush;
- (void)flushRect:(NXRect)rect;

- (BOOL)supportsMode:(int)width height:(int)height depth:(int)depth;
- (BOOL)setMode:(int)width height:(int)height depth:(int)depth;

@end

static NXFrameBuffer *_sharedFrameBuffer = nil;

@implementation NXFrameBuffer

+ (NXFrameBuffer *)sharedFrameBuffer
{
    if (!_sharedFrameBuffer) {
        _sharedFrameBuffer = [self new];
    }
    return _sharedFrameBuffer;
}

+ (BOOL)initializeFrameBuffer
{
    NXFrameBuffer *fb = [self sharedFrameBuffer];
    
    DKitInitialize();
    
    DKitDriverRef x11Driver = DKitCreateDriver_X11Framebuffer();
    if (x11Driver) {
        DKitRegisterDriver(x11Driver);
    }
    
    DKitDriverRef nullDriver = DKitCreateDriver_NoFramebuffer();
    if (nullDriver) {
        DKitRegisterDriver(nullDriver);
    }
    
    /* Probe for best driver */
    int probed = DKitProbeDrivers();
    if (probed <= 0) {
        /* No X11, use null driver */
        DKitSetFramebufferDriver(nullDriver);
    } else {
        /* Use X11 if available, otherwise null */
        DKitDriverRef fbDriver = DKitGetFramebufferDriver();
        if (!fbDriver) {
            fbDriver = nullDriver;
        }
        DKitSetFramebufferDriver(fbDriver);
    }
    
    /* Start the framebuffer driver */
    int started = DKitStartDrivers();
    if (started <= 0) {
        return NO;
    }
    
    DKitFramebufferDriverRef driver = (DKitFramebufferDriverRef)DKitGetFramebufferDriver();
    if (driver) {
        fb->driver = driver;
        fb->isActive = YES;
        return YES;
    }
    
    return NO;
}

+ (void)shutdown
{
    DKitStopDrivers();
    DKitShutdown();
    
    if (_sharedFrameBuffer) {
        [_sharedFrameBuffer free];
        _sharedFrameBuffer = nil;
    }
}

- (id)init
{
    self = [super init];
    if (self) {
        driver = NULL;
        screenRect.origin_x = 0;
        screenRect.origin_y = 0;
        screenRect.size_width = 640;
        screenRect.size_height = 480;
        depth = 32;
        isActive = NO;
    }
    return self;
}

- (BOOL)isActive
{
    return isActive;
}

- (NXRect)bounds
{
    return screenRect;
}

- (int)depth
{
    return depth;
}

- (void *)framebufferAddress
{
    if (!driver) return NULL;
    return driver->framebufferInfo.framebuffer;
}

- (void)setPixel:(int)x y:(int)y color:(uint32_t)color
{
    if (!driver || !driver->framebufferInfo.framebuffer) return;
    
    uint32_t *fb = (uint32_t *)driver->framebufferInfo.framebuffer;
    int width = driver->framebufferInfo.width;
    
    if (x >= 0 && x < (int)width && y >= 0 && y < (int)driver->framebufferInfo.height) {
        fb[y * width + x] = color;
    }
}

- (uint32_t)getPixel:(int)x y:(int)y
{
    if (!driver || !driver->framebufferInfo.framebuffer) return 0;
    
    uint32_t *fb = (uint32_t *)driver->framebufferInfo.framebuffer;
    int width = driver->framebufferInfo.width;
    
    if (x >= 0 && x < (int)width && y >= 0 && y < (int)driver->framebufferInfo.height) {
        return fb[y * width + x];
    }
    return 0;
}

- (void)fillRect:(NXRect)rect color:(uint32_t)color
{
    if (!driver) return;
    
    DKitRect dr;
    dr.x = (int)rect.origin_x;
    dr.y = (int)rect.origin_y;
    dr.width = (int)rect.size_width;
    dr.height = (int)rect.size_height;
    
    if (driver->fillRect) {
        driver->fillRect((DKitDriverRef)driver, dr, color);
    }
}

- (void)copyRect:(NXRect)src toPoint:(int)dx :(int)dy
{
    if (!driver) return;
    
    DKitRect dr;
    dr.x = (int)src.origin_x;
    dr.y = (int)src.origin_y;
    dr.width = (int)src.size_width;
    dr.height = (int)src.size_height;
    
    if (driver->copyRect) {
        driver->copyRect((DKitDriverRef)driver, dr, dx, dy);
    }
}

- (void)flush
{
    if (!driver) return;
    
    if (driver->flush) {
        driver->flush((DKitDriverRef)driver, 0, 0, 
                      driver->framebufferInfo.width, 
                      driver->framebufferInfo.height);
    }
}

- (void)flushRect:(NXRect)rect
{
    if (!driver) return;
    
    DKitRect dr;
    dr.x = (int)rect.origin_x;
    dr.y = (int)rect.origin_y;
    dr.width = (int)rect.size_width;
    dr.height = (int)rect.size_height;
    
    if (driver->flushRect) {
        driver->flushRect((DKitDriverRef)driver, dr);
    }
}

- (BOOL)supportsMode:(int)width height:(int)height depth:(int)bpp
{
    if (!driver) return NO;
    
    if (driver->getDisplayModes) {
        DKitDisplayMode *modes = NULL;
        uint32_t count = 0;
        
        if (driver->getDisplayModes((DKitDriverRef)driver, &modes, &count) == DKIT_SUCCESS) {
            int i;
            for (i = 0; i < (int)count; i++) {
                if (modes[i].width == width && 
                    modes[i].height == height && 
                    modes[i].bitsPerPixel == bpp) {
                    return YES;
                }
            }
        }
    }
    
    /* Fallback: assume all modes supported */
    return YES;
}

- (BOOL)setMode:(int)width height:(int)height depth:(int)bpp
{
    if (!driver) return NO;
    
    if (driver->setMode) {
        int result = driver->setMode((DKitDriverRef)driver, width, height, bpp);
        if (result == DKIT_SUCCESS) {
            screenRect.size_width = width;
            screenRect.size_height = height;
            depth = bpp;
            return YES;
        }
    }
    
    return NO;
}

@end

void DeviceKit_InitializeFrameBuffer(void)
{
    [NXFrameBuffer initializeFrameBuffer];
}

void DeviceKit_ShutdownFrameBuffer(void)
{
    [NXFrameBuffer shutdown];
}

DKitFramebufferDriverRef DeviceKit_GetFramebufferDriver(void)
{
    return (DKitFramebufferDriverRef)DKitGetFramebufferDriver();
}

uint32_t *DeviceKit_GetFramebufferMemory(void)
{
    NXFrameBuffer *fb = [NXFrameBuffer sharedFrameBuffer];
    return (uint32_t *)[fb framebufferAddress];
}