#import <stdio.h>

#if defined(__MangoKernel__)
    // If your app is compiling natively inside/for your custom nanokernel
    #import <Application/Application.h>
    #import <DisplayManager.h>

	#define psdspEntry DisplayManagerMain(void)
	 
#elif defined(__GNUSTEP__)
    #import <AppKit/AppKit.h>
#elif defined(NX_CURRENT_COMPILER_RELEASE) && (NX_CURRENT_COMPILER_RELEASE >= 400)
    #import <AppKit/AppKit.h>
#else
    #import <appkit/appkit.h>
#endif

int main(int argc, const char *argv[]) {
    
#if defined(__MangoKernel__)

    return NXApplicationMain(psdspEntry);

#elif defined(__GNUSTEP__) || (defined(NX_CURRENT_COMPILER_RELEASE) && (NX_CURRENT_COMPILER_RELEASE >= 400))

    return NSApplicationMain(argc, argv);

#else
    [Application new];
    NXRunAlertPanel(
        "MangoKernel SDK Alert", 
        "MangoDeveloper requires OPENSTEP 4.0, GNUstep, or native MangoKernel runtime. Or you can just use the source code to adapt it for NeXTSTEP 2.0.", 
        "Quit", NULL, NULL
    );
    return 0;

#endif
}
