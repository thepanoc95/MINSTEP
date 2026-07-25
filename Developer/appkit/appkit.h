/*
 * appkit/appkit.h - MinSTEP AppKit Umbrella Header
 *
 * The AppKit framework provides the classes for building graphical
 * user interfaces in MinSTEP. It is inspired by NeXTSTEP's AppKit
 * and provides window, view, menu, and control classes.
 *
 * Copyright (c) 2026 MinSTEP Project
 * Licensed under the MIT License.
 */

#ifndef _APPKIT_APPKIT_H_
#define _APPKIT_APPKIT_H_

#objc
#import <foundation/foundation.h>

/* Forward declarations of AppKit classes */
@class NXApplication;
@class NXWindow;
@class NXView;
@class NXControl;
@class NXText;
@class NXButton;
@class NXScrollView;
@class NXMenu;
@class NXMenuItem;
@class NXOpenPanel;
@class NXSavePanel;
@class MXApplication;
@class MXNibBuilder;
@class NXAlertPanel;

/* TUI (Text User Interface) support - for terminal-based apps */
#import <appkit/terminal/tui.h>
#import <appkit/terminalapp.h>

/* AppKit version */
#define NXAppKitVersionNumber 1.0
#define NXAppKitVersionString "MinSTEP AppKit 1.0"

/* Return values for modal panels */
#define NX_OKTAG       1
#define NX_CANCELTAG   0
#define NX_ABORTTAG   -1
#define NX_NONMODALTAG -2

/* Key equivalent modifier flags */
#define NX_COMMANDKEY  (1 << 0)
#define NX_SHIFTKEY    (1 << 1)
#define NX_OPTIONKEY   (1 << 2)
#define NX_CONTROLKEY  (1 << 3)

/* Menu item types */
#define NX_MENUITEMENTRY  0
#define NX_MENUSEPAENTRY  1

/* Window style masks */
#define NX_TITLEDWINDOWMASK    (1 << 0)
#define NX_CLOSABLEWINDOWMASK  (1 << 1)
#define NX_MINIATURIZABLEWINDOWMASK (1 << 2)
#define NX_RESIZABLEWINDOWMASK (1 << 3)
#define NX_UNITYTITLEDWINDOWMASK (1 << 4)

/* View autoresizing masks */
#define NX_NOTSIZABLE           0
#define NX_WIDTHSIZABLE         (1 << 0)
#define NX_HEIGHTSIZABLE        (1 << 1)
#define NX_WIDTHSIZABLE|NX_HEIGHTSIZABLE 3

/* Control styles */
#define NX_ROUNDRECTBUTTON      0
#define NX_SWITCHBUTTON         1
#define NX_CHECKBOX             2
#define NX_RADIOBUTTON          3
#define NX_PUSHBUTTON           4
#define NX_BEVELBUTTON          5

/* Text styles */
#define NX_TXSTANDARDSTYLE      0
#define NX_TXATTRIBUTABLESTYLE  1
#define NX_TXRTFSTYLE           2

/* Border types */
#define NX_NOBORDER             0
#define NX_LINEBORDER           1
#define NX_BEZELBORDER          2
#define NX_RECTBORDER           3

/* Control states */
#define NX_OFFSTATE             0
#define NX_ONSTATE              1
#define NX_MIXEDSTATE           2

/* NXAlertPanel levels */
#define NX_WARNINGALERTLEVEL    1
#define NX_STOPALERTLEVEL       2
#define NX_INFORMATIONALERTLEVEL 3

/* Cell flags */
#define NX_CELLMASK             0x07
#define NX_STATECELL            0x01
#define NX_PUSHCELL             0x02
#define NX_SWITCHCELL           0x03
#define NX_RADIOCELL            0x04

/* Function declarations for alert panels */
id NXRunAlertPanel(id sender, const char *title, const char *msg, 
                   const char *defaultButton, const char *alternateButton,
                   const char *otherButton, ...);
id NXGetAlertPanel(id sender, const char *title, const char *msg,
                   const char *defaultButton, const char *alternateButton,
                   const char *otherButton);
void NXFreeAlertPanel(id panel);

#endif /* _APPKIT_APPKIT_H_ */