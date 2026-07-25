/*
 * appkit/dps.h - AppKit bridge for MINSTEP Display PostScript.
 */
#ifndef _APPKIT_DPS_H_
#define _APPKIT_DPS_H_

#include "../../PostScript/Display/DPSSystem.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MX_DPS_FORCE_ENV "MX_FORCE_DPS"
#define MX_DPS_DISABLE_ENV "MX_DISABLE_DPS"
#define MX_DPS_DEVICE_ENV "MX_DPS_DEVICE"
#define MX_DPS_SIZE_ENV "MX_DPS_SIZE"

typedef struct MXDPSApplication MXDPSApplication;
typedef struct MXDPSWindow MXDPSWindow;

typedef enum {
    MX_DPS_BACKEND_ASCII = DPS_DEVICE_ASCII,
    MX_DPS_BACKEND_LINUX_CONSOLE = DPS_DEVICE_LINUX_CONSOLE,
    MX_DPS_BACKEND_MEMORY = DPS_DEVICE_MEMORY
} MXDPSBackend;

int MXUseDPS(void);
MXDPSBackend MXDPSDefaultBackend(void);
void MXDPSDefaultSize(int *width, int *height);

MXDPSApplication *MXDPSApplicationCreate(int width, int height, MXDPSBackend backend);
void MXDPSApplicationDestroy(MXDPSApplication *app);
DPSDisplay *MXDPSApplicationDisplay(MXDPSApplication *app);
int MXDPSApplicationRunOnce(MXDPSApplication *app);
int MXDPSApplicationFlush(MXDPSApplication *app);
const char *MXDPSApplicationError(MXDPSApplication *app);

MXDPSWindow *MXDPSWindowCreate(MXDPSApplication *app, int x, int y, int width, int height, const char *title);
void MXDPSWindowDestroy(MXDPSWindow *window);
DPSContext *MXDPSWindowContext(MXDPSWindow *window);
int MXDPSWindowRenderPostScript(MXDPSWindow *window, const char *program);
int MXDPSWindowOrderFront(MXDPSWindow *window);
int MXDPSWindowMove(MXDPSWindow *window, int x, int y);
int MXDPSWindowID(MXDPSWindow *window);

#ifdef __cplusplus
}
#endif

#endif /* _APPKIT_DPS_H_ */
