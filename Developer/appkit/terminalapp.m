/*
 * appkit/terminalapp.m - MinSTEP Terminal Application Implementation
 *
 * Terminal-based application support for MinSTEP AppKit.
 *
 * Copyright (c) 2026 MinSTEP Project
 * Licensed under the MIT License.
 */

#objc
#import <appkit/terminalapp.h>

/* Global state */
static MXTerminalApplication *_sharedTerminalApp = nil;

/* ========================================================================
 * Display Mode Detection
 * ======================================================================== */

BOOL MXUseTUI(void)
{
    /* Check environment variable */
    const char *env = getenv(MX_TUI_FORCE_ENV);
    if (env && strcmp(env, "1") == 0) {
        return YES;
    }
    
    /* Check if DISPLAY is set (for GUI mode) */
    const char *display = getenv("DISPLAY");
    if (display && strlen(display) > 0) {
        return NO; /* Has display, use GUI */
    }
    
    /* Check if running in a terminal */
    if (!isatty(STDIN_FILENO) || !isatty(STDOUT_FILENO)) {
        return NO; /* Not a terminal, can't use TUI */
    }
    
    /* Default to TUI if no display server available */
    return YES;
}

/* ========================================================================
 * MXTerminalApplication Implementation
 * ======================================================================== */

@implementation MXTerminalApplication

+ (MXTerminalApplication *)sharedApplication
{
    if (!_sharedTerminalApp) {
        _sharedTerminalApp = [self new];
    }
    return _sharedTerminalApp;
}

+ (id)new
{
    MXTerminalApplication *obj = [super new];
    if (obj) {
        obj->tuiEventLoop = nil;
        obj->tuiWindows = NULL;
        obj->tuiWindowCount = 0;
    }
    return obj;
}

+ (BOOL)useTUI
{
    return MXUseTUI();
}

- (void)run
{
    if ([MXTerminalApplication useTUI]) {
        [self runTUI];
    } else {
        [self runGUI];
    }
}

- (void)runTUI
{
    /* Initialize TUI system */
    MXTUIInitialize();
    
    /* Get event loop */
    tuiEventLoop = [MXTUIEventLoop sharedEventLoop];
    
    /* Set up signal handlers for resize */
    signal(SIGWINCH, SIG_IGN);
    
    /* Run the event loop */
    [tuiEventLoop run];
}

- (void)runGUI
{
    /* GUI mode would connect to PostScript display server */
    /* For now, fall back to TUI */
    [self runTUI];
}

- (void)draw
{
    MXTUIRefresh();
}

- (void)addTUIWindow:(MXTUIWindow *)window
{
    if (tuiWindowCount == 0) {
        tuiWindows = (MXTUIWindow **)malloc(4 * sizeof(MXTUIWindow *));
    }
    tuiWindows[tuiWindowCount++] = window;
    
    if (tuiEventLoop) {
        [tuiEventLoop addWindow:window];
    }
}

- (void)removeTUIWindow:(MXTUIWindow *)window
{
    int i;
    for (i = 0; i < tuiWindowCount; i++) {
        if (tuiWindows[i] == window) {
            int j;
            for (j = i; j < tuiWindowCount - 1; j++) {
                tuiWindows[j] = tuiWindows[j + 1];
            }
            tuiWindowCount--;
            break;
        }
    }
    
    if (tuiEventLoop) {
        [tuiEventLoop removeWindow:window];
    }
}

- (void)refreshTUI
{
    MXTUIRefresh();
}

@end

/* ========================================================================
 * MXTerminalTextView Implementation
 * ======================================================================== */

@implementation MXTerminalTextView

+ (id)new
{
    MXTerminalTextView *obj = [super new];
    if (obj) {
        obj->tuiTextView = nil;
        obj->history = nil;
        obj->historySize = 100;
    }
    return obj;
}

- (void)setTUIView:(MXTUITextView *)view
{
    tuiTextView = view;
}

- (MXTUITextView *)tuiTextView
{
    return tuiTextView;
}

- (void)appendText:(const char *)text
{
    if (tuiTextView) {
        [tuiTextView appendText:text];
    }
    /* Also update underlying text storage */
    [super setString:text];
}

- (void)appendLine:(const char *)line
{
    if (tuiTextView) {
        [tuiTextView appendText:line];
        [tuiTextView appendText:"\n"];
    }
}

- (void)clear
{
    if (tuiTextView) {
        [tuiTextView clear];
    }
    [super setString:""];
}

@end

/* ========================================================================
 * MXTerminalButton Implementation
 * ======================================================================== */

@implementation MXTerminalButton

+ (id)new
{
    MXTerminalButton *obj = [super new];
    if (obj) {
        obj->tuiButton = nil;
        obj->x = 0;
        obj->y = 0;
        obj->width = 0;
        obj->height = 1;
        obj->isHighlighted = NO;
    }
    return obj;
}

+ (id)buttonWithTitle:(const char *)title atX:(int)posX y:(int)posY
{
    MXTerminalButton *btn = [self new];
    [btn setTitle:title];
    [btn setPositionX:posX y:posY];
    return btn;
}

- (void)setTUIButton:(MXTUIButton *)button
{
    tuiButton = button;
}

- (MXTUIButton *)tuiButton
{
    return tuiButton;
}

- (void)draw
{
    if (tuiButton) {
        [tuiButton display];
    }
}

- (void)highlight
{
    isHighlighted = YES;
    if (tuiButton) {
        [tuiButton setHighlighted:YES];
    }
}

- (void)unhighlight
{
    isHighlighted = NO;
    if (tuiButton) {
        [tuiButton setHighlighted:NO];
    }
}

- (void)setPositionX:(int)posX y:(int)posY
{
    x = posX;
    y = posY;
    if (tuiButton) {
        TUIRect frame = TUIRectMake(posX, posY, width, 1);
        [tuiButton setFrame:frame];
    }
}

@end

/* ========================================================================
 * MXTerminalMenu Implementation
 * ======================================================================== */

@implementation MXTerminalMenu

+ (id)new
{
    MXTerminalMenu *obj = [super new];
    if (obj) {
        obj->tuiMenuBar = nil;
        obj->selectedIndex = -1;
        obj->isOpen = NO;
    }
    return obj;
}

+ (id)menuWithTitle:(const char *)title
{
    MXTerminalMenu *menu = [self new];
    [menu setTitle:title];
    return menu;
}

- (void)setTUIMenuBar:(MXTUIMenuBar *)menuBar
{
    tuiMenuBar = menuBar;
}

- (MXTUIMenuBar *)tuiMenuBar
{
    return tuiMenuBar;
}

- (void)display
{
    if (tuiMenuBar) {
        [tuiMenuBar display];
    }
}

- (int)runModalSelection
{
    /* Simple menu selection using keyboard */
    MXTUIConsole *con = [MXTUIConsole sharedConsole];
    [con initialize];
    
    [self display];
    [con refresh];
    
    int result = -1;
    int count = [self numberOfItems];
    
    while (1) {
        int ch = getchar();
        
        if (ch == 27) { /* ESC */
            result = -1;
            break;
        } else if (ch == '\n' || ch == '\r') {
            /* Enter - select current */
            result = selectedIndex;
            break;
        } else if (ch == '[') { /* Arrow keys */
            int arrow = getchar();
            if (arrow == 'C') { /* Right */
                selectedIndex++;
                if (selectedIndex >= count) selectedIndex = 0;
            } else if (arrow == 'D') { /* Left */
                selectedIndex--;
                if (selectedIndex < 0) selectedIndex = count - 1;
            }
            [self display];
            [con refresh];
        } else if (ch >= '1' && ch <= '9') {
            int idx = ch - '1';
            if (idx < count) {
                result = idx;
                break;
            }
        }
    }
    
    return result;
}

- (void)setSelectedIndex:(int)index
{
    selectedIndex = index;
    if (tuiMenuBar) {
        [tuiMenuBar selectItemAtIndex:index];
    }
}

- (int)selectedIndex
{
    return selectedIndex;
}

- (BOOL)isOpen
{
    return isOpen;
}

- (void)setOpen:(BOOL)open
{
    isOpen = open;
    if (tuiMenuBar) {
        if (open) {
            [tuiMenuBar openMenuAtIndex:selectedIndex];
        } else {
            [tuiMenuBar closeMenu];
        }
    }
}

@end

/* ========================================================================
 * MXTerminalWindow Implementation
 * ======================================================================== */

@implementation MXTerminalWindow

+ (id)new
{
    MXTerminalWindow *obj = [super new];
    if (obj) {
        obj->tuiWindow = nil;
        obj->isMovable = YES;
    }
    return obj;
}

+ (id)windowWithTitle:(const char *)title frame:(NXRect)frame
{
    MXTerminalWindow *win = [self new];
    [win setTitle:title];
    
    /* Create TUI window */
    TUIRect tuiFrame = TUIRectMake((int)frame.origin_x, (int)frame.origin_y,
                                    (int)frame.size_width, (int)frame.size_height);
    MXTUIWindow *tuiWin = [MXTUIWindow windowWithTitle:title frame:tuiFrame];
    [win setTUIWindow:tuiWin];
    
    return win;
}

- (void)setTUIWindow:(MXTUIWindow *)window
{
    tuiWindow = window;
}

- (MXTUIWindow *)tuiWindow
{
    return tuiWindow;
}

- (void)setMovable:(BOOL)movable
{
    isMovable = movable;
    if (tuiWindow) {
        [tuiWindow setMovable:movable];
    }
}

- (BOOL)isMovable
{
    return isMovable;
}

- (void)draw
{
    if (tuiWindow) {
        [tuiWindow display];
    }
}

@end

/* ========================================================================
 * MXTerminalPanel Implementation
 * ======================================================================== */

@implementation MXTerminalPanel

+ (id)alertPanelWithTitle:(const char *)title
                  message:(const char *)message
           defaultButton:(const char *)defButton
         alternateButton:(const char *)altButton
              otherButton:(const char *)otherButton
{
    MXTerminalPanel *panel = [self new];
    [panel setTitle:title];
    panel->messageText = message ? strdup(message) : NULL;
    panel->defaultButton = defButton ? strdup(defButton) : NULL;
    panel->alternateButton = altButton ? strdup(altButton) : NULL;
    panel->otherButton = otherButton ? strdup(otherButton) : NULL;
    
    /* Create TUI alert panel */
    MXTUIAlertPanel *tuiPanel = [MXTUIAlertPanel alertWithTitle:title
                                                        message:message
                                                 defaultButton:defButton
                                               alternateButton:altButton
                                                    otherButton:otherButton];
    panel->tuiAlertPanel = tuiPanel;
    
    return panel;
}

- (int)runModal
{
    if (tuiAlertPanel) {
        return [tuiAlertPanel runModal];
    }
    return NX_CANCELTAG;
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

@end

/* ========================================================================
 * MXTerminalOpenPanel Implementation
 * ======================================================================== */

@implementation MXTerminalOpenPanel

+ (id)openPanel
{
    static MXTerminalOpenPanel *sharedPanel = nil;
    if (!sharedPanel) {
        sharedPanel = [self new];
    }
    return sharedPanel;
}

+ (id)new
{
    MXTerminalOpenPanel *obj = [super new];
    if (obj) {
        obj->tuiFileList = nil;
        obj->tuiPathField = nil;
        obj->currentDirectory = NULL;
        obj->files = NULL;
        obj->fileCount = 0;
    }
    return obj;
}

- (int)runModal
{
    /* Simple terminal file picker */
    MXTUIConsole *con = [MXTUIConsole sharedConsole];
    [con initialize];
    
    /* Draw simple file browser */
    printf("\n");
    printf("=== File Open ===\n");
    printf("Current: %s\n", currentDirectory ? currentDirectory : "/");
    printf("\n");
    printf("Files:\n");
    
    int i;
    for (i = 0; i < fileCount; i++) {
        printf("  %d. %s\n", i + 1, files[i]);
    }
    
    printf("\n");
    printf("Enter number (0 to cancel): ");
    fflush(stdout);
    
    int choice = 0;
    scanf("%d", &choice);
    
    if (choice > 0 && choice <= fileCount) {
        [self setFilename:files[choice - 1]];
        return NX_OKTAG;
    }
    
    return NX_CANCELTAG;
}

- (void)setCurrentDirectory:(const char *)path
{
    if (currentDirectory) free((void *)currentDirectory);
    currentDirectory = path ? strdup(path) : NULL;
    [self refreshFileList];
}

- (const char *)currentDirectory
{
    return currentDirectory;
}

- (void)refreshFileList
{
    /* Would scan directory and populate files array */
    /* This is a placeholder */
}

@end

/* ========================================================================
 * MXTerminalSavePanel Implementation
 * ======================================================================== */

@implementation MXTerminalSavePanel

+ (id)savePanel
{
    static MXTerminalSavePanel *sharedPanel = nil;
    if (!sharedPanel) {
        sharedPanel = [self new];
    }
    return sharedPanel;
}

+ (id)new
{
    MXTerminalSavePanel *obj = [super new];
    if (obj) {
        obj->tuiFilenameField = nil;
        obj->tuiPathField = nil;
    }
    return obj;
}

- (int)runModal
{
    /* Simple terminal save dialog */
    printf("\n");
    printf("=== File Save ===\n");
    printf("Directory: %s\n", [self directory] ? [self directory] : ".");
    printf("\n");
    printf("Filename: ");
    fflush(stdout);
    
    static char filenameBuf[1024];
    if (scanf("%1023s", filenameBuf) == 1) {
        [self setFilename:filenameBuf];
        return NX_OKTAG;
    }
    
    return NX_CANCELTAG;
}

- (const char *)filename
{
    return [self filename];
}

- (const char *)directory
{
    return [self directory];
}

@end
