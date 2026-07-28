#include "../Headers/DeviceKit.h"
#include <stdio.h>

/* ========================================================================
 * x86_64 Platform Detection
 * ======================================================================== */

static void x86_64_detect(void)
{
    printf("DKit: x86_64 platform detected\n");
}

/* ========================================================================
 * Framebuffer Initialization for x86_64
 * ======================================================================== */

int x86_64_initialize_framebuffer(void)
{
    x86_64_detect();
    
    /* Prefer X11 on x86_64 if DISPLAY is set */
    if (getenv("DISPLAY") != NULL) {
        DKitDriverRef x11Driver = DKitCreateDriver_X11Framebuffer();
        if (x11Driver) {
            DKitRegisterDriver(x11Driver);
            DKitSetFramebufferDriver(x11Driver);
            return DKIT_SUCCESS;
        }
    }
    
    /* Fall back to null driver */
    DKitDriverRef nullDriver = DKitCreateDriver_NoFramebuffer();
    if (nullDriver) {
        DKitRegisterDriver(nullDriver);
        DKitSetFramebufferDriver(nullDriver);
        return DKIT_SUCCESS;
    }
    
    return DKIT_ERROR_GENERAL;
}

/* ========================================================================
 * Platform Info
 * ======================================================================== */

const char *x86_64_get_name(void)
{
    return "x86_64";
}

uint32_t x86_64_get_cpu_type(void)
{
    return 0x0100007C; /* CPU_TYPE_X86_64 in NeXT format */
}