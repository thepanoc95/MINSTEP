/* MINSTEP Display PostScript display-system layer. */
#ifndef MINSTEP_DPS_SYSTEM_H
#define MINSTEP_DPS_SYSTEM_H

#include "DPSContext.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct DPSDisplay DPSDisplay;
typedef struct DPSWindow DPSWindow;

typedef enum {
    DPS_EVENT_NONE = 0,
    DPS_EVENT_EXPOSE,
    DPS_EVENT_KEY,
    DPS_EVENT_MOUSE,
    DPS_EVENT_CLOSE
} DPSEventType;

typedef struct {
    DPSEventType type;
    int window_id;
    int x;
    int y;
    int button;
    int key;
} DPSEvent;

DPSDisplay *DPSDisplayCreate(int width, int height, DPSDeviceKind kind);
void DPSDisplayDestroy(DPSDisplay *display);
DPSWindow *DPSWindowCreate(DPSDisplay *display, int x, int y, int width, int height, const char *title);
int DPSWindowID(DPSWindow *window);
DPSContext *DPSWindowContext(DPSWindow *window);
int DPSWindowOrderFront(DPSWindow *window);
int DPSWindowMove(DPSWindow *window, int x, int y);
int DPSWindowSetTitle(DPSWindow *window, const char *title);
int DPSDisplayPostEvent(DPSDisplay *display, DPSEvent event);
int DPSDisplayNextEvent(DPSDisplay *display, DPSEvent *event);
int DPSDisplayComposite(DPSDisplay *display);
int DPSDisplayFlush(DPSDisplay *display);
const char *DPSDisplayError(DPSDisplay *display);

#ifdef __cplusplus
}
#endif

#endif /* MINSTEP_DPS_SYSTEM_H */
