#objc
#import "TextEditController.h"

@implementation TextEditController

    - (id)init {
        self = [super init];
        if (self) {
            currentFilePath = NULL;
        }
        return self;
    }

    - (void)setMainWindow:(NXWindow *)aWindow {
        mainWindow = aWindow;
    }

    - (void)setTextView:(NXText *)aText {
        textView = aText;
    }

    - (void)open:(id)sender {
        NXOpenPanel *openPanel = [NXOpenPanel new];

        if ([openPanel runModal] == NX_OKTAG) {
            if (currentFilePath) free(currentFilePath);
            currentFilePath = strdup(path);

            [mainWindow setTitle:[NSString stringWithCString:path]];
        } else {
            NXRunAlertPanel(
                "TextEdit",
                "Unable to open the selected document.",
                "OK",
                NULL,
                NULL
            );
        }
    }

    - (void)saveAs:(id)sender {
        NXSavePanel *savePanel = [NXSavePanel new];

        if ([savePanel runModal] == NX_OKTAG) {
            const char *path = [savePanel filename];

            if ([textView writeRTFToFile:path]) {
                if (currentFilePath) free(currentFilePath);
                currentFilePath = strdup(path);
                [mainWindow setTitle:[NSString stringWithCString:path]];
            } else {
                NXRunAlertPanel(
                    "TextEdit",
                    "Unable to save the current document.",
                    "OK",
                    NULL,
                    NULL
                );
            }
        }
    }

    - (void)quit:(id)sender {
        [NXApp terminate:self];
    }

@end