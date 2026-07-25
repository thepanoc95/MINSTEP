/* #objc: MINSTEP Objective-C translation unit */
#include "dps.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct MXDPSApplication {
    DPSDisplay *display;
    int width;
    int height;
    MXDPSBackend backend;
    char error[160];
};

struct MXDPSWindow {
    MXDPSApplication *app;
    DPSWindow *window;
};

static void app_error(MXDPSApplication *app, const char *message) {
    if (!app) return;
    strncpy(app->error, message, sizeof(app->error) - 1);
    app->error[sizeof(app->error) - 1] = '\0';
}

static int env_is_true(const char *name) {
    const char *value = getenv(name);
    return value && (strcmp(value, "1") == 0 || strcmp(value, "YES") == 0 || strcmp(value, "true") == 0);
}

int MXUseDPS(void) {
    if (env_is_true(MX_DPS_DISABLE_ENV)) return 0;
    if (env_is_true(MX_DPS_FORCE_ENV)) return 1;
    if (getenv("DISPLAY") && getenv("DISPLAY")[0] != '\0') return 1;
    if (isatty(STDOUT_FILENO)) return 1;
    return 0;
}

MXDPSBackend MXDPSDefaultBackend(void) {
    const char *device = getenv(MX_DPS_DEVICE_ENV);
    if (!device) return isatty(STDOUT_FILENO) ? MX_DPS_BACKEND_ASCII : MX_DPS_BACKEND_MEMORY;
    if (strcmp(device, "linux-console") == 0 || strcmp(device, "console") == 0) return MX_DPS_BACKEND_LINUX_CONSOLE;
    if (strcmp(device, "memory") == 0 || strcmp(device, "framebuffer") == 0) return MX_DPS_BACKEND_MEMORY;
    return MX_DPS_BACKEND_ASCII;
}

void MXDPSDefaultSize(int *width, int *height) {
    int w = 80;
    int h = 25;
    const char *size = getenv(MX_DPS_SIZE_ENV);
    if (size) sscanf(size, "%dx%d", &w, &h);
    if (width) *width = w > 0 ? w : 80;
    if (height) *height = h > 0 ? h : 25;
}

MXDPSApplication *MXDPSApplicationCreate(int width, int height, MXDPSBackend backend) {
    MXDPSApplication *app;
    if (width <= 0 || height <= 0) MXDPSDefaultSize(&width, &height);
    app = (MXDPSApplication *)calloc(1, sizeof(MXDPSApplication));
    if (!app) return NULL;
    app->display = DPSDisplayCreate(width, height, (DPSDeviceKind)backend);
    if (!app->display) { free(app); return NULL; }
    app->width = width;
    app->height = height;
    app->backend = backend;
    return app;
}

void MXDPSApplicationDestroy(MXDPSApplication *app) {
    if (!app) return;
    DPSDisplayDestroy(app->display);
    free(app);
}

DPSDisplay *MXDPSApplicationDisplay(MXDPSApplication *app) { return app ? app->display : NULL; }

int MXDPSApplicationRunOnce(MXDPSApplication *app) {
    DPSEvent event;
    if (!app) return 0;
    while (DPSDisplayNextEvent(app->display, &event)) {
        if (event.type == DPS_EVENT_CLOSE) return 0;
    }
    return MXDPSApplicationFlush(app);
}

int MXDPSApplicationFlush(MXDPSApplication *app) {
    if (!app) return 0;
    if (!DPSDisplayFlush(app->display)) {
        app_error(app, DPSDisplayError(app->display));
        return 0;
    }
    return 1;
}

const char *MXDPSApplicationError(MXDPSApplication *app) {
    return app && app->error[0] ? app->error : "noerror";
}

MXDPSWindow *MXDPSWindowCreate(MXDPSApplication *app, int x, int y, int width, int height, const char *title) {
    MXDPSWindow *window;
    if (!app) return NULL;
    window = (MXDPSWindow *)calloc(1, sizeof(MXDPSWindow));
    if (!window) return NULL;
    window->app = app;
    window->window = DPSWindowCreate(app->display, x, y, width, height, title);
    if (!window->window) { free(window); app_error(app, DPSDisplayError(app->display)); return NULL; }
    return window;
}

void MXDPSWindowDestroy(MXDPSWindow *window) {
    /* The owning DPSDisplay releases DPSWindow storage. */
    free(window);
}

DPSContext *MXDPSWindowContext(MXDPSWindow *window) {
    return window ? DPSWindowContext(window->window) : NULL;
}

int MXDPSWindowRenderPostScript(MXDPSWindow *window, const char *program) {
    DPSContext *ctx = MXDPSWindowContext(window);
    if (!ctx) return 0;
    return DPSExecuteString(ctx, program);
}

int MXDPSWindowOrderFront(MXDPSWindow *window) {
    return window ? DPSWindowOrderFront(window->window) : 0;
}

int MXDPSWindowMove(MXDPSWindow *window, int x, int y) {
    return window ? DPSWindowMove(window->window, x, y) : 0;
}

int MXDPSWindowID(MXDPSWindow *window) {
    return window ? DPSWindowID(window->window) : 0;
}
