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
#include <string>
#include <sstream>

// This example shows how to create and populate touch bars for Qt applications.
// Two approaches are demonstrated: creating a global touch bar for the entire
// application via the NSApplication delegate, and creating per-window touch bars
// via the NSWindow delegate. Applications may use either or both of these, for example
// to provide global base touch bar with window specific additions. Refer to the
// NSTouchBar documentation for further details.

// Create identifiers for two button items.
static NSTouchBarItemIdentifier EscapeButtonItemIdentifier = @"com.csmoe.EscapeButtonItemIdentifier";
static NSTouchBarItemIdentifier MouseSensitivityItemIdentifier = @"com.csmoe.MouseSensitivityItemIdentifier";
static NSTouchBarItemIdentifier MouseSensitivitySliderItemIdentifier = @"com.csmoe.MouseSensitivitySliderItemIdentifier";
static NSTouchBarItemIdentifier MouseZoomSensitivityRatioSliderItemIdentifier = @"com.csmoe.MouseZoomSensitivityRatioSliderItemIdentifier";
static NSTouchBarItemIdentifier MouseFilterButtonItemIdentifier = @"com.csmoe.MouseFilterButtonItemIdentifier";
static NSTouchBarItemIdentifier CrosshairTypeItemIdentifier = @"com.csmoe.CrosshairTypeItemIdentifier";
static NSTouchBarItemIdentifier CustomizeItemIdentifier = @"com.csmoe.CustomizeItemIdentifier";
static NSTouchBarItemIdentifier CrosshairTypeScrubberItemIdentifier = @"com.csmoe.CrosshairTypeScrubberItemIdentifier";
static NSTouchBarItemIdentifier CrosshairColorPickerItemIdentifier = @"com.csmoe.CrosshairColorPickerItemIdentifier";
static NSTouchBarItemIdentifier RightHandButtonItemIdentifier = @"com.myapp.RightHandButtonItemIdentifier";
static NSTouchBarItemIdentifier FastSwitchButtonItemIdentifier = @"com.myapp.FastSwitchButtonItemIdentifier";
static NSTouchBarItemIdentifier AutoSwitchButtonItemIdentifier = @"com.myapp.AutoSwitchButtonItemIdentifier";
static NSTouchBarItemIdentifier ShadowButtonItemIdentifier = @"com.myapp.ShadowButtonItemIdentifier";
static NSTouchBarItemIdentifier WeatherButtonItemIdentifier = @"com.myapp.WeatherButtonItemIdentifier";
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

		NSScrubber *scrubber = [[[NSScrubber alloc] init] autorelease];
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
@property (strong) NSSliderTouchBarItem *touchBarMouseZoomSensitivityRatio;
@property (strong) NSColorPickerTouchBarItem *touchBarCrosshairColor;

@property (strong) NSButton *touchBarButtonRightHanded;
@property (strong) NSButton *touchBarButtonAutoSwitch;
@property (strong) NSButton *touchBarButtonFastSwitch;
@property (strong) NSButton *touchBarButtonWeather;
@property (strong) NSButton *touchBarButtonShadow;
@property (strong) NSButton *touchBarButtonMouseFilter;

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
			CustomizeItemIdentifier
    //      Button1Identifier,
    //      Button2Identifier,
    //      Button3Identifier,
    //      Button4Identifier
	];

	bar.escapeKeyReplacementItemIdentifier = EscapeButtonItemIdentifier;

	return bar;
}

- (NSTouchBarItem *)touchBar:(NSTouchBar *)touchBar makeItemForIdentifier:(NSTouchBarItemIdentifier)identifier
{
	// Create touch bar items as NSCustomTouchBarItems which can contain any NSView.
	if ([identifier isEqualToString:MouseSensitivityItemIdentifier]) {
		NSPopoverTouchBarItem *item = [[NSPopoverTouchBarItem alloc] initWithIdentifier:identifier];
		item.collapsedRepresentationLabel = @"Mouse";
		item.showsCloseButton = YES;
		//NSImage *img = [[NSWorkspace sharedWorkspace] iconForFileType:NSFileTypeForHFSTypeCode(kGenericPreferencesIcon)];
		//item.collapsedRepresentationImage = [NSImage imageNamed:NSImageNameTouchBarRewindTemplate];
		//item.collapsedRepresentationImage = [[NSImage alloc] initByReferencingFile:@"/System/Library/PreferencePanes/Mouse.prefPane/Contents/Resources/Mouse.icns"];

		NSTouchBar *secondaryTouchBar = [[NSTouchBar alloc] init];
		secondaryTouchBar.delegate = self;
		secondaryTouchBar.defaultItemIdentifiers = @[MouseSensitivitySliderItemIdentifier, MouseZoomSensitivityRatioSliderItemIdentifier, MouseFilterButtonItemIdentifier];

		NSTouchBar *secondaryTouchBar2 = [[NSTouchBar alloc] init];
		secondaryTouchBar2.delegate = self;
		secondaryTouchBar2.defaultItemIdentifiers = @[MouseSensitivitySliderItemIdentifier];

		item.pressAndHoldTouchBar = secondaryTouchBar2;
		item.popoverTouchBar = secondaryTouchBar;
		return item;
	}
	else if ([identifier isEqualToString:MouseSensitivitySliderItemIdentifier]) {
		self.touchBarMouseSensitivity = [[NSSliderTouchBarItem alloc] initWithIdentifier:identifier];
		self.touchBarMouseSensitivity.label = @"Sensitivity";
		self.touchBarMouseSensitivity.slider.minValue = 0.1;
		self.touchBarMouseSensitivity.slider.maxValue = 10.0;
		self.touchBarMouseSensitivity.slider.floatValue = Cvar_VariableValue("sensitivity");
		self.touchBarMouseSensitivity.slider.target = self;
		self.touchBarMouseSensitivity.slider.action = @selector(sensitivityDidChange);
		[self.touchBarMouseSensitivity.slider addConstraint:[NSLayoutConstraint constraintWithItem:self.touchBarMouseSensitivity.slider attribute:NSLayoutAttributeWidth relatedBy:NSLayoutRelationLessThanOrEqual toItem:nil attribute:NSLayoutAttributeNotAnAttribute multiplier:1.0 constant:250]];
		return self.touchBarMouseSensitivity;
	}
	else if ([identifier isEqualToString:MouseZoomSensitivityRatioSliderItemIdentifier]) {
		self.touchBarMouseZoomSensitivityRatio = [[NSSliderTouchBarItem alloc] initWithIdentifier:identifier];
		self.touchBarMouseZoomSensitivityRatio.label = @"Zoom Ratio";
		self.touchBarMouseZoomSensitivityRatio.slider.minValue = 0.0;
		self.touchBarMouseZoomSensitivityRatio.slider.maxValue = 2.0;
		self.touchBarMouseZoomSensitivityRatio.slider.floatValue = Cvar_VariableValue("zoom_sensitivity_ratio");
		self.touchBarMouseZoomSensitivityRatio.slider.target = self;
		self.touchBarMouseZoomSensitivityRatio.slider.action = @selector(sensitivityZoomRatioDidChange);
		[self.touchBarMouseZoomSensitivityRatio.slider addConstraint:[NSLayoutConstraint constraintWithItem:self.touchBarMouseZoomSensitivityRatio.slider attribute:NSLayoutAttributeWidth relatedBy:NSLayoutRelationLessThanOrEqual toItem:nil attribute:NSLayoutAttributeNotAnAttribute multiplier:1.0 constant:250]];
		return self.touchBarMouseZoomSensitivityRatio;
	}
	else if ([identifier isEqualToString:MouseFilterButtonItemIdentifier]) {
		NSString *title=[[NSString alloc]initWithString:@"Filter"];
		NSCustomTouchBarItem *item = [[[NSCustomTouchBarItem alloc] initWithIdentifier:identifier] autorelease];
		self.touchBarButtonMouseFilter = [[NSButton buttonWithTitle:title target:self
		                                  action:@selector(buttonMouseFilterClicked)] autorelease];

		[self.touchBarButtonMouseFilter setButtonType: NSButtonTypePushOnPushOff];
		self.touchBarButtonMouseFilter.allowsMixedState = NO;
		self.touchBarButtonMouseFilter.imageHugsTitle = true;

		self.touchBarButtonMouseFilter.state = Cvar_VariableInteger("m_filter") ? NSControlStateValueOn : NSControlStateValueOff;

		item.view =  self.touchBarButtonMouseFilter;
		return item;
	}
	else if ([identifier isEqualToString:CrosshairTypeItemIdentifier]) {
		NSPopoverTouchBarItem *item = [[NSPopoverTouchBarItem alloc] initWithIdentifier:identifier];

		item.collapsedRepresentationLabel = @"Crosshair";

		NSTouchBar *secondaryTouchBar = [[NSTouchBar alloc] init];
		secondaryTouchBar.delegate = self;
		secondaryTouchBar.defaultItemIdentifiers = @[CrosshairTypeScrubberItemIdentifier];

		item.popoverTouchBar = secondaryTouchBar;

		return item;
	}
	else if ([identifier isEqualToString:CrosshairTypeScrubberItemIdentifier]) {
		CrosshairTypeTouchBarItem *item = [[CrosshairTypeTouchBarItem alloc] initWithIdentifier:identifier];

		return item;
	}
	else if ([identifier isEqualToString:CrosshairColorPickerItemIdentifier]) {
		self.touchBarCrosshairColor = [NSColorPickerTouchBarItem strokeColorPickerWithIdentifier:identifier];

		self.touchBarCrosshairColor.action = @selector(crosshairColorClicked);
		self.touchBarCrosshairColor.allowedColorSpaces = @[ [NSColorSpace sRGBColorSpace]];

		int r=0, g=0, b=0;
		std::istringstream(Cvar_VariableString("cl_crosshair_color")) >> r >> g >> b;
		//sscanf(Cvar_VariableString("cl_crosshair_color"), "%d %d %d", &r, &g, &b);
		self.touchBarCrosshairColor.color = [NSColor colorWithRed:r/255.0f green:g/255.0f blue:b/255.0f alpha:1.0f];

		return self.touchBarCrosshairColor;
	}
	else if ([identifier isEqualToString:CustomizeItemIdentifier]) {
		NSPopoverTouchBarItem *item = [[NSPopoverTouchBarItem alloc] initWithIdentifier:identifier];
		item.collapsedRepresentationLabel = @"Customize";

		NSTouchBar *secondaryTouchBar = [[NSTouchBar alloc] init];
		secondaryTouchBar.delegate = self;
		secondaryTouchBar.defaultItemIdentifiers = @[RightHandButtonItemIdentifier,
		                                             FastSwitchButtonItemIdentifier,
		                                             AutoSwitchButtonItemIdentifier,
		                                             ShadowButtonItemIdentifier,
		                                             WeatherButtonItemIdentifier
		];

		//item.pressAndHoldTouchBar = secondaryTouchBar;
		item.popoverTouchBar = secondaryTouchBar;

		return item;
	}
	else if ([identifier isEqualToString:RightHandButtonItemIdentifier]) {
		NSString *title=[[NSString alloc]initWithString:@"Right-hand"];

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
	else if ([identifier isEqualToString:AutoSwitchButtonItemIdentifier]) {
		NSString *title=[[NSString alloc]initWithString:@"Auto-switch"];

		NSCustomTouchBarItem *item = [[[NSCustomTouchBarItem alloc] initWithIdentifier:identifier] autorelease];

		self.touchBarButtonAutoSwitch = [[NSButton buttonWithTitle:title target:self
		                                                     action:@selector(buttonAutoSwitchClicked)] autorelease];

		[self.touchBarButtonAutoSwitch setButtonType: NSButtonTypePushOnPushOff];
		self.touchBarButtonAutoSwitch.allowsMixedState = NO;
		self.touchBarButtonAutoSwitch.imageHugsTitle = true;

		self.touchBarButtonAutoSwitch.state = Cvar_VariableInteger("_cl_autowepswitch") ? NSControlStateValueOn : NSControlStateValueOff;

		item.view =  self.touchBarButtonAutoSwitch;
		return item;
	}
	else if ([identifier isEqualToString:FastSwitchButtonItemIdentifier]) {
		NSString *title=[[NSString alloc]initWithString:@"Fast-switch"];

		NSCustomTouchBarItem *item = [[[NSCustomTouchBarItem alloc] initWithIdentifier:identifier] autorelease];

		self.touchBarButtonFastSwitch = [[NSButton buttonWithTitle:title target:self
		                                                     action:@selector(buttonFastSwitchClicked)] autorelease];

		[self.touchBarButtonFastSwitch setButtonType: NSButtonTypePushOnPushOff];
		self.touchBarButtonFastSwitch.allowsMixedState = NO;
		self.touchBarButtonFastSwitch.imageHugsTitle = true;

		self.touchBarButtonFastSwitch.state = Cvar_VariableInteger("hud_fastswitch") ? NSControlStateValueOn : NSControlStateValueOff;

		item.view =  self.touchBarButtonFastSwitch;
		return item;
	}
	else if ([identifier isEqualToString:ShadowButtonItemIdentifier]) {
		NSString *title=[[NSString alloc]initWithString:@"Shadow"];

		NSCustomTouchBarItem *item = [[[NSCustomTouchBarItem alloc] initWithIdentifier:identifier] autorelease];

		self.touchBarButtonShadow = [[NSButton buttonWithTitle:title target:self
		                                                    action:@selector(buttonShadowClicked)] autorelease];

		[self.touchBarButtonShadow setButtonType: NSButtonTypePushOnPushOff];
		self.touchBarButtonShadow.allowsMixedState = NO;
		self.touchBarButtonShadow.imageHugsTitle = true;

		self.touchBarButtonShadow.state = Cvar_VariableInteger("cl_shadows") ? NSControlStateValueOn : NSControlStateValueOff;

		item.view =  self.touchBarButtonShadow;
		return item;
	}
	else if ([identifier isEqualToString:WeatherButtonItemIdentifier]) {
		NSString *title=[[NSString alloc]initWithString:@"Weather"];

		NSCustomTouchBarItem *item = [[[NSCustomTouchBarItem alloc] initWithIdentifier:identifier] autorelease];

		self.touchBarButtonWeather = [[NSButton buttonWithTitle:title target:self
		                                                    action:@selector(buttonWeatherClicked)] autorelease];

		[self.touchBarButtonWeather setButtonType: NSButtonTypePushOnPushOff];
		self.touchBarButtonWeather.allowsMixedState = NO;
		self.touchBarButtonWeather.imageHugsTitle = true;

		self.touchBarButtonWeather.state = Cvar_VariableInteger("cl_weather") ? NSControlStateValueOn : NSControlStateValueOff;

		item.view =  self.touchBarButtonWeather;
		return item;
	}
	else if ([identifier isEqualToString:EscapeButtonItemIdentifier]) {
		NSString *title=[[NSString alloc]initWithString:@"Menu"];

		NSCustomTouchBarItem *item = [[[NSCustomTouchBarItem alloc] initWithIdentifier:identifier] autorelease];
		NSButton *button = [[NSButton buttonWithTitle:title target:self
		                                           action:@selector(buttonESCClicked)] autorelease];

		NSImage *img = [NSImage imageNamed:NSImageNameTouchBarSidebarTemplate];
		button.image = img;
		button.imageHugsTitle = true;
		item.view =  button;
		return item;
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

- (void)sensitivityZoomRatioDidChange
{
	Cvar_SetFloat("zoom_sensitivity_ratio", self.touchBarMouseZoomSensitivityRatio.slider.floatValue);
}

- (void)buttonMouseFilterClicked
{
	BOOL isOn = static_cast<BOOL>(self.touchBarButtonMouseFilter.state);
	Cvar_SetFloat("m_filter", isOn);
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

- (void)buttonAutoSwitchClicked
{
	BOOL isOn = static_cast<BOOL>(self.touchBarButtonAutoSwitch.state);
	Cvar_SetFloat("_cl_autowepswitch", isOn);
}

- (void)buttonFastSwitchClicked
{
	BOOL isOn = static_cast<BOOL>(self.touchBarButtonFastSwitch.state);
	Cvar_SetFloat("hud_fastswitch", isOn);
}

- (void)buttonShadowClicked
{
	BOOL isOn = static_cast<BOOL>(self.touchBarButtonFastSwitch.state);
	Cvar_SetFloat("cl_shadows", isOn);
}

- (void)buttonWeatherClicked
{
	BOOL isOn = static_cast<BOOL>(self.touchBarButtonFastSwitch.state);
	Cvar_SetFloat("cl_weather", isOn);
}

- (void)buttonESCClicked
{
	Cbuf_AddText( "escape\n" );
}

@end

extern "C" void TouchBar_Install()
{
	// Install TouchBarProvider as application delegate
	TouchBarProvider *touchBarProvider = [[TouchBarProvider alloc] init];
	[touchBarProvider installAsDelegateForApplication:[NSApplication sharedApplication]];
}
