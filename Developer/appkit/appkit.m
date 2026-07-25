/*
 * appkit/appkit.m - MinSTEP AppKit Implementation
 *
 * Implementation of core AppKit classes for MinSTEP.
 * Includes NXApplication, NXWindow, NXView, NXControl, NXButton,
 * NXText, NXScrollView, NXMenu, NXMenuItem, NXOpenPanel, NXSavePanel,
 * and related classes.
 *
 * Copyright (c) 2026 MinSTEP Project
 * Licensed under the MIT License.
 */

#objc
#import "appkit.h"

/* ========================================================================
 * NXRect - Rectangle Type
 * ======================================================================== */

typedef struct _NXRect {
    float origin_x;
    float origin_y;
    float size_width;
    float size_height;
} NXRect;

typedef struct _NXPoint {
    float x;
    float y;
} NXPoint;

typedef struct _NXSize {
    float width;
    float height;
} NXSize;

/* NXRect convenience functions */
static inline NXRect NXMakeRect(float x, float y, float w, float h) {
    NXRect r;
    r.origin_x = x;
    r.origin_y = y;
    r.size_width = w;
    r.size_height = h;
    return r;
}

static inline NXPoint NXMakePoint(float x, float y) {
    NXPoint p;
    p.x = x;
    p.y = y;
    return p;
}

static inline NXSize NXMakeSize(float w, float h) {
    NXSize s;
    s.width = w;
    s.height = h;
    return s;
}

#define NXWidth(r) ((r).size_width)
#define NXHeight(r) ((r).size_height)
#define NXOriginX(r) ((r).origin_x)
#define NXOriginY(r) ((r).origin_y)

/* ========================================================================
 * NXApplication - Application Class
 * ======================================================================== */

@interface NXApplication : Object
{
    NXMenu *mainMenu;
    NXWindow *keyWindow;
    NXWindow *mainWindow;
    NXWindow **windowList;
    int windowCount;
    int windowCapacity;
    id delegate;
    BOOL isRunning;
    const char *appName;
}

+ (NXApplication *)sharedApplication;
+ (id)new;

- (NXMenu *)mainMenu;
- (void)setMainMenu:(NXMenu *)menu;
- (NXWindow *)keyWindow;
- (NXWindow *)mainWindow;
- (void)setMainWindow:(NXWindow *)window;
- (id)delegate;
- (void)setDelegate:(id)delegate;
- (void)run;
- (void)terminate:(id)sender;
- (void)stop:(id)sender;
- (void)loadNibFile:(const char *)nibPath;
- (void)loadNibFile:(const char *)nibPath withOwner:(id)owner;
- (void)addWindow:(NXWindow *)window;
- (void)removeWindow:(NXWindow *)window;
- (void)sendAction:(SEL)action to:(id)target from:(id)sender;
- (id)targetForAction:(SEL)action;

@end

/* Global NXApp pointer */
id NXApp = nil;

/* ========================================================================
 * NXView - Base View Class
 * ======================================================================== */

@interface NXView : Object
{
    NXRect frame;
    NXRect bounds;
    NXView *superview;
    NXView **subviews;
    int subviewCount;
    int subviewCapacity;
    id target;
    SEL action;
    unsigned char autoresizingMask;
}

- (void)resize:(NXRect)newFrame;
- (void)moveTo:(float)x :(float)y;
- (void)sizeTo:(float)w :(float)h;
- (void)setFrame:(NXRect)newFrame;
- (NXRect)frame;
- (void)setBounds:(NXRect)newBounds;
- (NXRect)bounds;
- (void)addSubview:(NXView *)aView;
- (void)removeFromSuperview;
- (void)removeSubview:(NXView *)aView;
- (void)drawRect:(NXRect)rect;
- (void)display;
- (void)displayRect:(NXRect)rect;
- (id)superview;
- (NXView **)subviews;
- (int)subviewCount;
- (void)setTarget:(id)anObject;
- (id)target;
- (void)setAction:(SEL)aSelector;
- (SEL)action;
- (void)setAutoresizingMask:(unsigned char)mask;
- (unsigned char)autoresizingMask;

@end

/* ========================================================================
 * NXControl - Control Base Class
 * ======================================================================== */

@interface NXControl : NXView
{
    int cellFlags;
    int cellType;
    int controlState;
    NXView *cell;
}

- (void)setEnabled:(BOOL)flag;
- (BOOL)isEnabled;
- (void)setState:(int)state;
- (int)state;
- (void)setCell:(NXView *)cell;
- (NXView *)cell;

@end

/* ========================================================================
 * NXButton - Button Control
 * ======================================================================== */

@interface NXButton : NXControl
{
    const char *title;
    const char *keyEquivalent;
    int buttonType;
}

+ (NXButton *)new;
+ (NXButton *)buttonWithTitle:(const char *)aTitle;
- (void)setTitle:(const char *)aTitle;
- (const char *)title;
- (void)setKeyEquivalent:(const char *)key;
- (const char *)keyEquivalent;
- (void)setButtonType:(int)type;
- (int)buttonType;
- (void)setImage:(id)image;
- (id)image;

@end

/* ========================================================================
 * NXText - Text View
 * ======================================================================== */

@interface NXText : NXView
{
    const char *textStorage;
    int textLength;
    int textStyle;
    id delegate;
    BOOL isEditable;
    BOOL isSelectable;
    BOOL isRichText;
}

+ (NXText *)new;
+ (NXText *)newWithFrame:(NXRect)frame;
- (void)setString:(const char *)string;
- (const char *)string;
- (void)setEditable:(BOOL)flag;
- (BOOL)isEditable;
- (void)setSelectable:(BOOL)flag;
- (BOOL)isSelectable;
- (BOOL)writeRTFToFile:(const char *)path;
- (BOOL)readRTFFromFile:(const char *)path;
- (void)setRichText:(BOOL)flag;
- (BOOL)isRichText;
- (void)selectAll:(id)sender;
- (void)copy:(id)sender;
- (void)cut:(id)sender;
- (void)paste:(id)sender;

@end

/* ========================================================================
 * NXScrollView - Scroll View Container
 * ======================================================================== */

@interface NXScrollView : NXView
{
    NXView *documentView;
    NXView *horizontalScroller;
    NXView *verticalScroller;
    NXRect contentSize;
    NXPoint documentOrigin;
    BOOL hasHorizontalScroller;
    BOOL hasVerticalScroller;
    BOOL hasScroller;
    float scrollerWidth;
}

+ (NXScrollView *)newWithFrame:(NXRect)frame;
- (void)setDocumentView:(NXView *)aView;
- (NXView *)documentView;
- (void)setHasHorizontalScroller:(BOOL)flag;
- (BOOL)hasHorizontalScroller;
- (void)setHasVerticalScroller:(BOOL)flag;
- (BOOL)hasVerticalScroller;
- (void)setHasScroller:(BOOL)flag;
- (BOOL)hasScroller;
- (void)scrollToPoint:(NXPoint)point;
- (NXPoint)documentVisibleOrigin;
- (void)tile;

@end

/* ========================================================================
 * NXWindow - Window Class
 * ======================================================================== */

@interface NXWindow : NXView
{
    NXView *contentView;
    const char *title;
    NXRect windowRect;
    int styleMask;
    NXWindow **childWindows;
    int childCount;
    NXWindow *parentWindow;
    BOOL isVisible;
    BOOL isMiniaturized;
    BOOL isReleasedWhenClosed;
    id delegate;
}

+ (NXWindow *)new;
+ (NXWindow *)newWithContentRect:(NXRect)contentRect styleMask:(int)styleMask;
- (void)setContentView:(NXView *)aView;
- (NXView *)contentView;
- (void)setTitle:(const char *)aTitle;
- (const char *)title;
- (void)makeKeyAndOrderFront:(id)sender;
- (void)orderOut:(id)sender;
- (void)orderWindow:(int)place;
- (void)close;
- (void)miniaturize:(id)sender;
- (void)deminiaturize:(id)sender;
- (BOOL)isVisible;
- (BOOL)isMiniaturized;
- (void)setReleasedWhenClosed:(BOOL)flag;
- (BOOL)isReleasedWhenClosed;
- (void)setDelegate:(id)delegate;
- (id)delegate;
- (void)addChildWindow:(NXWindow *)childWindow;
- (void)removeChildWindow:(NXWindow *)childWindow;
- (void)setStyleMask:(int)mask;
- (int)styleMask;

@end

/* ========================================================================
 * NXMenu - Menu Class
 * ======================================================================== */

@interface NXMenu : Object
{
    const char *title;
    NXMenuItem **items;
    int itemCount;
    int itemCapacity;
    NXMenu *submenu;
    NXMenu *superMenu;
}

+ (NXMenu *)new;
+ (NXMenu *)menuWithTitle:(const char *)aTitle;
- (void)addItem:(NXMenuItem *)anItem;
- (void)addItemWithTitle:(const char *)aTitle action:(SEL)action keyEquivalent:(const char *)key;
- (NXMenuItem *)addItemSeparator;
- (void)insertItem:(NXMenuItem *)anItem atIndex:(int)index;
- (void)removeItemAtIndex:(int)index;
- (void)removeItem:(NXMenuItem *)anItem;
- (NXMenuItem *)itemAtIndex:(int)index;
- (int)numberOfItems;
- (void)setSubmenu:(NXMenu *)aMenu forItem:(NXMenuItem *)anItem;
- (NXMenu *)submenu;
- (void)addSubmenu:(NXMenu *)submenu;
- (NXMenu *)supermenu;
- (void)setTitle:(const char *)aTitle;
- (const char *)title;

@end

/* ========================================================================
 * NXMenuItem - Menu Item Class
 * ======================================================================== */

@interface NXMenuItem : Object
{
    const char *title;
    SEL action;
    id target;
    const char *keyEquivalent;
    int keyEquivalentModifierMask;
    NXMenu *submenu;
    int tag;
    BOOL isEnabled;
    int itemType;
    id representedObject;
}

+ (NXMenuItem *)new;
+ (NXMenuItem *)itemWithTitle:(const char *)aTitle action:(SEL)action keyEquivalent:(const char *)key;
- (void)setTitle:(const char *)aTitle;
- (const char *)title;
- (void)setAction:(SEL)anAction;
- (SEL)action;
- (void)setTarget:(id)anObject;
- (id)target;
- (void)setKeyEquivalent:(const char *)key;
- (const char *)keyEquivalent;
- (void)setKeyEquivalentModifierMask:(int)mask;
- (int)keyEquivalentModifierMask;
- (void)setSubmenu:(NXMenu *)aMenu;
- (NXMenu *)submenu;
- (void)setEnabled:(BOOL)flag;
- (BOOL)isEnabled;
- (void)setTag:(int)aTag;
- (int)tag;
- (void)setType:(int)aType;
- (int)type;
- (void)setRepresentedObject:(id)object;
- (id)representedObject;

@end

/* ========================================================================
 * NXOpenPanel - Open File Panel
 * ======================================================================== */

@interface NXOpenPanel : Object
{
    const char *directory;
    const char *filename;
    const char **allowedFileTypes;
    int allowedFileTypeCount;
    BOOL canChooseFiles;
    BOOL canChooseDirectories;
    BOOL allowsMultipleSelection;
    id delegate;
}

+ (NXOpenPanel *)openPanel;
+ (id)new;
- (int)runModal;
- (int)runModalForDirectory:(const char *)dir file:(const char *)file types:(const char **)types;
- (void)setDirectory:(const char *)path;
- (const char *)directory;
- (void)setFilename:(const char *)name;
- (const char *)filename;
- (void)setAllowedFileTypes:(const char **)types count:(int)count;
- (const char **)allowedFileTypes;
- (int)allowedFileTypeCount;
- (void)setCanChooseFiles:(BOOL)flag;
- (BOOL)canChooseFiles;
- (void)setCanChooseDirectories:(BOOL)flag;
- (BOOL)canChooseDirectories;
- (void)setAllowsMultipleSelection:(BOOL)flag;
- (BOOL)allowsMultipleSelection;
- (void)setDelegate:(id)delegate;
- (id)delegate;

@end

/* ========================================================================
 * NXSavePanel - Save File Panel
 * ======================================================================== */

@interface NXSavePanel : Object
{
    const char *directory;
    const char *filename;
    const char *requiredFileType;
    const char *message;
    id delegate;
    BOOL canCreateDirectories;
    BOOL isExpanded;
}

+ (NXSavePanel *)savePanel;
+ (id)new;
- (int)runModal;
- (int)runModalForDirectory:(const char *)dir file:(const char *)file;
- (void)setDirectory:(const char *)path;
- (const char *)directory;
- (void)setFilename:(const char *)name;
- (const char *)filename;
- (void)setRequiredFileType:(const char *)type;
- (const char *)requiredFileType;
- (void)setMessage:(const char *)message;
- (const char *)message;
- (void)setCanCreateDirectories:(BOOL)flag;
- (BOOL)canCreateDirectories;
- (void)setExpanded:(BOOL)flag;
- (BOOL)isExpanded;
- (void)setDelegate:(id)delegate;
- (id)delegate;

@end

/* ========================================================================
 * NXAlertPanel - Alert Panel
 * ======================================================================== */

@interface NXAlertPanel : NXWindow
{
    NXView *messageTextView;
    NXButton *defaultButton;
    NXButton *alternateButton;
    NXButton *otherButton;
    int alertStyle;
}

+ (NXAlertPanel *)alertPanelWithTitle:(const char *)title 
                              message:(const char *)message 
                       defaultButton:(const char *)defaultButton 
                     alternateButton:(const char *)alternateButton 
                          otherButton:(const char *)otherButton;

- (void)setAlertStyle:(int)style;
- (int)alertStyle;
- (void)setMessageText:(const char *)text;
- (const char *)messageText;
- (void)setDefaultButtonTitle:(const char *)title;
- (const char *)defaultButtonTitle;
- (void)setAlternateButtonTitle:(const char *)title;
- (const char *)alternateButtonTitle;
- (void)setOtherButtonTitle:(const char *)title;
- (const char *)otherButtonTitle;

@end

/* ========================================================================
 * MXApplication - MinSTEP Application Class
 * ======================================================================== */

@interface MXApplication : NXApplication
{
    const char *appBundlePath;
    id appDelegate;
}

+ (MXApplication *)new;
+ (id)sharedApplication;
- (void)run;
- (void)terminate:(id)sender;
- (void)loadNibFile:(const char *)nibPath;
- (void)loadNibFile:(const char *)nibPath withOwner:(id)owner;
- (void)setAppDelegate:(id)delegate;
- (id)appDelegate;
- (const char *)appBundlePath;

@end

/* ========================================================================
 * NXApplication Implementation
 * ======================================================================== */

@implementation NXApplication

+ (NXApplication *)sharedApplication
{
    if (!NXApp) {
        NXApp = [self new];
    }
    return NXApp;
}

+ (id)new
{
    NXApplication *obj = [super new];
    if (obj) {
        obj->mainMenu = nil;
        obj->keyWindow = nil;
        obj->mainWindow = nil;
        obj->windowList = NULL;
        obj->windowCount = 0;
        obj->windowCapacity = 0;
        obj->delegate = nil;
        obj->isRunning = NO;
        obj->appName = "Application";
    }
    return obj;
}

- (NXMenu *)mainMenu
{
    return mainMenu;
}

- (void)setMainMenu:(NXMenu *)menu
{
    mainMenu = menu;
}

- (NXWindow *)keyWindow
{
    return keyWindow;
}

- (NXWindow *)mainWindow
{
    return mainWindow;
}

- (void)setMainWindow:(NXWindow *)window
{
    mainWindow = window;
}

- (id)delegate
{
    return delegate;
}

- (void)setDelegate:(id)anObject
{
    delegate = anObject;
}

- (void)run
{
    isRunning = YES;
    /* Main event loop would be implemented here */
    while (isRunning) {
        /* Process events - simplified for now */
    }
}

- (void)stop:(id)sender
{
    isRunning = NO;
}

- (void)terminate:(id)sender
{
    isRunning = NO;
}

- (void)loadNibFile:(const char *)nibPath
{
    [self loadNibFile:nibPath withOwner:self];
}

- (void)loadNibFile:(const char *)nibPath withOwner:(id)owner
{
    /* NIB loading would be implemented here using MXNibBuilder */
    /* For now, this is a placeholder */
}

- (void)addWindow:(NXWindow *)window
{
    if (windowCount >= windowCapacity) {
        windowCapacity = windowCapacity ? windowCapacity * 2 : 4;
        windowList = (NXWindow **)realloc(windowList, windowCapacity * sizeof(NXWindow *));
    }
    windowList[windowCount++] = window;
}

- (void)removeWindow:(NXWindow *)window
{
    int i;
    for (i = 0; i < windowCount; i++) {
        if (windowList[i] == window) {
            int j;
            for (j = i; j < windowCount - 1; j++) {
                windowList[j] = windowList[j + 1];
            }
            windowCount--;
            break;
        }
    }
}

- (void)sendAction:(SEL)action to:(id)target from:(id)sender
{
    if (!target) {
        target = [self targetForAction:action];
    }
    if (target && [target respondsToSelector:action]) {
        [target performSelector:action withObject:sender];
    }
}

- (id)targetForAction:(SEL)action
{
    /* First responder chain lookup would go here */
    return nil;
}

@end

/* ========================================================================
 * NXView Implementation
 * ======================================================================== */

@implementation NXView

+ (id)new
{
    NXView *obj = [super new];
    if (obj) {
        obj->frame = NXMakeRect(0, 0, 100, 100);
        obj->bounds = obj->frame;
        obj->superview = nil;
        obj->subviews = NULL;
        obj->subviewCount = 0;
        obj->subviewCapacity = 0;
        obj->target = nil;
        obj->action = NULL;
        obj->autoresizingMask = NX_NOTSIZABLE;
    }
    return obj;
}

- (void)resize:(NXRect)newFrame
{
    [self setFrame:newFrame];
}

- (void)moveTo:(float)x :(float)y
{
    frame.origin_x = x;
    frame.origin_y = y;
}

- (void)sizeTo:(float)w :(float)h
{
    frame.size_width = w;
    frame.size_height = h;
}

- (void)setFrame:(NXRect)newFrame
{
    frame = newFrame;
    bounds.origin_x = 0;
    bounds.origin_y = 0;
    bounds.size_width = newFrame.size_width;
    bounds.size_height = newFrame.size_height;
}

- (NXRect)frame
{
    return frame;
}

- (void)setBounds:(NXRect)newBounds
{
    bounds = newBounds;
}

- (NXRect)bounds
{
    return bounds;
}

- (void)addSubview:(NXView *)aView
{
    if (subviewCount >= subviewCapacity) {
        subviewCapacity = subviewCapacity ? subviewCapacity * 2 : 4;
        subviews = (NXView **)realloc(subviews, subviewCapacity * sizeof(NXView *));
    }
    subviews[subviewCount++] = aView;
    aView->superview = self;
}

- (void)removeFromSuperview
{
    if (superview) {
        int i;
        for (i = 0; i < superview->subviewCount; i++) {
            if (superview->subviews[i] == self) {
                int j;
                for (j = i; j < superview->subviewCount - 1; j++) {
                    superview->subviews[j] = superview->subviews[j + 1];
                }
                superview->subviewCount--;
                break;
            }
        }
        superview = nil;
    }
}

- (void)removeSubview:(NXView *)aView
{
    int i;
    for (i = 0; i < subviewCount; i++) {
        if (subviews[i] == aView) {
            int j;
            for (j = i; j < subviewCount - 1; j++) {
                subviews[j] = subviews[j + 1];
            }
            subviewCount--;
            aView->superview = nil;
            break;
        }
    }
}

- (void)drawRect:(NXRect)rect
{
    /* Subclasses override this */
}

- (void)display
{
    [self displayRect:frame];
}

- (void)displayRect:(NXRect)rect
{
    [self drawRect:rect];
}

- (id)superview
{
    return superview;
}

- (NXView **)subviews
{
    return subviews;
}

- (int)subviewCount
{
    return subviewCount;
}

- (void)setTarget:(id)anObject
{
    target = anObject;
}

- (id)target
{
    return target;
}

- (void)setAction:(SEL)aSelector
{
    action = aSelector;
}

- (SEL)action
{
    return action;
}

- (void)setAutoresizingMask:(unsigned char)mask
{
    autoresizingMask = mask;
}

- (unsigned char)autoresizingMask
{
    return autoresizingMask;
}

@end

/* ========================================================================
 * NXControl Implementation
 * ======================================================================== */

@implementation NXControl

- (void)setEnabled:(BOOL)flag
{
    if (flag) {
        cellFlags &= ~0x01;
    } else {
        cellFlags |= 0x01;
    }
}

- (BOOL)isEnabled
{
    return !(cellFlags & 0x01);
}

- (void)setState:(int)state
{
    controlState = state;
}

- (int)state
{
    return controlState;
}

- (void)setCell:(NXView *)cellObj
{
    cell = cellObj;
}

- (NXView *)cell
{
    return cell;
}

@end

/* ========================================================================
 * NXButton Implementation
 * ======================================================================== */

@implementation NXButton

+ (id)new
{
    NXButton *obj = [super new];
    if (obj) {
        obj->title = NULL;
        obj->keyEquivalent = NULL;
        obj->buttonType = NX_PUSHBUTTON;
    }
    return obj;
}

+ (NXButton *)buttonWithTitle:(const char *)aTitle
{
    NXButton *btn = [self new];
    [btn setTitle:aTitle];
    return btn;
}

- (void)setTitle:(const char *)aTitle
{
    if (title) free((void *)title);
    title = aTitle ? strdup(aTitle) : NULL;
}

- (const char *)title
{
    return title;
}

- (void)setKeyEquivalent:(const char *)key
{
    if (keyEquivalent) free((void *)keyEquivalent);
    keyEquivalent = key ? strdup(key) : NULL;
}

- (const char *)keyEquivalent
{
    return keyEquivalent;
}

- (void)setButtonType:(int)type
{
    buttonType = type;
}

- (int)buttonType
{
    return buttonType;
}

- (void)setImage:(id)image
{
    /* Image handling would go here */
}

- (id)image
{
    return nil;
}

@end

/* ========================================================================
 * NXText Implementation
 * ======================================================================== */

@implementation NXText

+ (id)new
{
    NXText *obj = [super new];
    if (obj) {
        obj->textStorage = NULL;
        obj->textLength = 0;
        obj->textStyle = NX_TXSTANDARDSTYLE;
        obj->delegate = nil;
        obj->isEditable = YES;
        obj->isSelectable = YES;
        obj->isRichText = NO;
    }
    return obj;
}

+ (id)newWithFrame:(NXRect)frame
{
    NXText *obj = [self new];
    [obj setFrame:frame];
    return obj;
}

- (void)setString:(const char *)string
{
    if (textStorage) free((void *)textStorage);
    textStorage = string ? strdup(string) : NULL;
    textLength = string ? strlen(string) : 0;
}

- (const char *)string
{
    return textStorage;
}

- (void)setEditable:(BOOL)flag
{
    isEditable = flag;
}

- (BOOL)isEditable
{
    return isEditable;
}

- (void)setSelectable:(BOOL)flag
{
    isSelectable = flag;
}

- (BOOL)isSelectable
{
    return isSelectable;
}

- (BOOL)writeRTFToFile:(const char *)path
{
    /* RTF writing would be implemented here */
    return textStorage ? YES : NO;
}

- (BOOL)readRTFFromFile:(const char *)path
{
    /* RTF reading would be implemented here */
    return NO;
}

- (void)setRichText:(BOOL)flag
{
    isRichText = flag;
}

- (BOOL)isRichText
{
    return isRichText;
}

- (void)selectAll:(id)sender
{
    /* Selection handling would go here */
}

- (void)copy:(id)sender
{
    /* Copy to clipboard would go here */
}

- (void)cut:(id)sender
{
    /* Cut to clipboard would go here */
}

- (void)paste:(id)sender
{
    /* Paste from clipboard would go here */
}

@end

/* ========================================================================
 * NXScrollView Implementation
 * ======================================================================== */

@implementation NXScrollView

+ (id)newWithFrame:(NXRect)frame
{
    NXScrollView *obj = [super new];
    if (obj) {
        [obj setFrame:frame];
        obj->documentView = nil;
        obj->horizontalScroller = nil;
        obj->verticalScroller = nil;
        obj->contentSize = frame;
        obj->documentOrigin = NXMakePoint(0, 0);
        obj->hasHorizontalScroller = NO;
        obj->hasVerticalScroller = YES;
        obj->hasScroller = YES;
        obj->scrollerWidth = 15.0;
    }
    return obj;
}

- (void)setDocumentView:(NXView *)aView
{
    documentView = aView;
    if (aView) {
        [self addSubview:aView];
        [self tile];
    }
}

- (NXView *)documentView
{
    return documentView;
}

- (void)setHasHorizontalScroller:(BOOL)flag
{
    hasHorizontalScroller = flag;
    [self tile];
}

- (BOOL)hasHorizontalScroller
{
    return hasHorizontalScroller;
}

- (void)setHasVerticalScroller:(BOOL)flag
{
    hasVerticalScroller = flag;
    [self tile];
}

- (BOOL)hasVerticalScroller
{
    return hasVerticalScroller;
}

- (void)setHasScroller:(BOOL)flag
{
    hasScroller = flag;
    [self tile];
}

- (BOOL)hasScroller
{
    return hasScroller;
}

- (void)scrollToPoint:(NXPoint)point
{
    documentOrigin = point;
}

- (NXPoint)documentVisibleOrigin
{
    return documentOrigin;
}

- (void)tile
{
    float w = NXWidth(frame);
    float h = NXHeight(frame);
    float scrollerW = hasScroller ? scrollerWidth : 0;
    
    if (documentView) {
        NXRect docFrame;
        docFrame.origin_x = 0;
        docFrame.origin_y = 0;
        docFrame.size_width = w;
        docFrame.size_height = h;
        if (hasVerticalScroller) docFrame.size_width -= scrollerW;
        if (hasHorizontalScroller) docFrame.size_height -= scrollerW;
        [documentView setFrame:docFrame];
    }
}

@end

/* ========================================================================
 * NXWindow Implementation
 * ======================================================================== */

@implementation NXWindow

+ (id)new
{
    NXWindow *obj = [super new];
    if (obj) {
        obj->contentView = nil;
        obj->title = NULL;
        obj->windowRect = NXMakeRect(0, 0, 200, 200);
        obj->styleMask = NX_TITLEDWINDOWMASK | NX_CLOSABLEWINDOWMASK;
        obj->childWindows = NULL;
        obj->childCount = 0;
        obj->parentWindow = nil;
        obj->isVisible = NO;
        obj->isMiniaturized = NO;
        obj->isReleasedWhenClosed = NO;
        obj->delegate = nil;
    }
    return obj;
}

+ (id)newWithContentRect:(NXRect)contentRect styleMask:(int)mask
{
    NXWindow *obj = [self new];
    if (obj) {
        obj->windowRect = contentRect;
        obj->styleMask = mask;
        [obj setFrame:contentRect];
    }
    return obj;
}

- (void)setContentView:(NXView *)aView
{
    contentView = aView;
    if (aView) {
        [self addSubview:aView];
    }
}

- (NXView *)contentView
{
    return contentView;
}

- (void)setTitle:(const char *)aTitle
{
    if (title) free((void *)title);
    title = aTitle ? strdup(aTitle) : NULL;
}

- (const char *)title
{
    return title;
}

- (void)makeKeyAndOrderFront:(id)sender
{
    isVisible = YES;
    if (NXApp) {
        [NXApp addWindow:self];
        [NXApp setKeyWindow:self];
    }
}

- (void)orderOut:(id)sender
{
    isVisible = NO;
    if (NXApp) {
        [NXApp removeWindow:self];
    }
}

- (void)orderWindow:(int)place
{
    /* Window ordering would go here */
}

- (void)close
{
    if (isReleasedWhenClosed) {
        /* Release resources */
    }
    [self orderOut:nil];
}

- (void)miniaturize:(id)sender
{
    isMiniaturized = YES;
    [self orderOut:nil];
}

- (void)deminiaturize:(id)sender
{
    isMiniaturized = NO;
    [self makeKeyAndOrderFront:nil];
}

- (BOOL)isVisible
{
    return isVisible;
}

- (BOOL)isMiniaturized
{
    return isMiniaturized;
}

- (void)setReleasedWhenClosed:(BOOL)flag
{
    isReleasedWhenClosed = flag;
}

- (BOOL)isReleasedWhenClosed
{
    return isReleasedWhenClosed;
}

- (void)setDelegate:(id)anObject
{
    delegate = anObject;
}

- (id)delegate
{
    return delegate;
}

- (void)addChildWindow:(NXWindow *)childWindow
{
    if (childCount == 0) {
        childWindows = (NXWindow **)malloc(4 * sizeof(NXWindow *));
    } else {
        childWindows = (NXWindow **)realloc(childWindows, (childCount + 1) * sizeof(NXWindow *));
    }
    childWindows[childCount++] = childWindow;
    childWindow->parentWindow = self;
}

- (void)removeChildWindow:(NXWindow *)childWindow
{
    int i;
    for (i = 0; i < childCount; i++) {
        if (childWindows[i] == childWindow) {
            int j;
            for (j = i; j < childCount - 1; j++) {
                childWindows[j] = childWindows[j + 1];
            }
            childCount--;
            childWindow->parentWindow = nil;
            break;
        }
    }
}

- (void)setStyleMask:(int)mask
{
    styleMask = mask;
}

- (int)styleMask
{
    return styleMask;
}

@end

/* ========================================================================
 * NXMenu Implementation
 * ======================================================================== */

@implementation NXMenu

+ (id)new
{
    NXMenu *obj = [super new];
    if (obj) {
        obj->title = NULL;
        obj->items = NULL;
        obj->itemCount = 0;
        obj->itemCapacity = 0;
        obj->submenu = nil;
        obj->superMenu = nil;
    }
    return obj;
}

+ (id)menuWithTitle:(const char *)aTitle
{
    NXMenu *menu = [self new];
    [menu setTitle:aTitle];
    return menu;
}

- (void)addItem:(NXMenuItem *)anItem
{
    if (itemCount >= itemCapacity) {
        itemCapacity = itemCapacity ? itemCapacity * 2 : 4;
        items = (NXMenuItem **)realloc(items, itemCapacity * sizeof(NXMenuItem *));
    }
    items[itemCount++] = anItem;
}

- (void)addItemWithTitle:(const char *)aTitle action:(SEL)action keyEquivalent:(const char *)key
{
    NXMenuItem *item = [NXMenuItem itemWithTitle:aTitle action:action keyEquivalent:key];
    [self addItem:item];
}

- (NXMenuItem *)addItemSeparator
{
    NXMenuItem *item = [NXMenuItem new];
    [item setType:NX_MENUSEPAENTRY];
    [self addItem:item];
    return item;
}

- (void)insertItem:(NXMenuItem *)anItem atIndex:(int)index
{
    if (index < 0 || index > itemCount) return;
    if (itemCount >= itemCapacity) {
        itemCapacity = itemCapacity ? itemCapacity * 2 : 4;
        items = (NXMenuItem **)realloc(items, itemCapacity * sizeof(NXMenuItem *));
    }
    int i;
    for (i = itemCount; i > index; i--) {
        items[i] = items[i - 1];
    }
    items[index] = anItem;
    itemCount++;
}

- (void)removeItemAtIndex:(int)index
{
    if (index < 0 || index >= itemCount) return;
    int i;
    for (i = index; i < itemCount - 1; i++) {
        items[i] = items[i + 1];
    }
    itemCount--;
}

- (void)removeItem:(NXMenuItem *)anItem
{
    int i;
    for (i = 0; i < itemCount; i++) {
        if (items[i] == anItem) {
            [self removeItemAtIndex:i];
            break;
        }
    }
}

- (NXMenuItem *)itemAtIndex:(int)index
{
    if (index < 0 || index >= itemCount) return nil;
    return items[index];
}

- (int)numberOfItems
{
    return itemCount;
}

- (void)setSubmenu:(NXMenu *)aMenu forItem:(NXMenuItem *)anItem
{
    [anItem setSubmenu:aMenu];
    [aMenu setSuperMenu:self];
}

- (NXMenu *)submenu
{
    return submenu;
}

- (void)addSubmenu:(NXMenu *)menu
{
    submenu = menu;
    [menu setSuperMenu:self];
}

- (NXMenu *)supermenu
{
    return superMenu;
}

- (void)setTitle:(const char *)aTitle
{
    if (title) free((void *)title);
    title = aTitle ? strdup(aTitle) : NULL;
}

- (const char *)title
{
    return title;
}

@end

/* ========================================================================
 * NXMenuItem Implementation
 * ======================================================================== */

@implementation NXMenuItem

+ (id)new
{
    NXMenuItem *obj = [super new];
    if (obj) {
        obj->title = NULL;
        obj->action = NULL;
        obj->target = nil;
        obj->keyEquivalent = NULL;
        obj->keyEquivalentModifierMask = 0;
        obj->submenu = nil;
        obj->tag = 0;
        obj->isEnabled = YES;
        obj->itemType = NX_MENUITEMENTRY;
        obj->representedObject = nil;
    }
    return obj;
}

+ (id)itemWithTitle:(const char *)aTitle action:(SEL)action keyEquivalent:(const char *)key
{
    NXMenuItem *item = [self new];
    [item setTitle:aTitle];
    [item setAction:action];
    [item setKeyEquivalent:key];
    return item;
}

- (void)setTitle:(const char *)aTitle
{
    if (title) free((void *)title);
    title = aTitle ? strdup(aTitle) : NULL;
}

- (const char *)title
{
    return title;
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

- (void)setKeyEquivalent:(const char *)key
{
    if (keyEquivalent) free((void *)keyEquivalent);
    keyEquivalent = key ? strdup(key) : NULL;
}

- (const char *)keyEquivalent
{
    return keyEquivalent;
}

- (void)setKeyEquivalentModifierMask:(int)mask
{
    keyEquivalentModifierMask = mask;
}

- (int)keyEquivalentModifierMask
{
    return keyEquivalentModifierMask;
}

- (void)setSubmenu:(NXMenu *)aMenu
{
    submenu = aMenu;
}

- (NXMenu *)submenu
{
    return submenu;
}

- (void)setEnabled:(BOOL)flag
{
    isEnabled = flag;
}

- (BOOL)isEnabled
{
    return isEnabled;
}

- (void)setTag:(int)aTag
{
    tag = aTag;
}

- (int)tag
{
    return tag;
}

- (void)setType:(int)aType
{
    itemType = aType;
}

- (int)type
{
    return itemType;
}

- (void)setRepresentedObject:(id)object
{
    representedObject = object;
}

- (id)representedObject
{
    return representedObject;
}

@end

/* ========================================================================
 * NXOpenPanel Implementation
 * ======================================================================== */

@implementation NXOpenPanel

+ (id)openPanel
{
    static NXOpenPanel *sharedPanel = nil;
    if (!sharedPanel) {
        sharedPanel = [self new];
    }
    return sharedPanel;
}

+ (id)new
{
    NXOpenPanel *obj = [super new];
    if (obj) {
        obj->directory = NULL;
        obj->filename = NULL;
        obj->allowedFileTypes = NULL;
        obj->allowedFileTypeCount = 0;
        obj->canChooseFiles = YES;
        obj->canChooseDirectories = NO;
        obj->allowsMultipleSelection = NO;
        obj->delegate = nil;
    }
    return obj;
}

- (int)runModal
{
    return [self runModalForDirectory:directory file:filename types:allowedFileTypes];
}

- (int)runModalForDirectory:(const char *)dir file:(const char *)file types:(const char **)types
{
    /* Modal dialog would be implemented here */
    return NX_CANCELTAG;
}

- (void)setDirectory:(const char *)path
{
    if (directory) free((void *)directory);
    directory = path ? strdup(path) : NULL;
}

- (const char *)directory
{
    return directory;
}

- (void)setFilename:(const char *)name
{
    if (filename) free((void *)filename);
    filename = name ? strdup(name) : NULL;
}

- (const char *)filename
{
    return filename;
}

- (void)setAllowedFileTypes:(const char **)types count:(int)count
{
    allowedFileTypes = types;
    allowedFileTypeCount = count;
}

- (const char **)allowedFileTypes
{
    return allowedFileTypes;
}

- (int)allowedFileTypeCount
{
    return allowedFileTypeCount;
}

- (void)setCanChooseFiles:(BOOL)flag
{
    canChooseFiles = flag;
}

- (BOOL)canChooseFiles
{
    return canChooseFiles;
}

- (void)setCanChooseDirectories:(BOOL)flag
{
    canChooseDirectories = flag;
}

- (BOOL)canChooseDirectories
{
    return canChooseDirectories;
}

- (void)setAllowsMultipleSelection:(BOOL)flag
{
    allowsMultipleSelection = flag;
}

- (BOOL)allowsMultipleSelection
{
    return allowsMultipleSelection;
}

- (void)setDelegate:(id)anObject
{
    delegate = anObject;
}

- (id)delegate
{
    return delegate;
}

@end

/* ========================================================================
 * NXSavePanel Implementation
 * ======================================================================== */

@implementation NXSavePanel

+ (id)savePanel
{
    static NXSavePanel *sharedPanel = nil;
    if (!sharedPanel) {
        sharedPanel = [self new];
    }
    return sharedPanel;
}

+ (id)new
{
    NXSavePanel *obj = [super new];
    if (obj) {
        obj->directory = NULL;
        obj->filename = NULL;
        obj->requiredFileType = NULL;
        obj->message = NULL;
        obj->delegate = nil;
        obj->canCreateDirectories = YES;
        obj->isExpanded = NO;
    }
    return obj;
}

- (int)runModal
{
    return [self runModalForDirectory:directory file:filename];
}

- (int)runModalForDirectory:(const char *)dir file:(const char *)file
{
    /* Modal dialog would be implemented here */
    return NX_CANCELTAG;
}

- (void)setDirectory:(const char *)path
{
    if (directory) free((void *)directory);
    directory = path ? strdup(path) : NULL;
}

- (const char *)directory
{
    return directory;
}

- (void)setFilename:(const char *)name
{
    if (filename) free((void *)filename);
    filename = name ? strdup(name) : NULL;
}

- (const char *)filename
{
    return filename;
}

- (void)setRequiredFileType:(const char *)type
{
    if (requiredFileType) free((void *)requiredFileType);
    requiredFileType = type ? strdup(type) : NULL;
}

- (const char *)requiredFileType
{
    return requiredFileType;
}

- (void)setMessage:(const char *)msg
{
    if (message) free((void *)message);
    message = msg ? strdup(msg) : NULL;
}

- (const char *)message
{
    return message;
}

- (void)setCanCreateDirectories:(BOOL)flag
{
    canCreateDirectories = flag;
}

- (BOOL)canCreateDirectories
{
    return canCreateDirectories;
}

- (void)setExpanded:(BOOL)flag
{
    isExpanded = flag;
}

- (BOOL)isExpanded
{
    return isExpanded;
}

- (void)setDelegate:(id)anObject
{
    delegate = anObject;
}

- (id)delegate
{
    return delegate;
}

@end

/* ========================================================================
 * NXAlertPanel Implementation
 * ======================================================================== */

@implementation NXAlertPanel

+ (id)alertPanelWithTitle:(const char *)title 
                  message:(const char *)message 
           defaultButton:(const char *)defaultButton 
         alternateButton:(const char *)alternateButton 
              otherButton:(const char *)otherButton
{
    NXAlertPanel *panel = [self new];
    [panel setTitle:title];
    [panel setAlertStyle:NX_INFORMATIONALERTLEVEL];
    return panel;
}

- (void)setAlertStyle:(int)style
{
    alertStyle = style;
}

- (int)alertStyle
{
    return alertStyle;
}

- (void)setMessageText:(const char *)text
{
    /* Message text would be set here */
}

- (const char *)messageText
{
    return NULL;
}

- (void)setDefaultButtonTitle:(const char *)title
{
    /* Default button title would be set here */
}

- (const char *)defaultButtonTitle
{
    return NULL;
}

- (void)setAlternateButtonTitle:(const char *)title
{
    /* Alternate button title would be set here */
}

- (const char *)alternateButtonTitle
{
    return NULL;
}

- (void)setOtherButtonTitle:(const char *)title
{
    /* Other button title would be set here */
}

- (const char *)otherButtonTitle
{
    return NULL;
}

@end

/* ========================================================================
 * MXApplication Implementation
 * ======================================================================== */

@implementation MXApplication

static MXApplication *sharedApplication = nil;

+ (MXApplication *)new
{
    if (!sharedApplication) {
        sharedApplication = [super new];
    }
    return sharedApplication;
}

+ (id)sharedApplication
{
    if (!sharedApplication) {
        sharedApplication = [self new];
    }
    return sharedApplication;
}

- (void)run
{
    NXApp = self;
    /* Main event loop would be implemented here */
    [super run];
}

- (void)terminate:(id)sender
{
    /* Cleanup and exit */
    [super terminate:sender];
}

- (void)loadNibFile:(const char *)nibPath
{
    [self loadNibFile:nibPath withOwner:self];
}

- (void)loadNibFile:(const char *)nibPath withOwner:(id)owner
{
    /* NIB loading would be implemented here using MXNibBuilder */
}

- (void)setAppDelegate:(id)delegate
{
    appDelegate = delegate;
}

- (id)appDelegate
{
    return appDelegate;
}

- (const char *)appBundlePath
{
    return appBundlePath;
}

@end

/* ========================================================================
 * Alert Panel Functions
 * ======================================================================== */

id NXRunAlertPanel(id sender, const char *title, const char *msg,
                   const char *defaultButton, const char *alternateButton,
                   const char *otherButton, ...)
{
    NXAlertPanel *panel = [NXAlertPanel alertPanelWithTitle:title
                                                    message:msg
                                             defaultButton:defaultButton
                                           alternateButton:alternateButton
                                                otherButton:otherButton];
    return panel;
}

id NXGetAlertPanel(id sender, const char *title, const char *msg,
                   const char *defaultButton, const char *alternateButton,
                   const char *otherButton)
{
    return [NXAlertPanel alertPanelWithTitle:title
                                     message:msg
                              defaultButton:defaultButton
                            alternateButton:alternateButton
                                 otherButton:otherButton];
}

void NXFreeAlertPanel(id panel)
{
    /* Panel cleanup would go here */
}