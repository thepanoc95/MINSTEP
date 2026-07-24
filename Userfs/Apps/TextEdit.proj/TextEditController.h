#objc
#import <objc/Object.h>
#import <appkit/appkit.h>
#import <appkit/terminalapp.h>


@interface TextEditController: Object
    {
        NXWindow *mainWindow;
        NXText   *textView;
        const char *currentFilePath;
    }

    - (void)open:(id)sender;
    - (void)saveAs:(id)sender;
    - (void)quit:(id)sender;

    - (void)setMainWindow:(NXWindow *)aWindow;
    - (void)setTextView:(NXText *)aText;
@end