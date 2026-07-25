/*
 * appkit/terminalapp.h - MinSTEP Terminal Application Header
 *
 * Terminal-based application support for MinSTEP AppKit.
 * Provides TUI (Text User Interface) mode for applications
 * that run without a PostScript display server.
 *
 * Copyright (c) 2026 MinSTEP Project
 * Licensed under the MIT License.
 */

#ifndef _APPKIT_TERMINALAPP_H_
#define _APPKIT_TERMINALAPP_H_

#objc
#import <appkit/appkit.h>
#import <appkit/terminal/tui.h>

/* ========================================================================
 * Display Mode Detection
 * ======================================================================== */

/* Environment variable to force TUI mode */
#define MX_TUI_FORCE_ENV "MX_FORCE_TUI"
#define MX_TUI_DISABLE_ENV "MX_DISABLE_TUI"

/* Check if TUI mode should be used. DPS is preferred when available. */
BOOL MXUseTUI(void);
BOOL MXUseGUI(void);

/* ========================================================================
 * MXTerminalApplication - Terminal-based Application
 * ======================================================================== */

@interface MXTerminalApplication : MXApplication
{
    MXTUIEventLoop *tuiEventLoop;
    MXTUIWindow **tuiWindows;
    int tuiWindowCount;
}

+ (MXTerminalApplication *)sharedApplication;
+ (BOOL)useTUI;
- (void)run;
- (void)runTUI;
- (void)runGUI;
- (void)draw;
- (void)addTUIWindow:(MXTUIWindow *)window;
- (void)removeTUIWindow:(MXTUIWindow *)window;
- (void)refreshTUI;

@end

/* ========================================================================
 * MXTerminalTextView - Terminal Text Display
 * ======================================================================== */

@interface MXTerminalTextView : NXText
{
    MXTUITextView *tuiTextView;
    id history;
    int historySize;
}

+ (MXTerminalTextView *)new;
- (void)setTUIView:(MXTUITextView *)view;
- (MXTUITextView *)tuiTextView;
- (void)appendText:(const char *)text;
- (void)appendLine:(const char *)line;
- (void)clear;

@end

/* ========================================================================
 * MXTerminalButton - Terminal Button
 * ======================================================================== */

@interface MXTerminalButton : NXButton
{
    MXTUIButton *tuiButton;
    int x, y;
    int width, height;
    BOOL isHighlighted;
}

+ (MXTerminalButton *)new;
+ (MXTerminalButton *)buttonWithTitle:(const char *)title atX:(int)x y:(int)y;
- (void)setTUIButton:(MXTUIButton *)button;
- (MXTUIButton *)tuiButton;
- (void)draw;
- (void)highlight;
- (void)unhighlight;
- (void)setPositionX:(int)posX y:(int)posY;

@end

/* ========================================================================
 * MXTerminalMenu - Terminal Menu Display
 * ======================================================================== */

@interface MXTerminalMenu : NXMenu
{
    MXTUIMenuBar *tuiMenuBar;
    int selectedIndex;
    BOOL isOpen;
}

+ (MXTerminalMenu *)new;
+ (MXTerminalMenu *)menuWithTitle:(const char *)title;
- (void)setTUIMenuBar:(MXTUIMenuBar *)menuBar;
- (MXTUIMenuBar *)tuiMenuBar;
- (void)display;
- (int)runModalSelection;
- (void)setSelectedIndex:(int)index;
- (int)selectedIndex;
- (BOOL)isOpen;
- (void)setOpen:(BOOL)open;

@end

/* ========================================================================
 * MXTerminalWindow - Terminal Window
 * ======================================================================== */

@interface MXTerminalWindow : NXWindow
{
    MXTUIWindow *tuiWindow;
    BOOL isMovable;
}

+ (MXTerminalWindow *)new;
+ (MXTerminalWindow *)windowWithTitle:(const char *)title frame:(NXRect)frame;
- (void)setTUIWindow:(MXTUIWindow *)window;
- (MXTUIWindow *)tuiWindow;
- (void)setMovable:(BOOL)movable;
- (BOOL)isMovable;
- (void)draw;

@end

/* ========================================================================
 * MXTerminalPanel - Terminal Dialog Panel
 * ======================================================================== */

@interface MXTerminalPanel : NXWindow
{
    MXTUIAlertPanel *tuiAlertPanel;
    const char *messageText;
    const char *defaultButton;
    const char *alternateButton;
    const char *otherButton;
}

+ (MXTerminalPanel *)alertPanelWithTitle:(const char *)title
                                 message:(const char *)message
                          defaultButton:(const char *)defaultButton
                        alternateButton:(const char *)alternateButton
                             otherButton:(const char *)otherButton;
- (int)runModal;
- (void)setMessageText:(const char *)text;
- (const char *)messageText;

@end

/* ========================================================================
 * MXTerminalOpenPanel - Terminal File Open Panel
 * ======================================================================== */

@interface MXTerminalOpenPanel : NXOpenPanel
{
    MXTUIListView *tuiFileList;
    MXTUITextField *tuiPathField;
    const char *currentDirectory;
    const char **files;
    int fileCount;
}

+ (MXTerminalOpenPanel *)openPanel;
- (int)runModal;
- (void)setCurrentDirectory:(const char *)path;
- (const char *)currentDirectory;
- (void)refreshFileList;

@end

/* ========================================================================
 * MXTerminalSavePanel - Terminal File Save Panel
 * ======================================================================== */

@interface MXTerminalSavePanel : NXSavePanel
{
    MXTUITextField *tuiFilenameField;
    MXTUITextField *tuiPathField;
}

+ (MXTerminalSavePanel *)savePanel;
- (int)runModal;
- (const char *)filename;
- (const char *)directory;

@end

#endif /* _APPKIT_TERMINALAPP_H_ */
