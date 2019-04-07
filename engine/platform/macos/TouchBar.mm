//
// Created by 小白白 on 2019-04-05.
//

#import <AppKit/AppKit.h>
#include "common.h"

// This example shows how to create and populate touch bars for Qt applications.
// Two approaches are demonstrated: creating a global touch bar for the entire
// application via the NSApplication delegate, and creating per-window touch bars
// via the NSWindow delegate. Applications may use either or both of these, for example
// to provide global base touch bar with window specific additions. Refer to the
// NSTouchBar documentation for further details.

// The TouchBarProvider class implements the NSTouchBarDelegate protocol, as
// well as app and window delegate protocols.
@interface TouchBarProvider: NSResponder <NSTouchBarDelegate, NSApplicationDelegate, NSWindowDelegate>

@property (strong) NSCustomTouchBarItem *touchBarItem1;
@property (strong) NSCustomTouchBarItem *touchBarItem2;
@property (strong) NSCustomTouchBarItem *touchBarItem3;
@property (strong) NSCustomTouchBarItem *touchBarItem4;

@property (strong) NSButton *touchBarButton1;
@property (strong) NSButton *touchBarButton2;
@property (strong) NSButton *touchBarButton3;
@property (strong) NSButton *touchBarButton4;

@property (strong) NSObject *qtDelegate;

@end

// Create identifiers for two button items.
static NSTouchBarItemIdentifier Button1Identifier = @"com.myapp.Button1Identifier";
static NSTouchBarItemIdentifier Button2Identifier = @"com.myapp.Button2Identifier";
static NSTouchBarItemIdentifier Button3Identifier = @"com.myapp.Button3Identifier";
static NSTouchBarItemIdentifier Button4Identifier = @"com.myapp.Button4Identifier";

@implementation TouchBarProvider

- (NSTouchBar *)makeTouchBar
{
	// Create the touch bar with this instance as its delegate
	NSTouchBar *bar = [[NSTouchBar alloc] init];
	bar.delegate = self;

	// Add touch bar items: first, the very important emoji picker, followed
	// by two buttons. Note that no further handling of the emoji picker
	// is needed (emojii are automatically routed to any active text edit). Button
	// actions handlers are set up in makeItemForIdentifier below.
	bar.defaultItemIdentifiers = @[
	//		NSTouchBarItemIdentifierCharacterPicker,
            Button1Identifier,
            Button2Identifier,
            Button3Identifier,
            Button4Identifier
	];

	return bar;
}

- (NSTouchBarItem *)touchBar:(NSTouchBar *)touchBar makeItemForIdentifier:(NSTouchBarItemIdentifier)identifier
{

	// Create touch bar items as NSCustomTouchBarItems which can contain any NSView.
	if ([identifier isEqualToString:Button1Identifier]) {
		NSString *title=[[NSString alloc]initWithString:@"1"];

		self.touchBarItem1 = [[[NSCustomTouchBarItem alloc] initWithIdentifier:identifier] autorelease];
		self.touchBarButton1 = [[NSButton buttonWithTitle:title target:self
		                                           action:@selector(button1Clicked)] autorelease];
		self.touchBarButton1.imageHugsTitle = true;
		self.touchBarItem1.view =  self.touchBarButton1;
		return self.touchBarItem1;
	} else if ([identifier isEqualToString:Button2Identifier]) {
		NSString *title=[[NSString alloc]initWithString:@"2"];
		self.touchBarItem2 = [[[NSCustomTouchBarItem alloc] initWithIdentifier:identifier] autorelease];
		self.touchBarButton2 = [[NSButton buttonWithTitle:title target:self
		                                           action:@selector(button2Clicked)] autorelease];
		self.touchBarButton2.imageHugsTitle = true;
		self.touchBarItem2.view =  self.touchBarButton2;
		return self.touchBarItem2;
	}
	else if ([identifier isEqualToString:Button3Identifier]) {
		NSString *title=[[NSString alloc]initWithString:@"3"];
		self.touchBarItem3 = [[[NSCustomTouchBarItem alloc] initWithIdentifier:identifier] autorelease];
		self.touchBarButton3 = [[NSButton buttonWithTitle:title target:self
		                                           action:@selector(button3Clicked)] autorelease];
		self.touchBarButton3.imageHugsTitle = true;
		self.touchBarItem3.view =  self.touchBarButton3;
		return self.touchBarItem3;
	}
	else if ([identifier isEqualToString:Button4Identifier]) {
		NSString *title=[[NSString alloc]initWithString:@"4"];
		self.touchBarItem4 = [[[NSCustomTouchBarItem alloc] initWithIdentifier:identifier] autorelease];
		self.touchBarButton4 = [[NSButton buttonWithTitle:title target:self
		                                           action:@selector(button4Clicked)] autorelease];
		self.touchBarButton4.imageHugsTitle = true;
		self.touchBarItem4.view =  self.touchBarButton4;
		return self.touchBarItem4;
	}
	return nil;
}

- (void)installAsDelegateForWindow:(NSWindow *)window
{
	_qtDelegate = window.delegate; // Save current delegate for forwarding
	window.delegate = self;
}

- (void)installAsDelegateForApplication:(NSApplication *)application
{
	_qtDelegate = application.delegate; // Save current delegate for forwarding
	application.delegate = self;
}

- (BOOL)respondsToSelector:(SEL)aSelector
{
	// We want to forward to the qt delegate. Respond to selectors it
	// responds to in addition to selectors this instance resonds to.
	return [_qtDelegate respondsToSelector:aSelector] || [super respondsToSelector:aSelector];
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	// Forward to the existing delegate. This function is only called for selectors
	// this instance does not responds to, which means that the qt delegate
	// must respond to it (due to the respondsToSelector implementation above).
	[anInvocation invokeWithTarget:_qtDelegate];
}

- (void)button1Clicked
{
	Cbuf_AddText( "slot1\n" );
}

- (void)button2Clicked
{
	Cbuf_AddText( "slot2\n" );
}

- (void)button3Clicked
{
	Cbuf_AddText( "slot3\n" );
}

- (void)button4Clicked
{
	Cbuf_AddText( "slot4\n" );
}

@end

extern "C" void InstallTouchBar(NSWindow *nswin)
{

	{
		// Install TouchBarProvider as application delegate
		TouchBarProvider *touchBarProvider = [[TouchBarProvider alloc] init];
		[touchBarProvider installAsDelegateForApplication:[NSApplication sharedApplication]];
	}

	{
		// Install TouchBarProvider as window delegate
		TouchBarProvider *touchBarProvider = [[TouchBarProvider alloc] init];
		[touchBarProvider installAsDelegateForWindow:nswin];
	}
}
