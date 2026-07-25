/*
 * appkit/terminal/tui.h - MinSTEP TUI (Text User Interface) Header
 *
 * Terminal-based UI rendering for AppKit applications.
 * Allows AppKit apps to run without a PostScript display server.
 *
 * Copyright (c) 2026 MinSTEP Project
 * Licensed under the MIT License.
 */

#ifndef _APPKIT_TERMINAL_TUI_H_
#define _APPKIT_TERMINAL_TUI_H_

#objc
#import <appkit/appkit.h>

/* ========================================================================
 * ANSI/TUI Color Definitions
 * ======================================================================== */

#define TUI_COLOR_BLACK         0
#define TUI_COLOR_RED           1
#define TUI_COLOR_GREEN         2
#define TUI_COLOR_YELLOW        3
#define TUI_COLOR_BLUE          4
#define TUI_COLOR_MAGENTA       5
#define TUI_COLOR_CYAN          6
#define TUI_COLOR_WHITE         7
#define TUI_COLOR_DEFAULT       9

#define TUI_COLOR_FG(base)     (30 + (base))
#define TUI_COLOR_BG(base)     (40 + (base))
#define TUI_COLOR_BRIGHT_FG(base) (90 + (base))
#define TUI_COLOR_BRIGHT_BG(base) (100 + (base))

/* Text attributes */
#define TUI_ATTR_NORMAL         0
#define TUI_ATTR_BOLD           1
#define TUI_ATTR_UNDERLINE      4
#define TUI_ATTR_REVERSE        7
#define TUI_ATTR_BLINK          5
#define TUI_ATTR_DIM            2

/* Border styles */
#define TUI_BORDER_NONE         0
#define TUI_BORDER_SINGLE       1
#define TUI_BORDER_DOUBLE       2
#define TUI_BORDER_ROUNDED      3
#define TUI_BORDER_ASCII        4

/* ========================================================================
 * Forward Declarations
 * ======================================================================== */

@class MXTUIConsole;
@class MXTUIRenderer;
@class MXTUIWindow;
@class MXTUIControl;
@class MXTUIButton;
@class MXTUITextField;
@class MXTUIMenuBar;

/* ========================================================================
 * TUI Point and Rect
 * ======================================================================== */

typedef struct _TUIPoint {
    int x;
    int y;
} TUIPoint;

typedef struct _TUIRect {
    int x;
    int y;
    int width;
    int height;
} TUIRect;

static inline TUIPoint TUIPointMake(int x, int y) {
    TUIPoint p; p.x = x; p.y = y; return p;
}

static inline TUIRect TUIRectMake(int x, int y, int w, int h) {
    TUIRect r; r.x = x; r.y = y; r.width = w; r.height = h; return r;
}

/* ========================================================================
 * MXTUIConsole - Main TUI Console Manager
 * ======================================================================== */

@interface MXTUIConsole : Object
{
    int screenWidth;
    int screenHeight;
    TUIPoint cursorPosition;
    BOOL cursorVisible;
    BOOL alternateScreen;
    BOOL rawMode;
    FILE *output;
    FILE *input;
}

+ (MXTUIConsole *)sharedConsole;

- (void)initialize;
- (void)shutdown;
- (void)clear;
- (void)refresh;

- (int)screenWidth;
- (int)screenHeight;
- (void)moveCursorTo:(int)x y:(int)y;
- (void)hideCursor;
- (void)showCursor;

- (void)enableAlternateScreen:(BOOL)enable;
- (void)enableRawMode:(BOOL)enable;

- (void)setForegroundColor:(int)fg backgroundColor:(int)bg;
- (void)setAttributes:(int)attrs;
- (void)resetAttributes;

- (void)saveState;
- (void)restoreState;

- (void)beep;

@end

/* ========================================================================
 * MXTUIRenderer - Draws TUI Elements
 * ======================================================================== */

@interface MXTUIRenderer : Object
{
    MXTUIConsole *console;
}

+ (MXTUIRenderer *)rendererWithConsole:(MXTUIConsole *)console;

- (void)drawBox:(TUIRect)rect title:(const char *)title style:(int)style;
- (void)drawLineFrom:(TUIPoint)from to:(TUIPoint)to;
- (void)drawText:(const char *)text at:(TUIPoint)pos;
- (void)drawText:(const char *)text inRect:(TUIRect)rect alignment:(int)align;
- (void)drawChar:(char)c at:(TUIPoint)pos;
- (void)fillRect:(TUIRect)rect withChar:(char)ch;
- (void)drawSeparator:(int)y x1:(int)x1 x2:(int)x2;

- (void)drawButton:(const char *)title at:(TUIPoint)pos 
           width:(int)width selected:(BOOL)selected;
- (void)drawTextField:(const char *)text at:(TUIPoint)pos 
              width:(int)width hasFocus:(BOOL)focus;
- (void)drawScrollbar:(int)x y:(int)y height:(int)height 
              position:(float)pos thumbSize:(float)thumbSize;
- (void)drawCheckbox:(const char *)label at:(TUIPoint)pos checked:(BOOL)checked;
- (void)drawProgressBar:(TUIRect)rect progress:(float)progress;

@end

/* ========================================================================
 * MXTUIWidget - Base TUI Widget
 * ======================================================================== */

@interface MXTUIWidget : Object
{
    TUIRect frame;
    TUIRect bounds;
    MXTUIWidget *parent;
    MXTUIWidget **children;
    int childCount;
    int childCapacity;
    BOOL needsDisplay;
    BOOL isFocused;
    int foregroundColor;
    int backgroundColor;
    int borderStyle;
}

- (void)setFrame:(TUIRect)newFrame;
- (TUIRect)frame;
- (void)setBounds:(TUIRect)newBounds;
- (TUIRect)bounds;
- (void)addChild:(MXTUIWidget *)child;
- (void)removeChild:(MXTUIWidget *)child;
- (void)display;
- (void)displayInRect:(TUIRect)clipRect;
- (void)draw;
- (BOOL)isFocused;
- (void)setFocused:(BOOL)focused;
- (BOOL)needsDisplay;
- (void)setNeedsDisplay:(BOOL)needs;
- (BOOL)containsPoint:(TUIPoint)point;
- (void)moveTo:(int)x y:(int)y;
- (void)resizeTo:(int)width height:(int)height;

@end

/* ========================================================================
 * MXTUIWindow - TUI Window Widget
 * ======================================================================== */

@interface MXTUIWindow : MXTUIWidget
{
    const char *title;
    const char **titleBar;
    int titleBarHeight;
    MXTUIWidget **contentChildren;
    int contentChildCount;
    BOOL hasBorder;
    BOOL isModal;
    BOOL isMovable;
    BOOL isResizable;
    TUIRect savedFrame;
}

+ (MXTUIWindow *)new;
+ (MXTUIWindow *)windowWithTitle:(const char *)title frame:(TUIRect)frame;
- (void)setTitle:(const char *)newTitle;
- (const char *)title;
- (void)setHasBorder:(BOOL)hasBorder;
- (BOOL)hasBorder;
- (void)setMovable:(BOOL)movable;
- (BOOL)isMovable;
- (void)setResizable:(BOOL)resizable;
- (BOOL)isResizable;
- (void)setModal:(BOOL)modal;
- (BOOL)isModal;
- (void)draw;
- (TUIRect)contentRect;
- (void)addContentChild:(MXTUIWidget *)child;
- (void)removeContentChild:(MXTUIWidget *)child;
- (void)moveBy:(int)dx dy:(int)dy;

@end

/* ========================================================================
 * MXTUIControl - Base TUI Control
 * ======================================================================== */

@interface MXTUIControl : MXTUIWidget
{
    BOOL isEnabled;
    BOOL isHighlighted;
    SEL action;
    id target;
}

- (void)setEnabled:(BOOL)enabled;
- (BOOL)isEnabled;
- (void)setHighlighted:(BOOL)highlighted;
- (BOOL)isHighlighted;
- (void)setAction:(SEL)action;
- (SEL)action;
- (void)setTarget:(id)target;
- (id)target;
- (void)activate;

@end

/* ========================================================================
 * MXTUIButton - TUI Button Control
 * ======================================================================== */

@interface MXTUIButton : MXTUIControl
{
    const char *title;
    int width;
    BOOL isDefault;
}

+ (MXTUIButton *)new;
+ (MXTUIButton *)buttonWithTitle:(const char *)title;
- (void)setTitle:(const char *)newTitle;
- (const char *)title;
- (void)setDefault:(BOOL)isDefault;
- (BOOL)isDefault;
- (void)draw;

@end

/* ========================================================================
 * MXTUITextField - TUI Text Field Control
 * ======================================================================== */

@interface MXTUITextField : MXTUIControl
{
    const char *text;
    int textLength;
    int maxLength;
    int cursorPosition;
    int scrollOffset;
    BOOL isPassword;
    char passwordChar;
}

+ (MXTUITextField *)new;
+ (MXTUITextField *)textFieldWithFrame:(TUIRect)frame;
- (void)setStringValue:(const char *)string;
- (const char *)stringValue;
- (void)setMaxLength:(int)max;
- (int)maxLength;
- (void)setPassword:(BOOL)isPassword;
- (BOOL)isPassword;
- (void)insertText:(const char *)text;
- (void)deleteBackward;
- (void)draw;

@end

/* ========================================================================
 * MXTUIMenuBar - TUI Menu Bar
 * ======================================================================== */

@interface MXTUIMenuBar : MXTUIWidget
{
    NXMenu *menu;
    NXMenuItem **menuItems;
    int itemCount;
    int selectedIndex;
    BOOL isOpen;
    int openMenuIndex;
    TUIRect popupRect;
}

+ (MXTUIMenuBar *)new;
+ (MXTUIMenuBar *)menuBarWithMenu:(NXMenu *)aMenu;
- (void)setMenu:(NXMenu *)aMenu;
- (NXMenu *)menu;
- (void)draw;
- (int)selectedIndex;
- (void)selectItemAtIndex:(int)index;
- (void)openMenuAtIndex:(int)index;
- (void)closeMenu;
- (BOOL)isMenuOpen;

@end

/* ========================================================================
 * MXTUITextView - TUI Text View (multiline)
 * ======================================================================== */

@interface MXTUITextView : MXTUIControl
{
    const char *text;
    int textLength;
    int cursorLine;
    int cursorCol;
    int topLine;
    int visibleLines;
    BOOL wordWrap;
    const char **lines;
    int lineCount;
}

+ (MXTUITextView *)new;
+ (MXTUITextView *)textViewWithFrame:(TUIRect)frame;
- (void)setString:(const char *)string;
- (const char *)string;
- (void)appendText:(const char *)text;
- (void)clear;
- (void)draw;
- (void)scrollToLine:(int)line;
- (void)moveCursorUp;
- (void)moveCursorDown;
- (void)moveCursorLeft;
- (void)moveCursorRight;

@end

/* ========================================================================
 * MXTUIListView - TUI List/Table View
 * ======================================================================== */

@interface MXTUIListView : MXTUIControl
{
    const char **items;
    int itemCount;
    int selectedIndex;
    int visibleStart;
    int visibleCount;
    BOOL showScrollbar;
    float scrollPosition;
}

+ (MXTUIListView *)new;
+ (MXTUIListView *)listViewWithFrame:(TUIRect)frame;
- (void)addItem:(const char *)item;
- (void)removeItemAtIndex:(int)index;
- (void)clearItems;
- (int)selectedIndex;
- (void)selectItemAtIndex:(int)index;
- (const char *)selectedItem;
- (void)draw;
- (void)scrollUp;
- (void)scrollDown;

@end

/* ========================================================================
 * MXTUIAlertPanel - TUI Alert Dialog
 * ======================================================================== */

@interface MXTUIAlertPanel : MXTUIWindow
{
    const char *messageText;
    const char *defaultButtonTitle;
    const char *alternateButtonTitle;
    const char *otherButtonTitle;
    MXTUIButton **buttons;
    int buttonCount;
    int defaultButtonIndex;
}

+ (MXTUIAlertPanel *)alertWithTitle:(const char *)title 
                             message:(const char *)message 
                      defaultButton:(const char *)defaultButton 
                    alternateButton:(const char *)alternateButton 
                         otherButton:(const char *)otherButton;

- (void)setMessageText:(const char *)text;
- (const char *)messageText;
- (int)runModal;
- (void)draw;
- (void)buttonPressed:(int)buttonIndex;

@end

/* ========================================================================
 * MXTUIEventLoop - TUI Event Loop
 * ======================================================================== */

@interface MXTUIEventLoop : Object
{
    MXTUIConsole *console;
    MXTUIWindow **windows;
    int windowCount;
    int windowCapacity;
    MXTUIWindow *keyWindow;
    MXTUIWindow *mainWindow;
    BOOL isRunning;
    TUIPoint mousePosition;
    BOOL mouseDown;
}

+ (MXTUIEventLoop *)sharedEventLoop;

- (void)initialize;
- (void)run;
- (void)stop;
- (void)addWindow:(MXTUIWindow *)window;
- (void)removeWindow:(MXTUIWindow *)window;
- (void)setKeyWindow:(MXTUIWindow *)window;
- (MXTUIWindow *)keyWindow;
- (void)setMainWindow:(MXTUIWindow *)window;
- (MXTUIWindow *)mainWindow;
- (void)processKeyboardInput;
- (void)processMouseInput;
- (void)handleResize;

@end

/* ========================================================================
 * TUI Support Functions
 * ======================================================================== */

/* Initialize TUI system */
void MXTUIInitialize(void);

/* Shutdown TUI system */
void MXTUIShutdown(void);

/* Get shared console */
MXTUIConsole *MXTUISharedConsole(void);

/* Get shared event loop */
MXTUIEventLoop *MXTUISharedEventLoop(void);

/* Draw all dirty windows */
void MXTUIRefresh(void);

/* Center window on screen */
TUIRect MXTUICenteredRect(TUIRect windowRect, int screenW, int screenH);

/* Calculate window position for centering */
TUIPoint MXTUICenteredPoint(int windowW, int windowH, int screenW, int screenH);

#endif /* _APPKIT_TERMINAL_TUI_H_ */
