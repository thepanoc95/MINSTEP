/* #objc: MINSTEP Objective-C translation unit */
#include "DPSContext.h"
#include "DPSSystem.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char *read_file(const char *path) {
    FILE *f = fopen(path, "rb");
    long n;
    char *buf;
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    n = ftell(f);
    fseek(f, 0, SEEK_SET);
    buf = (char *)malloc((size_t)n + 1);
    if (!buf) { fclose(f); return NULL; }
    if (fread(buf, 1, (size_t)n, f) != (size_t)n) { fclose(f); free(buf); return NULL; }
    buf[n] = '\0';
    fclose(f);
    return buf;
}

int main(int argc, char **argv) {
    const char *program = "5 5 moveto 70 5 lineto 70 20 lineto 5 20 lineto 5 5 lineto stroke";
    DPSDeviceKind kind = DPS_DEVICE_ASCII;
    int width = 80, height = 25, i;
    int demo_system = 0;
    DPSContext *ctx;
    char *loaded = NULL;

    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--linux-console") == 0) kind = DPS_DEVICE_LINUX_CONSOLE;
        else if (strcmp(argv[i], "--demo-system") == 0) demo_system = 1;
        else if (strcmp(argv[i], "--size") == 0 && i + 1 < argc) sscanf(argv[++i], "%dx%d", &width, &height);
        else loaded = read_file(argv[i]);
    }
    if (loaded) program = loaded;

    if (demo_system) {
        DPSDisplay *display = DPSDisplayCreate(width, height, kind);
        DPSWindow *win;
        if (!display) { fprintf(stderr, "dps: display allocation failed\n"); free(loaded); return 1; }
        win = DPSWindowCreate(display, 2, 1, width > 8 ? width - 6 : width, height > 6 ? height - 4 : height, "DPS Demo");
        if (!win || !DPSExecuteString(DPSWindowContext(win), program) || !DPSDisplayFlush(display)) {
            fprintf(stderr, "dps: %s\n", DPSDisplayError(display));
            DPSDisplayDestroy(display); free(loaded); return 1;
        }
        DPSDisplayDestroy(display);
        free(loaded);
        return 0;
    }

    ctx = DPSCreateContext(width, height, kind);
    if (!ctx) { fprintf(stderr, "dps: out of memory\n"); free(loaded); return 1; }
    if (!DPSExecuteString(ctx, program) || !DPSFlush(ctx)) {
        fprintf(stderr, "dps: %s\n", DPSError(ctx));
        DPSDestroyContext(ctx); free(loaded); return 1;
    }
    DPSDestroyContext(ctx);
    free(loaded);
    return 0;
}
