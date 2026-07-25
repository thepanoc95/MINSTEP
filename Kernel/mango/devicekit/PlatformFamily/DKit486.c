/*
 * mango/devicekit/PlatformFamily/DKit486.c
 *
 * i386/x86 Platform Family implementation.
 *
 * Copyright (c) 2026 MinSTEP Project
 */

#include "../Headers/DeviceKit.h"
#include <stdio.h>

static void i386_detect(void)
{
    printf("DKit: i386 platform detected\n");
}

int i386_initialize_framebuffer(void)
{
    i386_detect();
    
    /* Same as x86_64 but for 32-bit */
    if (getenv("DISPLAY") != NULL) {
        DKitDriverRef x11Driver = DKitCreateDriver_X11Framebuffer();
        if (x11Driver) {
            DKitRegisterDriver(x11Driver);
            DKitSetFramebufferDriver(x11Driver);
            return DKIT_SUCCESS;
        }
    }
    
    DKitDriverRef nullDriver = DKitCreateDriver_NoFramebuffer();
    if (nullDriver) {
        DKitRegisterDriver(nullDriver);
        DKitSetFramebufferDriver(nullDriver);
        return DKIT_SUCCESS;
    }
    
    return DKIT_ERROR_GENERAL;
}

const char *i386_get_name(void)
{
    return "i386";
}

uint32_t i386_get_cpu_type(void)
{
    return 0x0000000A; /* CPU_TYPE_I386 in NeXT format */
}