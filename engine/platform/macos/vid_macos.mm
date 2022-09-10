/*
vid_macos.mm
Copyright (C) 2020 Moemod Hymei

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
#import <SDL_video.h>
#import <SDL_syswm.h>

#include "common.h"
#include "vid_macos.h"

static NSWindow *MacOS_GetNSWindow()
{
	SDL_SysWMinfo wmInfo;
	SDL_VERSION(&wmInfo.version);
	SDL_GetWindowWMInfo(host.hWnd, &wmInfo);
	return wmInfo.info.cocoa.window;
}

float MacOS_GetDPI()
{
	NSWindow *nsw = MacOS_GetNSWindow();
	[nsw contentLayoutRect];
	return [nsw backingScaleFactor];
}

float MacOS_TitleBarHeight()
{
	NSWindow *nsw = MacOS_GetNSWindow();
	NSRect frame = NSMakeRect (0, 0, 100, 100);
	NSRect contentRect = [nsw contentRectForFrameRect: frame];
	return (frame.size.height - contentRect.size.height);
}

void MacOS_OpenURL(const char *url)
{
	[[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:[[NSString alloc] initWithUTF8String:url]]];
}

void MacOS_InstallWindow()
{
    auto nsw = MacOS_GetNSWindow();

    nsw.styleMask |=  NSWindowStyleMaskUnifiedTitleAndToolbar;
    nsw.styleMask |=  NSWindowStyleMaskFullSizeContentView;
    nsw.titleVisibility = NSWindowTitleHidden;
    nsw.titlebarAppearsTransparent = TRUE;
}

void MacOS_ToggleWindowButtons(bool bShow)
{
    auto nsw = MacOS_GetNSWindow();

    NSButton *closeBtn = [nsw standardWindowButton:NSWindowCloseButton];
    NSButton *miniaturizeBtn = [nsw standardWindowButton:NSWindowMiniaturizeButton];
    NSButton *zoomBtn = [nsw standardWindowButton:NSWindowZoomButton];

    [closeBtn setHidden:!bShow];
    [miniaturizeBtn setHidden:!bShow];
    [zoomBtn setHidden:!bShow];
}

void MacOS_HiDPI_Scale(int *w, int *h)
{
    auto nsw = MacOS_GetNSWindow();
    if(w) *w *= nsw.backingScaleFactor;
    if(h) *h *= nsw.backingScaleFactor;
}