/*
 * appkit/terminal/tui.m - MinSTEP TUI Implementation
 *
 * Terminal-based UI rendering for AppKit applications.
 *
 * Copyright (c) 2026 MinSTEP Project
 * Licensed under the MIT License.
 */

#objc
#import <appkit/terminal/tui.h>
#import <appkit/appkit.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <signal.h>

/* Global state */
static MXTUIConsole *_sharedConsole = nil;
static MXTUIEventLoop *_sharedEventLoop = nil;
static struct termios _origTermios;
static BOOL _rawModeEnabled = NO;

/* ========================================================================
 * Terminal Control Sequences
 * ======================================================================== */

#define TUI_ESC "\033["
#define TUI_CSI_RESET    TUI_ESC "0m"
#define TUI_CSI_CLEAR    TUI_ESC "2J"
#define TUI_CSI_HOME     TUI_ESC "H"
#define TUI_CSI_HIDE_CURSOR    TUI_ESC "?25l"
#define TUI_CSI_SHOW_CURSOR    TUI_ESC "?25h"
#define TUI_CSI_ALT_SCREEN     TUI_ESC "?47l"
#define TUI_CSI_NORMAL_SCREEN  TUI_ESC "?47h"
#define TUI_CSI_SAVE_CURSOR    TUI_ESC "s"
#define TUI_CSI_RESTORE_CURSOR TUI_ESC "u"
#define TUI_CSI_SCROLL_START   TUI_ESC "1S"
#define TUI_CSI_SCROLL_END     TUI_ESC "1S"

#define TUI_MOVE_CURSOR(x, y) printf(TUI_ESC "%d;%dH", (y)+1, (x)+1)
#define TUI_SET_FG(c) printf(TUI_ESC "%dm", 30 + (c))
#define TUI_SET_BG(c) printf(TUI_ESC "%dm", 40 + (c))
#define TUI_SET_ATTR(a) printf(TUI_ESC "%dm", (a))
#define TUI_RESET_ATTR printf(TUI_ESC "0m")

/* ========================================================================
 * MXTUIConsole Implementation
 * ======================================================================== */

@implementation MXTUIConsole

+ (MXTUIConsole *)sharedConsole
{
    if (!_sharedConsole) {
        _sharedConsole = [self new];
    }
    return _sharedConsole;
}

+ (id)new
{
    MXTUIConsole *obj = [super new];
    if (obj) {
        obj->screenWidth = 80;
        obj->screenHeight = 24;
        obj->cursorPosition = TUIPointMake(0, 0);
        obj->cursorVisible = YES;
        obj->alternateScreen = NO;
        obj->rawMode = NO;
        obj->output = stdout;
        obj->input = stdin;
    }
    return obj;
}

- (void)initialize
{
    /* Get terminal size */
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0) {
        screenWidth = w.ws_col;
        screenHeight = w.ws_row;
    }
    
    /* Save original terminal settings */
    tcgetattr(STDIN_FILENO, &_origTermios);
    
    /* Set raw mode */
    struct termios raw = _origTermios;
    raw.c_lflag &= ~(ICANON | ECHO | ISIG);
    raw.c_iflag &= ~(IXON | IXOFF | ICRNL);
    raw.c_oflag &= ~OPOST;
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
    _rawModeEnabled = YES;
    
    /* Enable alternate screen buffer */
    [self enableAlternateScreen:YES];
    
    /* Hide cursor during rendering */
    [self hideCursor];
    
    /* Clear screen on init */
    [self clear];
}

- (void)shutdown
{
    /* Restore terminal state */
    [self resetAttributes];
    [self showCursor];
    [self enableAlternateScreen:NO];
    
    if (_rawModeEnabled) {
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &_origTermios);
        _rawModeEnabled = NO;
    }
}

- (void)clear
{
    printf(TUI_CSI_CLEAR);
    printf(TUI_CSI_HOME);
    fflush(stdout);
}

- (void)refresh
{
    fflush(stdout);
}

- (int)screenWidth
{
    return screenWidth;
}

- (int)screenHeight
{
    return screenHeight;
}

- (void)moveCursorTo:(int)x y:(int)y
{
    if (x < 0) x = 0;
    if (y < 0) y = 0;
    if (x >= screenWidth) x = screenWidth - 1;
    if (y >= screenHeight) y = screenHeight - 1;
    
    TUI_MOVE_CURSOR(x, y);
    cursorPosition.x = x;
    cursorPosition.y = y;
}

- (void)hideCursor
{
    printf(TUI_CSI_HIDE_CURSOR);
    fflush(stdout);
    cursorVisible = NO;
}

- (void)showCursor
{
    printf(TUI_CSI_SHOW_CURSOR);
    fflush(stdout);
    cursorVisible = YES;
}

- (void)enableAlternateScreen:(BOOL)enable
{
    if (enable && !alternateScreen) {
        printf(TUI_CSI_ALT_SCREEN);
        alternateScreen = YES;
    } else if (!enable && alternateScreen) {
        printf(TUI_CSI_NORMAL_SCREEN);
        alternateScreen = NO;
    }
    fflush(stdout);
}

- (void)enableRawMode:(BOOL)enable
{
    rawMode = enable;
    /* Already handled in initialize/shutdown */
}

- (void)setForegroundColor:(int)fg backgroundColor:(int)bg
{
    if (fg >= 0 && fg <= 7) {
        TUI_SET_FG(fg);
    } else if (fg == TUI_COLOR_DEFAULT) {
        printf(TUI_ESC "39m");
    }
    if (bg >= 0 && bg <= 7) {
        TUI_SET_BG(bg);
    } else if (bg == TUI_COLOR_DEFAULT) {
        printf(TUI_ESC "49m");
    }
    fflush(stdout);
}

- (void)setAttributes:(int)attrs
{
    TUI_SET_ATTR(attrs);
    fflush(stdout);
}

- (void)resetAttributes
{
    TUI_RESET_ATTR;
    fflush(stdout);
}

- (void)saveState
{
    printf(TUI_CSI_SAVE_CURSOR);
    fflush(stdout);
}

- (void)restoreState
{
    printf(TUI_CSI_RESTORE_CURSOR);
    fflush(stdout);
}

- (void)beep
{
    printf("\a");
    fflush(stdout);
}

@end

/* ========================================================================
 * MXTUIRenderer Implementation
 * ======================================================================== */

@implementation MXTUIRenderer

+ (MXTUIRenderer *)rendererWithConsole:(MXTUIConsole *)console
{
    MXTUIRenderer *r = [self new];
    r->console = console;
    return r;
}

/* Single-line box characters */
static const char *BOX_SINGLE_TL = "+";
static const char *BOX_SINGLE_TR = "+";
static const char *BOX_SINGLE_BL = "+";
static const char *BOX_SINGLE_BR = "+";
static const char *BOX_SINGLE_H = "-";
static const char *BOX_SINGLE_V = "|";

/* Double-line box characters */
static const char *BOX_DOUBLE_TL = "+";
static const char *BOX_DOUBLE_TR = "+";
static const char *BOX_DOUBLE_BL = "+";
static const char *BOX_DOUBLE_BR = "+";
static const char *BOX_DOUBLE_H = "=";
static const char *BOX_DOUBLE_V = "#";

/* ASCII box characters */
static const char *BOX_ASCII_TL = "+";
static const char *BOX_ASCII_TR = "+";
static const char *BOX_ASCII_BL = "+";
static const char *BOX_ASCII_BR = "+";
static const char *BOX_ASCII_H = "-";
static const char *BOX_ASCII_V = "|";

- (void)drawBox:(TUIRect)rect title:(const char *)title style:(int)style
{
    if (rect.width < 2 || rect.height < 2) return;
    
    MXTUIConsole *con = [MXTUIConsole sharedConsole];
    
    /* Draw top border */
    [con moveCursorTo:rect.x y:rect.y];
    if (style == TUI_BORDER_SINGLE) {
        printf("+");
        int i;
        for (i = 1; i < rect.width - 1; i++) {
            printf("-");
        }
        printf("+");
    } else if (style == TUI_BORDER_DOUBLE) {
        printf("+");
        int i;
        for (i = 1; i < rect.width - 1; i++) {
            printf("=");
        }
        printf("+");
    } else if (style == TUI_BORDER_ASCII) {
        printf("+");
        int i;
        for (i = 1; i < rect.width - 1; i++) {
            printf("-");
        }
        printf("+");
    } else if (style == TUI_BORDER_ROUNDED) {
        printf("+");
        int i;
        for (i = 1; i < rect.width - 1; i++) {
            printf("-");
        }
        printf("+");
    }
    
    /* Draw title if present */
    if (title && rect.width > 4) {
        [con moveCursorTo:rect.x + 1 y:rect.y];
        [con setAttributes:TUI_ATTR_REVERSE];
        printf(" %s ", title);
        [con resetAttributes];
    }
    
    /* Draw vertical borders */
    int y;
    for (y = rect.y + 1; y < rect.y + rect.height - 1; y++) {
        [con moveCursorTo:rect.x y:y];
        printf("|");
        [con moveCursorTo:rect.x + rect.width - 1 y:y];
        printf("|");
    }
    
    /* Draw bottom border */
    [con moveCursorTo:rect.x y:rect.y + rect.height - 1];
    printf("+");
    int i;
    for (i = 1; i < rect.width - 1; i++) {
        printf("-");
    }
    printf("+");
    
    [con refresh];
}

- (void)drawLineFrom:(TUIPoint)from to:(TUIPoint)to
{
    MXTUIConsole *con = [MXTUIConsole sharedConsole];
    
    if (from.y == to.y) {
        /* Horizontal line */
        [con moveCursorTo:from.x y:from.y];
        int x;
        for (x = from.x; x <= to.x; x++) {
            printf("-");
        }
    } else if (from.x == to.x) {
        /* Vertical line */
        int y;
        for (y = from.y; y <= to.y; y++) {
            [con moveCursorTo:from.x y:y];
            printf("|");
        }
    }
    
    [con refresh];
}

- (void)drawText:(const char *)text at:(TUIPoint)pos
{
    if (!text) return;
    MXTUIConsole *con = [MXTUIConsole sharedConsole];
    [con moveCursorTo:pos.x y:pos.y];
    printf("%s", text);
    [con refresh];
}

- (void)drawText:(const char *)text inRect:(TUIRect)rect alignment:(int)align
{
    if (!text) return;
    MXTUIConsole *con = [MXTUIConsole sharedConsole];
    int len = strlen(text);
    int x = rect.x;
    
    if (align == 1) { /* Center */
        x = rect.x + (rect.width - len) / 2;
        if (x < rect.x) x = rect.x;
    } else if (align == 2) { /* Right */
        x = rect.x + rect.width - len;
        if (x < rect.x) x = rect.x;
    }
    
    [con moveCursorTo:x y:rect.y];
    if (len > rect.width) {
        /* Truncate */
        int i;
        for (i = 0; i < rect.width - 1; i++) {
            printf("%c", text[i]);
        }
        printf(">");
    } else {
        printf("%s", text);
    }
    [con refresh];
}

- (void)drawChar:(char)c at:(TUIPoint)pos
{
    MXTUIConsole *con = [MXTUIConsole sharedConsole];
    [con moveCursorTo:pos.x y:pos.y];
    printf("%c", c);
    [con refresh];
}

- (void)fillRect:(TUIRect)rect withChar:(char)ch
{
    MXTUIConsole *con = [MXTUIConsole sharedConsole];
    int y;
    for (y = rect.y; y < rect.y + rect.height && y < con.screenHeight; y++) {
        [con moveCursorTo:rect.x y:y];
        int x;
        for (x = rect.x; x < rect.x + rect.width && x < con.screenWidth; x++) {
            printf("%c", ch);
        }
    }
    [con refresh];
}

- (void)drawSeparator:(int)y x1:(int)x1 x2:(int)x2
{
    MXTUIConsole *con = [MXTUIConsole sharedConsole];
    [con moveCursorTo:x1 y:y];
    int x;
    for (x = x1; x < x2; x++) {
        printf("-");
    }
    [con refresh];
}

/* Button drawing */
- (void)drawButton:(const char *)title at:(TUIPoint)pos 
           width:(int)width selected:(BOOL)selected
{
    if (!title) return;
    MXTUIConsole *con = [MXTUIConsole sharedConsole];
    int len = strlen(title);
    int w = (width > len + 4) ? width : len + 4;
    
    [con moveCursorTo:pos.x y:pos.y];
    
    if (selected) {
        [con setAttributes:TUI_ATTR_REVERSE];
        printf("[ ");
        printf("%s", title);
        int padding = w - len - 4;
        int i;
        for (i = 0; i < padding; i++) printf(" ");
        printf(" ]");
        [con resetAttributes];
    } else {
        printf("< ");
        printf("%s", title);
        int padding = w - len - 4;
        int i;
        for (i = 0; i < padding; i++) printf(" ");
        printf(" >");
    }
    
    [con refresh];
}

/* Text field drawing */
- (void)drawTextField:(const char *)text at:(TUIPoint)pos 
              width:(int)width hasFocus:(BOOL)focus
{
    MXTUIConsole *con = [MXTUIConsole sharedConsole];
    
    [con moveCursorTo:pos.x y:pos.y];
    printf("|");
    
    if (text) {
        int len = strlen(text);
        if (len > width - 2) {
            int i;
            for (i = len - width + 3; i < len; i++) {
                printf("%c", text[i]);
            }
        } else {
            printf("%s", text);
            int i;
            for (i = len; i < width - 2; i++) {
                printf(" ");
            }
        }
    } else {
        int i;
        for (i = 0; i < width - 2; i++) {
            printf(" ");
        }
    }
    
    printf("|");
    
    if (focus) {
        /* Show cursor */
        [con moveCursorTo:pos.x + 1 y:pos.y];
    }
    
    [con refresh];
}

/* Scrollbar drawing */
- (void)drawScrollbar:(int)x y:(int)y height:(int)height 
              position:(float)pos thumbSize:(float)thumbSize
{
    MXTUIConsole *con = [MXTUIConsole sharedConsole];
    
    int thumbH = (int)(height * thumbSize);
    if (thumbH < 1) thumbH = 1;
    int thumbY = y + (int)((height - thumbH) * pos);
    if (thumbY < y) thumbY = y;
    if (thumbY + thumbH > y + height) {
        thumbY = y + height - thumbH;
    }
    
    int i;
    for (i = 0; i < height; i++) {
        [con moveCursorTo:x y:y + i];
        if (i >= thumbY - y && i < thumbY - y + thumbH) {
            printf("#");
        } else {
            printf("|");
        }
    }
    
    [con refresh];
}

/* Checkbox drawing */
- (void)drawCheckbox:(const char *)label at:(TUIPoint)pos checked:(BOOL)checked
{
    MXTUIConsole *con = [MXTUIConsole sharedConsole];
    
    [con moveCursorTo:pos.x y:pos.y];
    if (checked) {
        printf("[X] %s", label ? label : "");
    } else {
        printf("[ ] %s", label ? label : "");
    }
    
    [con refresh];
}

/* Progress bar drawing */
- (void)drawProgressBar:(TUIRect)rect progress:(float)progress
{
    MXTUIConsole *con = [MXTUIConsole sharedConsole];
    
    if (progress < 0) progress = 0;
    if (progress > 1) progress = 1;
    
    [con moveCursorTo:rect.x y:rect.y];
    printf("[");
    
    int filled = (int)(rect.width * progress);
    int i;
    for (i = 0; i < rect.width; i++) {
        if (i < filled) {
            printf("#");
        } else {
            printf("-");
        }
    }
    printf("]");
    
    [con refresh];
}

@end

/* ========================================================================
 * MXTUIWidget Implementation
 * ======================================================================== */

@implementation MXTUIWidget

+ (id)new
{
    MXTUIWidget *obj = [super new];
    if (obj) {
        obj->frame = TUIRectMake(0, 0, 80, 24);
        obj->bounds = obj->frame;
        obj->parent = nil;
        obj->children = NULL;
        obj->childCount = 0;
        obj->childCapacity = 0;
        obj->needsDisplay = YES;
        obj->isFocused = NO;
        obj->foregroundColor = TUI_COLOR_WHITE;
        obj->backgroundColor = TUI_COLOR_BLACK;
        obj->borderStyle = TUI_BORDER_SINGLE;
    }
    return obj;
}

- (void)setFrame:(TUIRect)newFrame
{
    frame = newFrame;
    [self setNeedsDisplay:YES];
}

- (TUIRect)frame
{
    return frame;
}

- (void)setBounds:(TUIRect)newBounds
{
    bounds = newBounds;
}

- (TUIRect)bounds
{
    return bounds;
}

- (void)addChild:(MXTUIWidget *)child
{
    if (childCount >= childCapacity) {
        childCapacity = childCapacity ? childCapacity * 2 : 4;
        children = (MXTUIWidget **)realloc(children, childCapacity * sizeof(MXTUIWidget *));
    }
    children[childCount++] = child;
    child->parent = self;
}

- (void)removeChild:(MXTUIWidget *)child
{
    int i;
    for (i = 0; i < childCount; i++) {
        if (children[i] == child) {
            int j;
            for (j = i; j < childCount - 1; j++) {
                children[j] = children[j + 1];
            }
            childCount--;
            child->parent = nil;
            break;
        }
    }
}

- (void)display
{
    if (needsDisplay) {
        [self draw];
        needsDisplay = NO;
    }
    
    int i;
    for (i = 0; i < childCount; i++) {
        [children[i] display];
    }
}

- (void)displayInRect:(TUIRect)clipRect
{
    /* Clip drawing to rect */
    [self display];
}

- (void)draw
{
    /* Subclasses override */
}

- (BOOL)isFocused
{
    return isFocused;
}

- (void)setFocused:(BOOL)focused
{
    isFocused = focused;
    [self setNeedsDisplay:YES];
}

- (BOOL)needsDisplay
{
    return needsDisplay;
}

- (void)setNeedsDisplay:(BOOL)needs
{
    needsDisplay = needs;
    if (needs && parent) {
        [parent setNeedsDisplay:YES];
    }
}

- (BOOL)containsPoint:(TUIPoint)point
{
    return (point.x >= frame.x && point.x < frame.x + frame.width &&
            point.y >= frame.y && point.y < frame.y + frame.height);
}

- (void)moveTo:(int)x y:(int)y
{
    frame.x = x;
    frame.y = y;
    [self setNeedsDisplay:YES];
}

- (void)resizeTo:(int)width height:(int)height
{
    frame.width = width;
    frame.height = height;
    bounds.width = width;
    bounds.height = height;
    [self setNeedsDisplay:YES];
}

@end

/* ========================================================================
 * MXTUIWindow Implementation
 * ======================================================================== */

@implementation MXTUIWindow

+ (id)new
{
    MXTUIWindow *obj = [super new];
    if (obj) {
        obj->title = NULL;
        obj->titleBar = NULL;
        obj->titleBarHeight = 1;
        obj->contentChildren = NULL;
        obj->contentChildCount = 0;
        obj->hasBorder = YES;
        obj->isModal = NO;
        obj->isMovable = YES;
        obj->isResizable = NO;
    }
    return obj;
}

+ (id)windowWithTitle:(const char *)aTitle frame:(TUIRect)aFrame
{
    MXTUIWindow *win = [self new];
    [win setFrame:aFrame];
    [win setTitle:aTitle];
    return win;
}

- (void)setTitle:(const char *)newTitle
{
    if (title) free((void *)title);
    title = newTitle ? strdup(newTitle) : NULL;
    [self setNeedsDisplay:YES];
}

- (const char *)title
{
    return title;
}

- (void)setHasBorder:(BOOL)hasBorderVal
{
    hasBorder = hasBorderVal;
    [self setNeedsDisplay:YES];
}

- (BOOL)hasBorder
{
    return hasBorder;
}

- (void)setMovable:(BOOL)movable
{
    isMovable = movable;
}

- (BOOL)isMovable
{
    return isMovable;
}

- (void)setResizable:(BOOL)resizable
{
    isResizable = resizable;
}

- (BOOL)isResizable
{
    return isResizable;
}

- (void)setModal:(BOOL)modal
{
    isModal = modal;
}

- (BOOL)isModal
{
    return isModal;
}

- (void)draw
{
    MXTUIConsole *con = [MXTUIConsole sharedConsole];
    MXTUIRenderer *renderer = [MXTUIRenderer rendererWithConsole:con];
    
    if (hasBorder) {
        [renderer drawBox:frame title:title style:borderStyle];
    }
    
    /* Draw content children */
    int i;
    for (i = 0; i < contentChildCount; i++) {
        [contentChildren[i] display];
    }
    
    [con refresh];
}

- (TUIRect)contentRect
{
    TUIRect rect = frame;
    if (hasBorder) {
        rect.x += 1;
        rect.y += 1;
        rect.width -= 2;
        rect.height -= 2;
    }
    return rect;
}

- (void)addContentChild:(MXTUIWidget *)child
{
    if (contentChildCount == 0) {
        contentChildren = (MXTUIWidget **)malloc(8 * sizeof(MXTUIWidget *));
    }
    contentChildren[contentChildCount++] = child;
    
    TUIRect content = [self contentRect];
    child.frame.x = content.x;
    child.frame.y = content.y;
    
    [self setNeedsDisplay:YES];
}

- (void)removeContentChild:(MXTUIWidget *)child
{
    int i;
    for (i = 0; i < contentChildCount; i++) {
        if (contentChildren[i] == child) {
            int j;
            for (j = i; j < contentChildCount - 1; j++) {
                contentChildren[j] = contentChildren[j + 1];
            }
            contentChildCount--;
            break;
        }
    }
    [self setNeedsDisplay:YES];
}

- (void)moveBy:(int)dx dy:(int)dy
{
    if (!isMovable) return;
    
    MXTUIConsole *con = [MXTUIConsole sharedConsole];
    
    /* Clear old position */
    MXTUIRenderer *renderer = [MXTUIRenderer rendererWithConsole:con];
    [renderer fillRect:frame withChar:' '];
    
    /* Update position */
    frame.x += dx;
    frame.y += dy;
    
    /* Keep in bounds */
    if (frame.x < 0) frame.x = 0;
    if (frame.y < 0) frame.y = 0;
    if (frame.x + frame.width > con.screenWidth) {
        frame.x = con.screenWidth - frame.width;
    }
    if (frame.y + frame.height > con.screenHeight) {
        frame.y = con.screenHeight - frame.height;
    }
    
    [self setNeedsDisplay:YES];
}

@end

/* ========================================================================
 * MXTUIControl Implementation
 * ======================================================================== */

@implementation MXTUIControl

+ (id)new
{
    MXTUIControl *obj = [super new];
    if (obj) {
        obj->isEnabled = YES;
        obj->isHighlighted = NO;
        obj->action = NULL;
        obj->target = nil;
    }
    return obj;
}

- (void)setEnabled:(BOOL)enabled
{
    isEnabled = enabled;
    [self setNeedsDisplay:YES];
}

- (BOOL)isEnabled
{
    return isEnabled;
}

- (void)setHighlighted:(BOOL)highlighted
{
    isHighlighted = highlighted;
    [self setNeedsDisplay:YES];
}

- (BOOL)isHighlighted
{
    return isHighlighted;
}

- (void)setAction:(SEL)anAction
{
    action = anAction;
}

- (SEL)action
{
    return action;
}

- (void)setTarget:(id)anObject
{
    target = anObject;
}

- (id)target
{
    return target;
}

- (void)activate
{
    if (isEnabled && action && target) {
        if ([target respondsToSelector:action]) {
            [target performSelector:action withObject:self];
        }
    }
}

@end

/* ========================================================================
 * MXTUIButton Implementation
 * ======================================================================== */

@implementation MXTUIButton

+ (id)new
{
    MXTUIButton *obj = [super new];
    if (obj) {
        obj->title = NULL;
        obj->width = 0;
        obj->isDefault = NO;
    }
    return obj;
}

+ (id)buttonWithTitle:(const char *)aTitle
{
    MXTUIButton *btn = [self new];
    [btn setTitle:aTitle];
    return btn;
}

- (void)setTitle:(const char *)newTitle
{
    if (title) free((void *)title);
    title = newTitle ? strdup(newTitle) : NULL;
    [self setNeedsDisplay:YES];
}

- (const char *)title
{
    return title;
}

- (void)setDefault:(BOOL)def
{
    isDefault = def;
    [self setNeedsDisplay:YES];
}

- (BOOL)isDefault
{
    return isDefault;
}

- (void)draw
{
    if (!title) return;
    
    MXTUIConsole *con = [MXTUIConsole sharedConsole];
    MXTUIRenderer *renderer = [MXTUIRenderer rendererWithConsole:con];
    
    int w = width > 0 ? width : strlen(title) + 4;
    TUIPoint pos = TUIPointMake(frame.x, frame.y);
    
    [renderer drawButton:title at:pos width:w selected:isHighlighted];
}

@end

/* ========================================================================
 * MXTUITextField Implementation
 * ======================================================================== */

@implementation MXTUITextField

+ (id)new
{
    MXTUITextField *obj = [super new];
    if (obj) {
        obj->text = NULL;
        obj->textLength = 0;
        obj->maxLength = 256;
        obj->cursorPosition = 0;
        obj->scrollOffset = 0;
        obj->isPassword = NO;
        obj->passwordChar = '*';
    }
    return obj;
}

+ (id)textFieldWithFrame:(TUIRect)aFrame
{
    MXTUITextField *tf = [self new];
    [tf setFrame:aFrame];
    return tf;
}

- (void)setStringValue:(const char *)string
{
    if (text) free((void *)text);
    text = string ? strdup(string) : NULL;
    textLength = text ? strlen(text) : 0;
    if (cursorPosition > textLength) cursorPosition = textLength;
    [self setNeedsDisplay:YES];
}

- (const char *)stringValue
{
    return text;
}

- (void)setMaxLength:(int)max
{
    maxLength = max;
}

- (int)maxLength
{
    return maxLength;
}

- (void)setPassword:(BOOL)pwd
{
    isPassword = pwd;
    [self setNeedsDisplay:YES];
}

- (BOOL)isPassword
{
    return isPassword;
}

- (void)insertText:(const char *)newText
{
    if (!newText) return;
    int newLen = strlen(newText);
    if (textLength + newLen > maxLength) {
        newLen = maxLength - textLength;
    }
    if (newLen <= 0) return;
    
    int newTotal = textLength + newLen + 1;
    char *newStr = (char *)malloc(newTotal);
    
    if (text) {
        strncpy(newStr, text, cursorPosition);
        strncpy(newStr + cursorPosition, newText, newLen);
        strcpy(newStr + cursorPosition + newLen, text + cursorPosition);
        free((void *)text);
    } else {
        strncpy(newStr, newText, newLen);
        newStr[newLen] = '\0';
    }
    
    text = newStr;
    textLength += newLen;
    cursorPosition += newLen;
    [self setNeedsDisplay:YES];
}

- (void)deleteBackward
{
    if (cursorPosition <= 0 || !text) return;
    
    int i;
    for (i = cursorPosition - 1; i < textLength; i++) {
        text[i] = text[i + 1];
    }
    textLength--;
    cursorPosition--;
    [self setNeedsDisplay:YES];
}

- (void)draw
{
    MXTUIConsole *con = [MXTUIConsole sharedConsole];
    MXTUIRenderer *renderer = [MXTUIRenderer rendererWithConsole:con];
    
    const char *displayText = text;
    if (isPassword && text) {
        /* Show password characters */
        static char *masked = NULL;
        if (masked) free(masked);
        masked = (char *)malloc(textLength + 1);
        int i;
        for (i = 0; i < textLength; i++) {
            masked[i] = passwordChar;
        }
        masked[textLength] = '\0';
        displayText = masked;
    }
    
    TUIPoint pos = TUIPointMake(frame.x, frame.y);
    [renderer drawTextField:displayText at:pos width:frame.width hasFocus:isFocused];
}

@end

/* ========================================================================
 * MXTUIMenuBar Implementation
 * ======================================================================== */

@implementation MXTUIMenuBar

+ (id)new
{
    MXTUIMenuBar *obj = [super new];
    if (obj) {
        obj->menu = nil;
        obj->menuItems = NULL;
        obj->itemCount = 0;
        obj->selectedIndex = -1;
        obj->isOpen = NO;
        obj->openMenuIndex = -1;
    }
    return obj;
}

+ (id)menuBarWithMenu:(NXMenu *)aMenu
{
    MXTUIMenuBar *mb = [self new];
    [mb setMenu:aMenu];
    return mb;
}

- (void)setMenu:(NXMenu *)aMenu
{
    menu = aMenu;
    if (aMenu) {
        itemCount = [menu numberOfItems];
        menuItems = (NXMenuItem **)malloc(itemCount * sizeof(NXMenuItem *));
        int i;
        for (i = 0; i < itemCount; i++) {
            menuItems[i] = [menu itemAtIndex:i];
        }
    }
    [self setNeedsDisplay:YES];
}

- (NXMenu *)menu
{
    return menu;
}

- (void)draw
{
    MXTUIConsole *con = [MXTUIConsole sharedConsole];
    [con moveCursorTo:0 y:0];
    
    int x = 0;
    int i;
    for (i = 0; i < itemCount; i++) {
        const char *itemTitle = [menuItems[i] title];
        if (itemTitle) {
            if (i == selectedIndex || (isOpen && i == openMenuIndex)) {
                [con setAttributes:TUI_ATTR_REVERSE];
            }
            printf("%s", itemTitle);
            [con resetAttributes];
            printf("  ");
            x += strlen(itemTitle) + 2;
        }
    }
    
    /* Fill rest of line */
    while (x < con.screenWidth) {
        printf(" ");
        x++;
    }
    
    [con refresh];
}

- (int)selectedIndex
{
    return selectedIndex;
}

- (void)selectItemAtIndex:(int)index
{
    if (index >= 0 && index < itemCount) {
        selectedIndex = index;
        [self setNeedsDisplay:YES];
    }
}

- (void)openMenuAtIndex:(int)index
{
    if (index >= 0 && index < itemCount) {
        openMenuIndex = index;
        isOpen = YES;
        [self setNeedsDisplay:YES];
    }
}

- (void)closeMenu
{
    isOpen = NO;
    openMenuIndex = -1;
    [self setNeedsDisplay:YES];
}

- (BOOL)isMenuOpen
{
    return isOpen;
}

@end

/* ========================================================================
 * MXTUITextView Implementation
 * ======================================================================== */

@implementation MXTUITextView

+ (id)new
{
    MXTUITextView *obj = [super new];
    if (obj) {
        obj->text = NULL;
        obj->textLength = 0;
        obj->cursorLine = 0;
        obj->cursorCol = 0;
        obj->topLine = 0;
        obj->visibleLines = 10;
        obj->wordWrap = YES;
        obj->lines = NULL;
        obj->lineCount = 0;
    }
    return obj;
}

+ (id)textViewWithFrame:(TUIRect)aFrame
{
    MXTUITextView *tv = [self new];
    [tv setFrame:aFrame];
    tv->visibleLines = aFrame.height;
    return tv;
}

- (void)setString:(const char *)string
{
    if (text) free((void *)text);
    text = string ? strdup(string) : NULL;
    textLength = text ? strlen(text) : 0;
    
    /* Split into lines */
    if (lines) {
        int i;
        for (i = 0; i < lineCount; i++) {
            free((void *)lines[i]);
        }
        free(lines);
    }
    
    lineCount = 1;
    lines = (const char **)malloc(sizeof(const char *));
    lines[0] = text ? strdup(text) : strdup("");
    
    /* Count lines */
    if (text) {
        const char *p = text;
        lineCount = 1;
        while (*p) {
            if (*p == '\n') lineCount++;
            p++;
        }
        
        free((void *)lines[0]);
        lines = (const char **)realloc((void *)lines, lineCount * sizeof(const char *));
        
        /* Split */
        int lineIdx = 0;
        const char *lineStart = text;
        p = text;
        while (*p) {
            if (*p == '\n') {
                int len = p - lineStart;
                lines[lineIdx] = (const char *)malloc(len + 1);
                strncpy((char *)lines[lineIdx], lineStart, len);
                ((char *)lines[lineIdx])[len] = '\0';
                lineIdx++;
                lineStart = p + 1;
            }
            p++;
        }
        /* Last line */
        int len = p - lineStart;
        lines[lineIdx] = (const char *)malloc(len + 1);
        strncpy((char *)lines[lineIdx], lineStart, len);
        ((char *)lines[lineIdx])[len] = '\0';
    }
    
    [self setNeedsDisplay:YES];
}

- (const char *)string
{
    return text;
}

- (void)appendText:(const char *)newText
{
    if (!newText) return;
    
    int newLen = strlen(newText);
    int totalLen = textLength + newLen + 1;
    char *newStr = (char *)malloc(totalLen);
    
    if (text) {
        strcpy(newStr, text);
        free((void *)text);
    } else {
        newStr[0] = '\0';
    }
    strcat(newStr, newText);
    
    text = newStr;
    textLength += newLen;
    
    [self setString:text];
}

- (void)clear
{
    [self setString:""];
}

- (void)draw
{
    MXTUIConsole *con = [MXTUIConsole sharedConsole];
    
    int y;
    for (y = 0; y < visibleLines && y + topLine < lineCount; y++) {
        [con moveCursorTo:frame.x y:frame.y + y];
        const char *line = lines[topLine + y];
        if (line) {
            /* Truncate if needed */
            int maxW = frame.width;
            if (strlen(line) > (size_t)maxW) {
                int i;
                for (i = 0; i < maxW - 1; i++) {
                    printf("%c", line[i]);
                }
                printf(">");
            } else {
                printf("%s", line);
                /* Clear rest of line */
                int x;
                for (x = strlen(line); x < frame.width; x++) {
                    printf(" ");
                }
            }
        }
    }
    
    [con refresh];
}

- (void)scrollToLine:(int)line
{
    if (line < 0) line = 0;
    if (line > lineCount - visibleLines) {
        line = lineCount - visibleLines;
        if (line < 0) line = 0;
    }
    topLine = line;
    [self setNeedsDisplay:YES];
}

- (void)moveCursorUp
{
    if (cursorLine > 0) {
        cursorLine--;
        if (cursorLine < topLine) {
            [self scrollToLine:cursorLine];
        }
    }
}

- (void)moveCursorDown
{
    if (cursorLine < lineCount - 1) {
        cursorLine++;
        if (cursorLine >= topLine + visibleLines) {
            [self scrollToLine:topLine + 1];
        }
    }
}

- (void)moveCursorLeft
{
    if (cursorCol > 0) {
        cursorCol--;
    }
}

- (void)moveCursorRight
{
    cursorCol++;
}

@end

/* ========================================================================
 * MXTUIListView Implementation
 * ======================================================================== */

@implementation MXTUIListView

+ (id)new
{
    MXTUIListView *obj = [super new];
    if (obj) {
        obj->items = NULL;
        obj->itemCount = 0;
        obj->selectedIndex = -1;
        obj->visibleStart = 0;
        obj->visibleCount = 10;
        obj->showScrollbar = YES;
        obj->scrollPosition = 0;
    }
    return obj;
}

+ (id)listViewWithFrame:(TUIRect)aFrame
{
    MXTUIListView *lv = [self new];
    [lv setFrame:aFrame];
    lv->visibleCount = aFrame.height;
    return lv;
}

- (void)addItem:(const char *)item
{
    items = (const char **)realloc((void *)items, (itemCount + 1) * sizeof(const char *));
    items[itemCount++] = item ? strdup(item) : strdup("");
    
    if (selectedIndex < 0) selectedIndex = 0;
    [self setNeedsDisplay:YES];
}

- (void)removeItemAtIndex:(int)index
{
    if (index < 0 || index >= itemCount) return;
    
    free((void *)items[index]);
    int i;
    for (i = index; i < itemCount - 1; i++) {
        items[i] = items[i + 1];
    }
    itemCount--;
    
    if (selectedIndex >= itemCount) {
        selectedIndex = itemCount - 1;
        if (selectedIndex < 0) selectedIndex = -1;
    }
    [self setNeedsDisplay:YES];
}

- (void)clearItems
{
    int i;
    for (i = 0; i < itemCount; i++) {
        free((void *)items[i]);
    }
    free(items);
    items = NULL;
    itemCount = 0;
    selectedIndex = -1;
    [self setNeedsDisplay:YES];
}

- (int)selectedIndex
{
    return selectedIndex;
}

- (void)selectItemAtIndex:(int)index
{
    if (index >= 0 && index < itemCount) {
        selectedIndex = index;
        
        /* Update visible range */
        if (selectedIndex < visibleStart) {
            visibleStart = selectedIndex;
        } else if (selectedIndex >= visibleStart + visibleCount) {
            visibleStart = selectedIndex - visibleCount + 1;
        }
        
        /* Update scroll position */
        if (itemCount > visibleCount) {
            scrollPosition = (float)visibleStart / (itemCount - visibleCount);
        }
        
        [self setNeedsDisplay:YES];
    }
}

- (const char *)selectedItem
{
    if (selectedIndex >= 0 && selectedIndex < itemCount) {
        return items[selectedIndex];
    }
    return NULL;
}

- (void)draw
{
    MXTUIConsole *con = [MXTUIConsole sharedConsole];
    MXTUIRenderer *renderer = [MXTUIRenderer rendererWithConsole:con];
    
    int displayLines = (frame.height < itemCount) ? frame.height : itemCount;
    if (showScrollbar) displayLines--;
    
    int y;
    for (y = 0; y < displayLines; y++) {
        int itemIdx = visibleStart + y;
        if (itemIdx >= itemCount) break;
        
        [con moveCursorTo:frame.x y:frame.y + y];
        
        if (itemIdx == selectedIndex) {
            [con setAttributes:TUI_ATTR_REVERSE];
            printf(" %s", items[itemIdx]);
            [con resetAttributes];
            int x;
            for (x = strlen(items[itemIdx]) + 1; x < frame.width; x++) {
                printf(" ");
            }
        } else {
            printf(" %s", items[itemIdx]);
            int x;
            for (x = strlen(items[itemIdx]) + 1; x < frame.width; x++) {
                printf(" ");
            }
        }
    }
    
    /* Draw scrollbar */
    if (showScrollbar && itemCount > visibleCount) {
        [renderer drawScrollbar:frame.x + frame.width - 1 
                             y:frame.y 
                        height:frame.height 
                       position:scrollPosition 
                      thumbSize:(float)visibleCount / itemCount];
    }
    
    [con refresh];
}

- (void)scrollUp
{
    if (selectedIndex > 0) {
        [self selectItemAtIndex:selectedIndex - 1];
    }
}

- (void)scrollDown
{
    if (selectedIndex < itemCount - 1) {
        [self selectItemAtIndex:selectedIndex + 1];
    }
}

@end

/* ========================================================================
 * MXTUIAlertPanel Implementation
 * ======================================================================== */

@implementation MXTUIAlertPanel

+ (id)alertWithTitle:(const char *)title 
             message:(const char *)message 
      defaultButton:(const char *)defaultButton 
    alternateButton:(const char *)alternateButton 
         otherButton:(const char *)otherButton
{
    MXTUIAlertPanel *panel = [self new];
    [panel setTitle:title];
    [panel setMessageText:message];
    
    panel->defaultButtonTitle = defaultButton ? strdup(defaultButton) : NULL;
    panel->alternateButtonTitle = alternateButton ? strdup(alternateButton) : NULL;
    panel->otherButtonTitle = otherButton ? strdup(otherButton) : NULL;
    
    panel->buttonCount = 0;
    panel->buttons = NULL;
    panel->defaultButtonIndex = 0;
    
    if (defaultButton) panel->buttonCount++;
    if (alternateButton) panel->buttonCount++;
    if (otherButton) panel->buttonCount++;
    
    return panel;
}

- (void)setMessageText:(const char *)text
{
    if (messageText) free((void *)messageText);
    messageText = text ? strdup(text) : NULL;
}

- (const char *)messageText
{
    return messageText;
}

- (int)runModal
{
    MXTUIConsole *con = [MXTUIConsole sharedConsole];
    [self display];
    
    int result = 0;
    int currentButton = 0;
    
    while (1) {
        [self draw];
        [con refresh];
        
        int ch = getchar();
        if (ch == 27) { /* ESC */
            /* Check for arrow keys */
            int ch2 = getchar();
            if (ch2 == '[') {
                int ch3 = getchar();
                switch (ch3) {
                    case 'C': /* Right */
                        currentButton = (currentButton + 1) % buttonCount;
                        break;
                    case 'D': /* Left */
                        currentButton--;
                        if (currentButton < 0) currentButton = buttonCount - 1;
                        break;
                }
            }
        } else if (ch == '\n' || ch == '\r') {
            /* Enter - select current button */
            result = currentButton;
            break;
        } else if (ch == '1' && buttonCount > 0) {
            result = 0;
            break;
        } else if (ch == '2' && buttonCount > 1) {
            result = 1;
            break;
        } else if (ch == '3' && buttonCount > 2) {
            result = 2;
            break;
        } else if (ch == 'q' || ch == 'Q' || ch == 3) { /* Ctrl+C */
            result = -1;
            break;
        }
    }
    
    return result;
}

- (void)draw
{
    MXTUIWindow *win = (MXTUIWindow *)self;
    MXTUIConsole *con = [MXTUIConsole sharedConsole];
    MXTUIRenderer *renderer = [MXTUIRenderer rendererWithConsole:con];
    
    /* Draw window */
    TUIRect wrect = frame;
    [renderer drawBox:wrect title:title style:TUI_BORDER_SINGLE];
    
    /* Draw message */
    if (messageText) {
        TUIRect msgRect = TUIRectMake(wrect.x + 2, wrect.y + 2, wrect.width - 4, 1);
        [renderer drawText:messageText inRect:msgRect alignment:1];
    }
    
    /* Draw buttons */
    int btnX = wrect.x + 2;
    int btnY = wrect.y + wrect.height - 2;
    
    if (defaultButtonTitle) {
        [renderer drawButton:defaultButtonTitle at:TUIPointMake(btnX, btnY) 
                      width:0 selected:(defaultButtonIndex == 0)];
        btnX += strlen(defaultButtonTitle) + 4;
    }
    if (alternateButtonTitle) {
        [renderer drawButton:alternateButtonTitle at:TUIPointMake(btnX, btnY) 
                      width:0 selected:(defaultButtonIndex == 1)];
        btnX += strlen(alternateButtonTitle) + 4;
    }
    if (otherButtonTitle) {
        [renderer drawButton:otherButtonTitle at:TUIPointMake(btnX, btnY) 
                      width:0 selected:(defaultButtonIndex == 2)];
    }
}

- (void)buttonPressed:(int)buttonIndex
{
    /* Handle button press */
}

@end

/* ========================================================================
 * MXTUIEventLoop Implementation
 * ======================================================================== */

@implementation MXTUIEventLoop

+ (MXTUIEventLoop *)sharedEventLoop
{
    if (!_sharedEventLoop) {
        _sharedEventLoop = [self new];
    }
    return _sharedEventLoop;
}

+ (id)new
{
    MXTUIEventLoop *obj = [super new];
    if (obj) {
        obj->console = nil;
        obj->windows = NULL;
        obj->windowCount = 0;
        obj->windowCapacity = 0;
        obj->keyWindow = nil;
        obj->mainWindow = nil;
        obj->isRunning = NO;
        obj->mousePosition = TUIPointMake(0, 0);
        obj->mouseDown = NO;
    }
    return obj;
}

- (void)initialize
{
    console = [MXTUIConsole sharedConsole];
    [console initialize];
}

- (void)run
{
    isRunning = YES;
    
    while (isRunning) {
        [self processKeyboardInput];
        
        if (!isRunning) break;
        
        /* Redraw windows */
        MXTUIRefresh();
        
        /* Small delay to prevent CPU spinning */
        usleep(10000); /* 10ms */
    }
    
    [console shutdown];
}

- (void)stop
{
    isRunning = NO;
}

- (void)addWindow:(MXTUIWindow *)window
{
    if (windowCount >= windowCapacity) {
        windowCapacity = windowCapacity ? windowCapacity * 2 : 4;
        windows = (MXTUIWindow **)realloc(windows, windowCapacity * sizeof(MXTUIWindow *));
    }
    windows[windowCount++] = window;
    
    if (!keyWindow) {
        keyWindow = window;
    }
    if (!mainWindow) {
        mainWindow = window;
    }
}

- (void)removeWindow:(MXTUIWindow *)window
{
    int i;
    for (i = 0; i < windowCount; i++) {
        if (windows[i] == window) {
            int j;
            for (j = i; j < windowCount - 1; j++) {
                windows[j] = windows[j + 1];
            }
            windowCount--;
            break;
        }
    }
    
    if (keyWindow == window) {
        keyWindow = windowCount > 0 ? windows[0] : nil;
    }
    if (mainWindow == window) {
        mainWindow = windowCount > 0 ? windows[0] : nil;
    }
}

- (void)setKeyWindow:(MXTUIWindow *)window
{
    keyWindow = window;
}

- (MXTUIWindow *)keyWindow
{
    return keyWindow;
}

- (void)setMainWindow:(MXTUIWindow *)window
{
    mainWindow = window;
}

- (MXTUIWindow *)mainWindow
{
    return mainWindow;
}

- (void)processKeyboardInput
{
    /* Non-blocking keyboard input */
    fd_set readfds;
    struct timeval tv;
    
    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    
    int ret = select(STDIN_FILENO + 1, &readfds, NULL, NULL, &tv);
    
    if (ret > 0 && FD_ISSET(STDIN_FILENO, &readfds)) {
        int ch = getchar();
        
        if (ch == 27) { /* ESC sequence */
            int ch2 = getchar();
            if (ch2 == '[') {
                int ch3 = getchar();
                switch (ch3) {
                    case 'A': /* Up */
                        /* Handle up arrow */
                        break;
                    case 'B': /* Down */
                        /* Handle down arrow */
                        break;
                    case 'C': /* Right */
                        /* Handle right arrow */
                        break;
                    case 'D': /* Left */
                        /* Handle left arrow */
                        break;
                }
            }
        } else if (ch == 'q' || ch == 'Q') {
            [self stop];
        } else if (ch == 12) { /* Ctrl+L - refresh */
            [console clear];
        } else if (ch >= '0' && ch <= '9') {
            /* Number key for button selection */
        }
    }
}

- (void)processMouseInput
{
    /* Mouse input handling would go here for xterm mouse support */
}

- (void)handleResize
{
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0) {
        console.screenWidth = w.ws_col;
        console.screenHeight = w.ws_row;
        [self stop];
    }
}

@end

/* ========================================================================
 * Support Functions
 * ======================================================================== */

void MXTUIInitialize(void)
{
    _sharedConsole = [MXTUIConsole sharedConsole];
    _sharedEventLoop = [MXTUIEventLoop sharedEventLoop];
    [_sharedEventLoop initialize];
}

void MXTUIShutdown(void)
{
    if (_sharedConsole) {
        [_sharedConsole shutdown];
    }
}

MXTUIConsole *MXTUISharedConsole(void)
{
    return [MXTUIConsole sharedConsole];
}

MXTUIEventLoop *MXTUISharedEventLoop(void)
{
    return [MXTUIEventLoop sharedEventLoop];
}

void MXTUIRefresh(void)
{
    MXTUIConsole *con = [MXTUIConsole sharedConsole];
    MXTUIEventLoop *loop = [MXTUIEventLoop sharedEventLoop];
    
    if (!con || !loop) return;
    
    /* Clear screen */
    printf(TUI_CSI_CLEAR);
    printf(TUI_CSI_HOME);
    
    /* Draw all windows in order */
    int i;
    for (i = 0; i < loop.windowCount; i++) {
        [loop.windows[i] display];
    }
    
    [con refresh];
}

TUIRect MXTUICenteredRect(TUIRect windowRect, int screenW, int screenH)
{
    TUIRect result;
    result.width = windowRect.width;
    result.height = windowRect.height;
    result.x = (screenW - windowRect.width) / 2;
    result.y = (screenH - windowRect.height) / 2;
    if (result.x < 0) result.x = 0;
    if (result.y < 0) result.y = 0;
    return result;
}

TUIPoint MXTUICenteredPoint(int windowW, int windowH, int screenW, int screenH)
{
    TUIPoint p;
    p.x = (screenW - windowW) / 2;
    p.y = (screenH - windowH) / 2;
    if (p.x < 0) p.x = 0;
    if (p.y < 0) p.y = 0;
    return p;
}
