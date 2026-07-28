#include "../Headers/DeviceKit.h"
#include <stdio.h>
 
static void mips_detect(void)
{
    printf("DKit: mips platform detected\n");
}
 
 int mips_initialize_framebuffer(void)
 {
     mips_detect();

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

 void endianness(void) {
    //
 }
 
 const char *mips_get_name(void)
 {
     return "mips";
 }
 
 uint32_t mips_get_cpu_type(void)
 {
     return 0x00000008; /* CPU_TYPE_MIPS in NeXT format */
 }