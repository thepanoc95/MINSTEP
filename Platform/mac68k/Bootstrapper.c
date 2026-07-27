#include <Types.h>
#include <Windows.h>
#include <Menus.h>

int main() {

    MenuHandle menuBar;
    menuBar = GetNewMBar(128);
    SetMenuBar(menuBar);
    DrawMenuBar();

    WindowRef myWindow;
    Rect bounds = {100, 100, 400, 600};
    CreateNewWindow(kDocumentWindowClass, 
    kWindowStandardDocumentAttributes | kWindowGoAwayMask,
    &bounds, &myWindow);
    ShowWindow(myWindow);

    CFStringRef title = CFStringCreateWithCString(
      NULL, "MINSTEP Operating System Bootstrapper", kCFStringEncodingMacRoman);
    SetWindowTitle(myWindow, title);
    CFRelease(title);

    return 0;
}