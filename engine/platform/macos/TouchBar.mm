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
#include <string>
#include <sstream>
#include <utility>

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
static NSTouchBarItemIdentifier CrosshairGroupItemIdentifier = @"com.csmoe.CrosshairGroupItemIdentifier";
static NSTouchBarItemIdentifier CrosshairTypeItemIdentifier = @"com.csmoe.CrosshairTypeItemIdentifier";
static NSTouchBarItemIdentifier CustomizeItemIdentifier = @"com.csmoe.CustomizeItemIdentifier";
static NSTouchBarItemIdentifier CrosshairTypeScrubberItemIdentifier = @"com.csmoe.CrosshairTypeScrubberItemIdentifier";
static NSTouchBarItemIdentifier CrosshairColorPickerItemIdentifier = @"com.csmoe.CrosshairColorPickerItemIdentifier";
static NSTouchBarItemIdentifier RightHandButtonItemIdentifier = @"com.csmoe.CvarToggleButtonItemIdentifier.cl_crosshair_color";
static NSTouchBarItemIdentifier FastSwitchButtonItemIdentifier = @"com.csmoe.CvarToggleButtonItemIdentifier.hud_fastswitch";
static NSTouchBarItemIdentifier AutoSwitchButtonItemIdentifier = @"com.csmoe.CvarToggleButtonItemIdentifier._cl_autowepswitch";
static NSTouchBarItemIdentifier ShadowButtonItemIdentifier = @"com.csmoe.CvarToggleButtonItemIdentifier.cl_shadows";
static NSTouchBarItemIdentifier WeatherButtonItemIdentifier = @"com.csmoe.CvarToggleButtonItemIdentifier.cl_weather";
static NSTouchBarItemIdentifier TextItemIdentifier = @"com.csmoe.TextItemIdentifier";

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

@property(nonatomic, retain) CrosshairTypeScrubberDelegate *scrubberDelegate;
@end
@implementation CrosshairTypeTouchBarItem

- (instancetype)initWithIdentifier:(NSTouchBarItemIdentifier)identifier {
	self = [super initWithIdentifier:identifier];
	if (self) {

		CrosshairTypeScrubberDelegate *scrubberDelegate = [[CrosshairTypeScrubberDelegate alloc] init];
		[self setScrubberDelegate:scrubberDelegate];
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

@interface CvarButtonCallback : NSObject
-(instancetype) initWithCvar:(const char *)cvar Button:(NSButton *)button;
-(SEL) selector;
-(void) updateButtonState;
@property (weak) NSButton *button;
@property std::string cvar;
@end
@implementation CvarButtonCallback
-(IBAction) action:(id)sender
{
	Cbuf_AddText( ([self cvar] + '\n' ).c_str() );
}
- (instancetype)initWithCvar:(const char *)cvar Button:(NSButton *)button
{
	if([super init])
	{
		[self setButton:button];
		[self setCvar:cvar];
		return self;
	}
	return nil;
}
- (SEL)selector {
	return @selector(action:);
}
-(void) updateButtonState {
}
@end

@interface CvarToggleButtonCallback : CvarButtonCallback
@end
@implementation CvarToggleButtonCallback
-(IBAction) action:(id)sender
{
	BOOL isOn = static_cast<BOOL>([self button].state);
	Cvar_SetFloat([self cvar].c_str(), isOn);
}
-(void) updateButtonState
{
	self.button.state = Cvar_VariableInteger(self.cvar.c_str()) ? NSControlStateValueOn : NSControlStateValueOff;
}
@end

// The TouchBarProvider class implements the NSTouchBarDelegate protocol, as
// well as app and window delegate protocols.
@interface TouchBarProvider: NSResponder <NSTouchBarDelegate, NSApplicationDelegate, NSWindowDelegate>

-(instancetype) init;

@property (strong) NSSliderTouchBarItem *touchBarMouseSensitivity;
@property (strong) NSSliderTouchBarItem *touchBarMouseZoomSensitivityRatio;
@property (strong) NSColorPickerTouchBarItem *touchBarCrosshairColor;

@property (strong) NSMapTable<NSTouchBarItemIdentifier, CvarToggleButtonCallback *> *callbackMap;

@property (strong) id previousDelegate;

@end

@implementation TouchBarProvider
- (instancetype)init {
	self = [super init];
	if(self)
	{
		self.callbackMap = [[NSMapTable alloc] init];
	}
	return self;
}

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
			NSTouchBarItemIdentifierFixedSpaceSmall,
			CrosshairGroupItemIdentifier,
			NSTouchBarItemIdentifierFlexibleSpace,
			CustomizeItemIdentifier
	];

	bar.escapeKeyReplacementItemIdentifier = EscapeButtonItemIdentifier;

	return bar;
}

- (NSTouchBarItem *)buttonWithCvarButtonCallback:(CvarButtonCallback *)callback Identifier:(NSTouchBarItemIdentifier)identifier Title:(NSString *)title
{
	NSCustomTouchBarItem *item = [[NSCustomTouchBarItem alloc] initWithIdentifier:identifier];
	NSButton *button = [NSButton buttonWithTitle:title target:callback action:[callback selector]];
	[callback setButton:button];

	[button setButtonType: NSButtonTypePushOnPushOff];
	button.allowsMixedState = NO;
	button.imageHugsTitle = true;

	[callback updateButtonState];

	item.view =  button;
	return item;
}

- (NSTouchBarItem *)cvarToggleButtonTouchBarItemWithIdentifier:(NSTouchBarItemIdentifier)identifier Cvar:(const char *)cvar Title:(NSString *)title
{
	CvarToggleButtonCallback *callback = [[self callbackMap] objectForKey:identifier];
	if(!callback)
	{
		callback = [[CvarToggleButtonCallback alloc] initWithCvar:cvar Button:nil];
		[[self callbackMap] setObject:callback forKey:identifier];
	}
	return [self buttonWithCvarButtonCallback:callback Identifier:identifier Title:title];
}

- (NSTouchBarItem *)touchBar:(NSTouchBar *)touchBar makeItemForIdentifier:(NSTouchBarItemIdentifier)identifier
{
	// Create touch bar items as NSCustomTouchBarItems which can contain any NSView.
	if ([identifier isEqualToString:MouseSensitivityItemIdentifier]) {
		NSPopoverTouchBarItem *item = [[NSPopoverTouchBarItem alloc] initWithIdentifier:identifier];
		item.collapsedRepresentationLabel = @"<🖱>";
		//NSImage *img = [[NSWorkspace sharedWorkspace] iconForFileType:NSFileTypeForHFSTypeCode(kGenericPreferencesIcon)];
		//item.collapsedRepresentationImage = [NSImage imageNamed:NSImageNameTouchBarRewindTemplate];
		//item.collapsedRepresentationImage = [[NSImage alloc] initByReferencingFile:@"/System/Library/PreferencePanes/Mouse.prefPane/Contents/Resources/Mouse.icns"];


		NSTouchBar *secondaryTouchBar = [[NSTouchBar alloc] init];
		secondaryTouchBar.delegate = self;
		secondaryTouchBar.defaultItemIdentifiers = @[MouseSensitivitySliderItemIdentifier, MouseZoomSensitivityRatioSliderItemIdentifier];

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
		constexpr const char *cvar = "m_filter";
		NSString *title= @"Filter";
		auto item = [self cvarToggleButtonTouchBarItemWithIdentifier:identifier Cvar:cvar Title:title];
		return item;
	}
	else if ([identifier isEqualToString:CrosshairGroupItemIdentifier]) {
		NSGroupTouchBarItem *item = [[NSGroupTouchBarItem alloc] initWithIdentifier:identifier];

		NSTouchBar *secondaryTouchBar = [[NSTouchBar alloc] init];
		secondaryTouchBar.delegate = self;
		secondaryTouchBar.defaultItemIdentifiers = @[CrosshairTypeItemIdentifier,
		                                             CrosshairColorPickerItemIdentifier];

		item.groupTouchBar = secondaryTouchBar;

		return item;
	}
	else if ([identifier isEqualToString:CrosshairTypeItemIdentifier]) {
		NSPopoverTouchBarItem *item = [[NSPopoverTouchBarItem alloc] initWithIdentifier:identifier];

		//item.collapsedRepresentationLabel = @"Crosshair Type";
		item.collapsedRepresentationImage = [NSImage imageNamed:NSImageNameTouchBarRecordStartTemplate];

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
		NSColorPickerTouchBarItem *item = [NSColorPickerTouchBarItem strokeColorPickerWithIdentifier:identifier];

		self.touchBarCrosshairColor = item;

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
		//item.collapsedRepresentationLabel = @"Customize";
		item.collapsedRepresentationImage = [NSImage imageNamed:NSImageNameTouchBarUserTemplate];

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
		constexpr const char *cvar = "hand";
		NSString *title= @"Right-hand";
		auto item = [self cvarToggleButtonTouchBarItemWithIdentifier:identifier Cvar:cvar Title:title];
		return item;
	}
	else if ([identifier isEqualToString:AutoSwitchButtonItemIdentifier]) {
		constexpr const char *cvar = "_cl_autowepswitch";
		NSString *title= @"Auto-switch";
		auto item = [self cvarToggleButtonTouchBarItemWithIdentifier:identifier Cvar:cvar Title:title];
		return item;
	}
	else if ([identifier isEqualToString:FastSwitchButtonItemIdentifier]) {
		constexpr const char *cvar = "hud_fastswitch";
		NSString *title= @"Fast-switch";
		auto item = [self cvarToggleButtonTouchBarItemWithIdentifier:identifier Cvar:cvar Title:title];
		return item;
	}
	else if ([identifier isEqualToString:ShadowButtonItemIdentifier]) {
		constexpr const char *cvar = "cl_shadows";
		NSString *title= @"Shadow";
		auto item = [self cvarToggleButtonTouchBarItemWithIdentifier:identifier Cvar:cvar Title:title];
		return item;
	}
	else if ([identifier isEqualToString:WeatherButtonItemIdentifier]) {
		constexpr const char *cvar = "cl_weather";
		NSString *title= @"Weather";
		auto item = [self cvarToggleButtonTouchBarItemWithIdentifier:identifier Cvar:cvar Title:title];
		return item;
	}
	else if ([identifier isEqualToString:EscapeButtonItemIdentifier]) {
		NSString *title = @"Menu";

		NSCustomTouchBarItem *item = [[NSCustomTouchBarItem alloc] initWithIdentifier:identifier];
		NSButton *button = [NSButton buttonWithTitle:title target:self action:@selector(buttonESCClicked)];

		NSImage *img = [NSImage imageNamed:NSImageNameTouchBarSidebarTemplate];
		button.image = img;
		button.imageHugsTitle = false;
		item.view =  button;
		return item;
	}
	return nil;
}

- (void)installAsDelegateForWindow:(NSWindow *)window
{
	[self setPreviousDelegate:[window delegate]]; // Save current delegate for forwarding
	[window setDelegate: self];
}

- (void)installAsDelegateForApplication:(NSApplication *)application
{
	[self setPreviousDelegate:[application delegate]]; // Save current delegate for forwarding
	[application setDelegate:self];
}

- (BOOL)respondsToSelector:(SEL)aSelector
{
	// We want to forward to the qt delegate. Respond to selectors it
	// responds to in addition to selectors this instance resonds to.
	return [[self previousDelegate] respondsToSelector:aSelector] || [super respondsToSelector:aSelector];
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	// Forward to the existing delegate. This function is only called for selectors
	// this instance does not responds to, which means that the qt delegate
	// must respond to it (due to the respondsToSelector implementation above).
	[anInvocation invokeWithTarget:[self previousDelegate]];
}

- (void)sensitivityDidChange
{
	Cvar_SetFloat("sensitivity", self.touchBarMouseSensitivity.slider.floatValue);
}

- (void)sensitivityZoomRatioDidChange
{
	Cvar_SetFloat("zoom_sensitivity_ratio", self.touchBarMouseZoomSensitivityRatio.slider.floatValue);
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
