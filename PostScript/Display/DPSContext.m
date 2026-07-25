/* #objc: MINSTEP Objective-C translation unit */
/*
 * DPSContext.m - Display PostScript subset for MINSTEP.
 *
 * The original Adobe printer code is organized around operand stacks,
 * dictionaries, graphics state, paths, and devices.  This file follows
 * that structure in a smaller user-mode form suitable for bootstrapping
 * MINSTEP's window server before the full PostScript interpreter is made
 * re-entrant.
 */

#include "DPSContext.h"

#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum { DPS_OP_NUMBER, DPS_OP_NAME } DPSOperandType;

typedef struct {
    DPSOperandType type;
    double number;
    char name[64];
} DPSOperand;

typedef struct {
    double x;
    double y;
    unsigned char move;
} DPSPoint;

typedef struct {
    double gray;
    double line_width;
    double current_x;
    double current_y;
    int has_current;
} DPSGState;

struct DPSContext {
    int width;
    int height;
    DPSDeviceKind kind;
    unsigned char *pixels;
    DPSOperand stack[128];
    int stack_depth;
    DPSPoint path[512];
    int path_count;
    DPSGState gstate;
    char output_path[256];
    char error[160];
};

static void dps_set_error(DPSContext *ctx, const char *message) {
    if (!ctx) return;
    strncpy(ctx->error, message, sizeof(ctx->error) - 1);
    ctx->error[sizeof(ctx->error) - 1] = '\0';
}

static int dps_push_number(DPSContext *ctx, double number) {
    if (ctx->stack_depth >= (int)(sizeof(ctx->stack) / sizeof(ctx->stack[0]))) {
        dps_set_error(ctx, "stackoverflow");
        return 0;
    }
    ctx->stack[ctx->stack_depth].type = DPS_OP_NUMBER;
    ctx->stack[ctx->stack_depth].number = number;
    ctx->stack_depth++;
    return 1;
}

static int dps_pop_number(DPSContext *ctx, double *number) {
    if (ctx->stack_depth <= 0) {
        dps_set_error(ctx, "stackunderflow");
        return 0;
    }
    ctx->stack_depth--;
    if (ctx->stack[ctx->stack_depth].type != DPS_OP_NUMBER) {
        dps_set_error(ctx, "typecheck");
        return 0;
    }
    *number = ctx->stack[ctx->stack_depth].number;
    return 1;
}

void DPSClear(DPSContext *ctx, unsigned char value) {
    if (!ctx || !ctx->pixels) return;
    memset(ctx->pixels, value, (size_t)ctx->width * (size_t)ctx->height);
}

static void dps_plot(DPSContext *ctx, int x, int y) {
    if (x < 0 || y < 0 || x >= ctx->width || y >= ctx->height) return;
    ctx->pixels[(size_t)y * (size_t)ctx->width + (size_t)x] =
        (unsigned char)lrint((1.0 - ctx->gstate.gray) * 255.0);
}

static void dps_line(DPSContext *ctx, double ax, double ay, double bx, double by) {
    int x0 = (int)lrint(ax), y0 = (int)lrint(ay);
    int x1 = (int)lrint(bx), y1 = (int)lrint(by);
    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy;

    for (;;) {
        dps_plot(ctx, x0, y0);
        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

static int dps_add_path(DPSContext *ctx, double x, double y, unsigned char move) {
    if (ctx->path_count >= (int)(sizeof(ctx->path) / sizeof(ctx->path[0]))) {
        dps_set_error(ctx, "limitcheck");
        return 0;
    }
    ctx->path[ctx->path_count].x = x;
    ctx->path[ctx->path_count].y = y;
    ctx->path[ctx->path_count].move = move;
    ctx->path_count++;
    ctx->gstate.current_x = x;
    ctx->gstate.current_y = y;
    ctx->gstate.has_current = 1;
    return 1;
}

static int dps_stroke(DPSContext *ctx) {
    int i;
    for (i = 1; i < ctx->path_count; i++) {
        if (!ctx->path[i].move) {
            dps_line(ctx, ctx->path[i - 1].x, ctx->path[i - 1].y,
                     ctx->path[i].x, ctx->path[i].y);
        }
    }
    ctx->path_count = 0;
    return 1;
}

static int dps_fill_rect(DPSContext *ctx, int x, int y, int w, int h) {
    int ix, iy;
    if (w < 0) { x += w; w = -w; }
    if (h < 0) { y += h; h = -h; }
    for (iy = y; iy < y + h; iy++) {
        for (ix = x; ix < x + w; ix++) dps_plot(ctx, ix, iy);
    }
    return 1;
}

static int dps_execute_operator(DPSContext *ctx, const char *op) {
    double a, b, c, d;
    if (strcmp(op, "moveto") == 0) {
        if (!dps_pop_number(ctx, &b) || !dps_pop_number(ctx, &a)) return 0;
        return dps_add_path(ctx, a, b, 1);
    } else if (strcmp(op, "lineto") == 0) {
        if (!dps_pop_number(ctx, &b) || !dps_pop_number(ctx, &a)) return 0;
        if (!ctx->gstate.has_current) return dps_add_path(ctx, a, b, 1);
        return dps_add_path(ctx, a, b, 0);
    } else if (strcmp(op, "rlineto") == 0) {
        if (!dps_pop_number(ctx, &b) || !dps_pop_number(ctx, &a)) return 0;
        return dps_add_path(ctx, ctx->gstate.current_x + a, ctx->gstate.current_y + b, 0);
    } else if (strcmp(op, "stroke") == 0) {
        return dps_stroke(ctx);
    } else if (strcmp(op, "newpath") == 0) {
        ctx->path_count = 0; ctx->gstate.has_current = 0; return 1;
    } else if (strcmp(op, "setgray") == 0) {
        if (!dps_pop_number(ctx, &a)) return 0;
        if (a < 0.0) a = 0.0;
        if (a > 1.0) a = 1.0;
        ctx->gstate.gray = a; return 1;
    } else if (strcmp(op, "rectfill") == 0) {
        if (!dps_pop_number(ctx, &d) || !dps_pop_number(ctx, &c) ||
            !dps_pop_number(ctx, &b) || !dps_pop_number(ctx, &a)) return 0;
        return dps_fill_rect(ctx, (int)lrint(a), (int)lrint(b), (int)lrint(c), (int)lrint(d));
    } else if (strcmp(op, "erasepage") == 0 || strcmp(op, "showpage") == 0) {
        if (strcmp(op, "erasepage") == 0) DPSClear(ctx, 0);
        return 1;
    }
    dps_set_error(ctx, "undefined");
    return 0;
}

DPSContext *DPSCreateContext(int width, int height, DPSDeviceKind kind) {
    DPSContext *ctx;
    if (width <= 0) width = 80;
    if (height <= 0) height = 25;
    ctx = (DPSContext *)calloc(1, sizeof(DPSContext));
    if (!ctx) return NULL;
    ctx->pixels = (unsigned char *)calloc((size_t)width * (size_t)height, 1);
    if (!ctx->pixels) { free(ctx); return NULL; }
    ctx->width = width;
    ctx->height = height;
    ctx->kind = kind;
    ctx->gstate.gray = 0.0;
    ctx->gstate.line_width = 1.0;
    strcpy(ctx->output_path, "-");
    return ctx;
}

void DPSDestroyContext(DPSContext *ctx) {
    if (!ctx) return;
    free(ctx->pixels);
    free(ctx);
}

void DPSSetOutput(DPSContext *ctx, const char *path) {
    if (!ctx || !path) return;
    strncpy(ctx->output_path, path, sizeof(ctx->output_path) - 1);
    ctx->output_path[sizeof(ctx->output_path) - 1] = '\0';
}

int DPSExecuteString(DPSContext *ctx, const char *program) {
    const char *p;
    if (!ctx || !program) return 0;
    ctx->error[0] = '\0';
    for (p = program; *p; ) {
        char token[80];
        char *endptr;
        size_t len = 0;
        while (*p && isspace((unsigned char)*p)) p++;
        if (*p == '%') { while (*p && *p != '\n') p++; continue; }
        if (!*p) break;
        while (p[len] && !isspace((unsigned char)p[len]) && len < sizeof(token) - 1) len++;
        memcpy(token, p, len); token[len] = '\0'; p += len;
        errno = 0;
        double value = strtod(token, &endptr);
        if (endptr != token && *endptr == '\0' && errno == 0) {
            if (!dps_push_number(ctx, value)) return 0;
        } else if (!dps_execute_operator(ctx, token)) {
            return 0;
        }
    }
    return 1;
}

int DPSFlush(DPSContext *ctx) {
    static const char ramp[] = " .:-=+*#%@";
    FILE *out;
    int x, y;
    if (!ctx) return 0;
    out = strcmp(ctx->output_path, "-") == 0 ? stdout : fopen(ctx->output_path, "w");
    if (!out) { dps_set_error(ctx, strerror(errno)); return 0; }

    if (ctx->kind == DPS_DEVICE_LINUX_CONSOLE) fputs("\033[H\033[2J", out);
    for (y = 0; y < ctx->height; y++) {
        for (x = 0; x < ctx->width; x++) {
            unsigned char p = ctx->pixels[(size_t)y * (size_t)ctx->width + (size_t)x];
            fputc(ramp[(p * (sizeof(ramp) - 2)) / 255], out);
        }
        fputc('\n', out);
    }
    if (out != stdout) fclose(out);
    return 1;
}

const char *DPSError(DPSContext *ctx) {
    return ctx && ctx->error[0] ? ctx->error : "noerror";
}

int DPSWidth(DPSContext *ctx) { return ctx ? ctx->width : 0; }
int DPSHeight(DPSContext *ctx) { return ctx ? ctx->height : 0; }
unsigned char *DPSPixels(DPSContext *ctx) { return ctx ? ctx->pixels : NULL; }
