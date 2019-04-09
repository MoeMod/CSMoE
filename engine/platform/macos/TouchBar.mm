/*
TouchBar.mm - TouchBar support for macOS
Copyright (C) 2019 Moemod Hymei

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#import <AppKit/AppKit.h>

#include "common.h"
#include "crtlib.h"

// This example shows how to create and populate touch bars for Qt applications.
// Two approaches are demonstrated: creating a global touch bar for the entire
// application via the NSApplication delegate, and creating per-window touch bars
// via the NSWindow delegate. Applications may use either or both of these, for example
// to provide global base touch bar with window specific additions. Refer to the
// NSTouchBar documentation for further details.

// Create identifiers for two button items.
static NSTouchBarItemIdentifier MouseSensitivityItemIdentifier = @"com.csmoe.MouseSensitivityItemIdentifier";
static NSTouchBarItemIdentifier MouseSensitivitySliderItemIdentifier = @"com.csmoe.MouseSensitivitySliderItemIdentifier";
static NSTouchBarItemIdentifier CrosshairTypeItemIdentifier = @"com.csmoe.CrosshairTypeItemIdentifier";
static NSTouchBarItemIdentifier CrosshairTypeScrubberItemIdentifier = @"com.csmoe.CrosshairTypeScrubberItemIdentifier";
static NSTouchBarItemIdentifier CrosshairColorPickerItemIdentifier = @"com.csmoe.CrosshairColorPickerItemIdentifier";
static NSTouchBarItemIdentifier RightHandIdentifier = @"com.myapp.RightHandIdentifier";
static NSTouchBarItemIdentifier Button1Identifier = @"com.myapp.Button1Identifier";
static NSTouchBarItemIdentifier Button2Identifier = @"com.myapp.Button2Identifier";
static NSTouchBarItemIdentifier Button3Identifier = @"com.myapp.Button3Identifier";
static NSTouchBarItemIdentifier Button4Identifier = @"com.myapp.Button4Identifier";
static NSTouchBarItemIdentifier TextItemIdentifier = @"com.myapp.TextItemIdentifier";

@interface CrosshairTypeScrubberDelegate : NSResponder <NSScrubberDelegate, NSScrubberDataSource>
@property (strong) NSArray *fontNames;
@end
@implementation CrosshairTypeScrubberDelegate
- (NSInteger)numberOfItemsForScrubber:(NSScrubber *)scrubber {
	return self.fontNames.count;
}

- (NSScrubberItemView *)scrubber:(NSScrubber *)scrubber viewForItemAtIndex:(NSInteger)index {
	NSScrubberTextItemView *view = [scrubber makeItemWithIdentifier:TextItemIdentifier owner:nil];
	view.textField.stringValue = self.fontNames[index];

	return view;
}

- (NSSize)scrubber:(NSScrubber *)scrubber layout:(NSScrubberFlowLayout *)layout sizeForItemAtIndex:(NSInteger)itemIndex {
	NSString *string = self.fontNames[itemIndex];
	NSRect bounds = [string boundingRectWithSize:NSMakeSize(CGFLOAT_MAX, CGFLOAT_MAX)
	                                     options:NSStringDrawingUsesLineFragmentOrigin | NSStringDrawingUsesFontLeading
	                                  attributes:@{NSFontAttributeName: [NSFont systemFontOfSize:0]}];

	return NSMakeSize(bounds.size.width + 20, 30);
}

- (void)scrubber:(NSScrubber *)scrubber didSelectItemAtIndex:(NSInteger)selectedIndex {
	Cvar_SetFloat("cl_crosshair_type", static_cast<int>(selectedIndex));
}
@end

@interface CrosshairTypeTouchBarItem: NSCustomTouchBarItem

@end
@implementation CrosshairTypeTouchBarItem

- (instancetype)initWithIdentifier:(NSTouchBarItemIdentifier)identifier {
	self = [super initWithIdentifier:identifier];
	if (self) {

		CrosshairTypeScrubberDelegate *scrubberDelegate = [[CrosshairTypeScrubberDelegate alloc] init];
		scrubberDelegate.fontNames = @[@"Cross", @"Cross + Dot", @"Circle + Dot", @"Cross + Circle + Dot", @"Dot Only"];

		NSScrubber *scrubber = [[NSScrubber alloc] init];
		scrubber.scrubberLayout = [[NSScrubberFlowLayout alloc] init];
		//scrubber.mode = NSScrubberModeFree;
		scrubber.continuous = true;
		scrubber.selectionBackgroundStyle = [NSScrubberSelectionStyle outlineOverlayStyle];
		scrubber.delegate = scrubberDelegate;
		scrubber.dataSource = scrubberDelegate;
		scrubber.floatsSelectionViews = false;
		scrubber.selectedIndex = Cvar_VariableInteger("cl_crosshair_type");

		[scrubber registerClass:[NSScrubberTextItemView class] forItemIdentifier:TextItemIdentifier];


		self.view = scrubber;
	}
	return self;
}


@end

// The TouchBarProvider class implements the NSTouchBarDelegate protocol, as
// well as app and window delegate protocols.
@interface TouchBarProvider: NSResponder <NSTouchBarDelegate, NSApplicationDelegate, NSWindowDelegate>

@property (strong) NSSliderTouchBarItem *touchBarMouseSensitivity;
@property (strong) NSColorPickerTouchBarItem *touchBarCrosshairColor;
@property (strong) NSCustomTouchBarItem *touchBarItem1;
@property (strong) NSCustomTouchBarItem *touchBarItem2;
@property (strong) NSCustomTouchBarItem *touchBarItem3;
@property (strong) NSCustomTouchBarItem *touchBarItem4;

@property (strong) NSButton *touchBarButtonRightHanded;
@property (strong) NSButton *touchBarButton1;
@property (strong) NSButton *touchBarButton2;
@property (strong) NSButton *touchBarButton3;
@property (strong) NSButton *touchBarButton4;

@property (strong) NSObject *qtDelegate;

@end

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
			MouseSensitivityItemIdentifier,
			CrosshairTypeItemIdentifier,
			CrosshairColorPickerItemIdentifier,
			RightHandIdentifier,
    //      Button1Identifier,
    //      Button2Identifier,
    //      Button3Identifier,
    //      Button4Identifier
	];

	return bar;
}

- (NSTouchBarItem *)touchBar:(NSTouchBar *)touchBar makeItemForIdentifier:(NSTouchBarItemIdentifier)identifier
{

	// Create touch bar items as NSCustomTouchBarItems which can contain any NSView.
	if ([identifier isEqualToString:MouseSensitivityItemIdentifier]) {
		NSPopoverTouchBarItem *item = [[NSPopoverTouchBarItem alloc] initWithIdentifier:identifier];
		item.collapsedRepresentationLabel = @"Mouse Sense";
		item.showsCloseButton = YES;
		NSImage *img = [[NSWorkspace sharedWorkspace] iconForFileType:NSFileTypeForHFSTypeCode(kGenericPreferencesIcon)];
		//item.collapsedRepresentationImage = [NSImage imageNamed:NSImageNameTouchBarRewindTemplate];
		//item.collapsedRepresentationImage = img;

		NSTouchBar *secondaryTouchBar = [[NSTouchBar alloc] init];
		secondaryTouchBar.delegate = self;
		secondaryTouchBar.defaultItemIdentifiers = @[MouseSensitivitySliderItemIdentifier];

		item.pressAndHoldTouchBar = secondaryTouchBar;
		item.popoverTouchBar = secondaryTouchBar;

		return item;
	}
	else if ([identifier isEqualToString:MouseSensitivitySliderItemIdentifier]) {
		self.touchBarMouseSensitivity = [[NSSliderTouchBarItem alloc] initWithIdentifier:identifier];
		self.touchBarMouseSensitivity.label = @"Mouse Sensitivity";
		self.touchBarMouseSensitivity.slider.minValue = 0.1;
		self.touchBarMouseSensitivity.slider.maxValue = 10.0;
		self.touchBarMouseSensitivity.slider.floatValue = Cvar_VariableValue("sensitivity");
		self.touchBarMouseSensitivity.slider.target = self;
		self.touchBarMouseSensitivity.slider.action = @selector(sensitivityDidChange);
		[self.touchBarMouseSensitivity.slider addConstraint:[NSLayoutConstraint constraintWithItem:self.touchBarMouseSensitivity.slider attribute:NSLayoutAttributeWidth relatedBy:NSLayoutRelationEqual toItem:nil attribute:NSLayoutAttributeNotAnAttribute multiplier:1.0 constant:250]];
		return self.touchBarMouseSensitivity;
	}
	else if ([identifier isEqualToString:CrosshairTypeItemIdentifier]) {
		NSPopoverTouchBarItem *item = [[NSPopoverTouchBarItem alloc] initWithIdentifier:identifier];
		item.collapsedRepresentationLabel = @"Crosshair Type";

		NSTouchBar *secondaryTouchBar = [[NSTouchBar alloc] init];
		secondaryTouchBar.delegate = self;
		secondaryTouchBar.defaultItemIdentifiers = @[CrosshairTypeScrubberItemIdentifier];

		item.pressAndHoldTouchBar = secondaryTouchBar;
		item.popoverTouchBar = secondaryTouchBar;

		return item;
	}
	else if ([identifier isEqualToString:CrosshairTypeScrubberItemIdentifier]) {
		CrosshairTypeTouchBarItem *item = [[CrosshairTypeTouchBarItem alloc] initWithIdentifier:identifier];

		return item;
	}
	else if ([identifier isEqualToString:CrosshairColorPickerItemIdentifier]) {
		self.touchBarCrosshairColor = [[NSColorPickerTouchBarItem alloc] initWithIdentifier:identifier];

		NSTouchBar *secondaryTouchBar = [[NSTouchBar alloc] init];
		secondaryTouchBar.delegate = self;
		secondaryTouchBar.defaultItemIdentifiers = @[MouseSensitivitySliderItemIdentifier];

		self.touchBarCrosshairColor.action = @selector(crosshairColorClicked);
		self.touchBarCrosshairColor.allowedColorSpaces = @[ [NSColorSpace sRGBColorSpace]];

		int r=0, g=0, b=0;
		sscanf(Cvar_VariableString("cl_crosshair_color"), "%d %d %d", &r, &g, &b);
		self.touchBarCrosshairColor.color = [NSColor colorWithRed:r/255.0f green:g/255.0f blue:b/255.0f alpha:1.0f];

		return self.touchBarCrosshairColor;
	}
	else if ([identifier isEqualToString:RightHandIdentifier]) {
		NSString *title=[[NSString alloc]initWithString:@"Right-handed"];

		NSCustomTouchBarItem *item = [[[NSCustomTouchBarItem alloc] initWithIdentifier:identifier] autorelease];

		self.touchBarButtonRightHanded = [[NSButton buttonWithTitle:title target:self
		                                           action:@selector(buttonRighthandedClicked)] autorelease];

		[self.touchBarButtonRightHanded setButtonType: NSButtonTypePushOnPushOff];
		self.touchBarButtonRightHanded.allowsMixedState = NO;
		self.touchBarButtonRightHanded.imageHugsTitle = true;

		self.touchBarButtonRightHanded.state = Cvar_VariableInteger("hand") ? NSControlStateValueOn : NSControlStateValueOff;

		item.view =  self.touchBarButtonRightHanded;
		return item;
	}
	else if ([identifier isEqualToString:Button1Identifier]) {
		NSString *title=[[NSString alloc]initWithString:@"1"];

		self.touchBarItem1 = [[[NSCustomTouchBarItem alloc] initWithIdentifier:identifier] autorelease];
		self.touchBarButton1 = [[NSButton buttonWithTitle:title target:self
		                                           action:@selector(button1Clicked)] autorelease];
		self.touchBarButton1.imageHugsTitle = true;
		self.touchBarItem1.view =  self.touchBarButton1;
		return self.touchBarItem1;
	}
	else if ([identifier isEqualToString:Button2Identifier]) {
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

- (void)sensitivityDidChange
{
	Cvar_SetFloat("sensitivity", self.touchBarMouseSensitivity.slider.floatValue);
}

- (void)crosshairColorClicked
{
	char buffer[64];

	CGFloat r, g, b, a;
	[self.touchBarCrosshairColor.color getRed:&r green:&g blue:&b alpha:&a];

	sprintf(buffer, "%d %d %d",
	        static_cast<int>(r * 255),
	        static_cast<int>(g * 255),
			static_cast<int>(b * 255)
	);
	Cvar_Set("cl_crosshair_color", buffer);
}

- (void)buttonRighthandedClicked
{
	BOOL isOn = static_cast<BOOL>(self.touchBarButtonRightHanded.state);
	Cvar_SetFloat("hand", isOn);
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

extern "C" void TouchBar_Install()
{
	// Install TouchBarProvider as application delegate
	TouchBarProvider *touchBarProvider = [[TouchBarProvider alloc] init];
	[touchBarProvider installAsDelegateForApplication:[NSApplication sharedApplication]];
}
