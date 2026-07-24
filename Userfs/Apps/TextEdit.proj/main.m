#objc
#import <appkit/appkit.h>
#import <appkit/terminalapp.h>

int main(int argc, char *argv[]) {
    [MXApplication new];
    [NXApp loadNibFile:"TextEdit.nib.m"];
    [NXApp run];

    return 0;
}