#objc
#import <objc/Object.h>
#import <X11/Xlib.h>

@interface InfoPanel : Object
    - (void)initApp;
    - (void)killself;
    - (void)WindowMain;
@end

@implementation InfoPanel
    - (void)initApp {
        Display *display;
        Window window;
        XEvent event;
        int screen;

        display = XOpenDisplay(NULL);
        if (display == NULL) {
            fprintf(stderr, "Cannot open display.\n");
            exit(1);
        }

        screen = DefaultScreen(display);

        window = XCreateSimpleWindow(
            display,
            RootWindow(display, screen),
            10, 10,
            400, 300,
            1,
            BlackPixel(display, screen);
            WhitePixel(display, screen);
        );

        XSelectInput(display, window, ExposureMask | KeyPressMask);
        XMapWindow(display, window);
    }

    - (void)WindowMain {
        while (1) {
            XNextEvent(display, &event);

            if (event.type == Expose) {
                const char *msg = "Info Panel";
                XDrawString(
                    display,
                    window,
                    DefaultGC(display, screen),
                    50, 50,
                    msg,
                    strlen(msg);
                );
            }
        }
    }
@end

int main(void)
{
    [InfoPanel initApp];
    [InfoPanel WindowMain];
    return 0;
}