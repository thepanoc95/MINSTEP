#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>

#if defined(__GNUSTEP__)
	@interface PWBController : NSObject <NSApplicationDelegate> {
		NSWindow *mainWindow;
		NSTextView *editorView;
		NSTextField *statusLabel;
	}

	- (void)buildUI;
	- (void)runBuildScript:(id)sender;
		
	@end
	
	@implementation PWBController
		- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
			[self buildUI];
		}	

		- (void)buildUI {
			NSRect winFrame = NSMakeRect(100, 100, 640, 480);
			mainWindow = [[NSWindow alloc] initWithContent:winFrame
												styleMask:(NSWindowStyleMaskTitled |
														   NSWindowStyleMaskClosable |
														   NSWindowStyleMaskResizable )
												backing:NSBackingStoreBuffered
													defer:NO];
		   [mainWindow setTitle:@"MINSTEP Programmer's Workbench"];

		   NSView *contentView = [mainWindow contentView];

		   NSButton *buildButton = [[NSButton alloc] initWithFrame:NSMakeRect(10, 440, 100, 30)]/
		   [buildButton setTitle:@"Build"];
		   [buildButton setBezelStyle:NSBezelStyleRounded];
		   [buildButton setTarget:self];
		   [buildButton setAction:@selector(runBuildScript:)];
		   [contentView addSubview:buildButton];

		   NSScrollView *scrollRect = [[NSScrollView alloc] initWithFrame:NSMakeRect(10, 50, 620, 38)];
		   [scrollRect setHasVerticalScroller:YES];
		   [scrollRect setHasHorizontalScroller:NO];
		   [scrollRecr setAutoresizesSubviews:YES];

		    NSSize contentSize = [scrollRect contentSize];
    		editorView = [[NSTextView alloc] initWithFrame:NSMakeRect(0, 0, contentSize.width, contentSize.height)];
   		    [editorView setMinSize:NSMakeSize(0.0, contentSize.height)];
   			[editorView setMaxSize:NSMakeSize(FLT_MAX, FLT_MAX)];
   			[editorView setVerticallyResizable:YES];
   			[editorView setHorizontallyResizable:NO];
		    [editorView setFont:[NSFont userFixedPitchFontOfSize:12.0]];

		    [scrollRect setDocumentView:editorView];
		    [contentView addSubview:scrollRect];

		    statusLabel = [[NSTextField alloc] initWithFrame:NSMakeRect(10, 10, 620, 30)];
		    [statusLabel setEditable:NO];
		    [statusLabel setBordered:YES];
		    [statusLabel setStringValue:@"Status: Idle"];
		    [contentView addSubview:statusLabel];

		    [mainWindow makeKeyAndOrderFront:nil];
		}

		- (void)runBuildScript:(id)sender {
		    [statusLabel setStringValue:@"Status: Compiling Kernel Modules..."]; 
			NSTask *task = [[NSTask alloc] init];
			[task setLaunchPath:@"/bin/sh"];
			[task setArguments:@[@"-c", @"echo 'Building...' && sleep 1 && echo 'Done'"]];

			[task launch];
			[task waitUntilExit];

			if ([task terminationStatus] == 0) {
				[statusLabel setStringValue:@"Status: Build success."];
			} else {
				[statusLabel setStringValue:@"Status: Build failed."];
			}
		}
	@end

	int main(int argc, const char * argv[]) {
		@autoreleasepool {
			NSApplication *app = [NSApplication sharedApplication];

			PWBController *controller = [[PWDController alloc] init];
			[app setDelegate:controller];

			[app run];
		}
		return 0;
	}
#elif defined(NX_CURRENT_COMPILER_RELEASE) && defined(NX_CURRENT_COMPILER_RELEASE >= 400)
	#import <Foundation/Foundation.h>

	int main() {
		NSLog(@"Under construction!");
		return 0;
	}
#elif defined(NX_CURRENT_COMPILER_RELEASE) && defined(NX_CURRENT_COMPILER_RELEASE <= 400)
	#objc
	#import <appkit/appkit.h>

	NXRunAlertPanel(
		"Notice",
		"Under construction!",
		"OK",
		NULL,
		NULL
	);
#endif
