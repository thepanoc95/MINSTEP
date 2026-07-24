#objc
#import <appkit/interfacebuilder/nib.h>
#import "TextEditController.h"

@interface TextEditNib : MXNibBuilder
@end

@implementation TextEditNib

- (void)declareInterfaceWithOwner:(id)owner {
    TextEditController *controller = (TextEditController *)owner;

    DECLARE(MXButton);
    DECLARE(NXOpenPanel);
    DECLARE(NXRunAlertPanel);
    
    OUTLET(controller, mainWindow, [super NXWindow]);
    OUTLET(controller, textView,   [super NXText]);

    NXScrollView *scrollView = DECLARE(NXScrollView);
    [scrollView setHasVerticalScroller:YES];
    [scrollView setDocumentView:controller->textView];
    
    [[controller->mainWindow contentView] addSubview:scrollView];

    NXMenu *mainMenu = DECLARE(NXMenu);
    NXMenu *fileMenu = DECLARE(NXMenu);
    NXMenu *infoMenu = DECLARE(NXMenu);
    [mainMenu addSubmenu:fileMenu];
    [mainMenu addSubmenu:infoMenu];

    NXMenuItem *openItem = [fileMenu addItemWithTitle:"Open..." action:@selector(open:) keyEquivalent:"o"];
    NXMenuItem *saveItem = [fileMenu addItemWithTitle:"Save As..." action:@selector(saveAs:) keyEquivalent:"S"];
    [fileMenu addItemSeparator];
    NXMenuItem *quitItem = [fileMenu addItemWithTitle:"Quit" action:@selector(quit:) keyEquivalent:"q"];
    NXMenuItem *InfoItem = []

    [openItem setTarget:controller];
    [saveItem setTarget:controller];
    [quitItem setTarget:controller];

    [NXApp setMainMenu:mainMenu];

    [controller->mainWindow makeKeyAndOrderFront:nil];
}

@end