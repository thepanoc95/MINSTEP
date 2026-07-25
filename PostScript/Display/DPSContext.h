/*
 * Display PostScript subset for MINSTEP.
 *
 * This API intentionally mirrors the small, embeddable shape of the
 * PostScript 1.0 printer sources under PostScript/Adobe while adding
 * screen-oriented devices and a tiny event-friendly context object.
 */
#ifndef MINSTEP_DPS_CONTEXT_H
#define MINSTEP_DPS_CONTEXT_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    DPS_DEVICE_ASCII = 0,
    DPS_DEVICE_LINUX_CONSOLE = 1,
    DPS_DEVICE_MEMORY = 2
} DPSDeviceKind;

typedef struct DPSContext DPSContext;

DPSContext *DPSCreateContext(int width, int height, DPSDeviceKind kind);
void DPSDestroyContext(DPSContext *ctx);
void DPSSetOutput(DPSContext *ctx, const char *path);
int DPSExecuteString(DPSContext *ctx, const char *program);
int DPSFlush(DPSContext *ctx);
const char *DPSError(DPSContext *ctx);
int DPSWidth(DPSContext *ctx);
int DPSHeight(DPSContext *ctx);
unsigned char *DPSPixels(DPSContext *ctx);
void DPSClear(DPSContext *ctx, unsigned char value);

#ifdef __cplusplus
}
#endif

#endif /* MINSTEP_DPS_CONTEXT_H */
