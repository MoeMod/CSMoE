/*
 AppDelegate.m - iOS lauch dialog
 Copyright (C) 2019 MoeMod Hymei
 
 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 */

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import "SDL2/src/video/uikit/SDL_uikitappdelegate.h"
#include "launcherdialog.h"

/*
 3D Touch Handler
 */

@interface SDLUIKitDelegate(category)
- (BOOL)application:(UIApplication *)application willFinishLaunchingWithOptions:(NSDictionary *)launchOptions;
- (void)application:(UIApplication *)application performActionForShortcutItem:(UIApplicationShortcutItem *)shortcutItem completionHandler:(void (^)(BOOL succeeded))completionHandler;
@end

@implementation SDLUIKitDelegate(category)
- (BOOL)application:(UIApplication *)application willFinishLaunchingWithOptions:(NSDictionary *)launchOptions
{
	NSMutableArray *arrShortcutItem = [NSMutableArray new];
	
	UIApplicationShortcutItem *shoreItem1 = [[UIApplicationShortcutItem alloc]
											 initWithType:@"com.moemod.csmoe.Continue"
											 localizedTitle:@"Continue"
											 localizedSubtitle:nil
											 icon:[UIApplicationShortcutIcon iconWithType:UIApplicationShortcutIconTypePlay]
											 userInfo:nil];
	[arrShortcutItem addObject:shoreItem1];
	
	UIApplicationShortcutItem *shoreItem2 = [[UIApplicationShortcutItem alloc]
											 initWithType:@"com.moemod.csmoe.MainMenu"
											 localizedTitle:@"Main Menu"
											 localizedSubtitle:@""
											 icon:[UIApplicationShortcutIcon iconWithType:UIApplicationShortcutIconTypeHome]
											 userInfo:nil];
	[arrShortcutItem addObject:shoreItem2];
	
	/*
	UIApplicationShortcutItem *shoreItem3 = [[UIApplicationShortcutItem alloc]
											 initWithType:@"com.moemod.csmoe.CreateGame"
											 localizedTitle:@"Create Game"
											 localizedSubtitle:@""
											 icon:[UIApplicationShortcutIcon iconWithType:UIApplicationShortcutIconTypeAdd]
											 userInfo:nil];
	[arrShortcutItem addObject:shoreItem3];
	*/
	
	[UIApplication sharedApplication].shortcutItems = arrShortcutItem;
	return TRUE;
}
- (void)application:(UIApplication *)application performActionForShortcutItem:(UIApplicationShortcutItem *)shortcutItem completionHandler:(void (^)(BOOL succeeded))completionHandler
{
	if(!shortcutItem)
		return;
	
	if([shortcutItem.type isEqualToString:@"com.moemod.csmoe.Continue"])
	{
		// ...
	}
	else if([shortcutItem.type isEqualToString:@"com.moemod.csmoe.MainMenu"])
	{
		IOS_SetDefaultArgs();
		
		g_iStartGameStatus = XGS_SKIP;
		
	}
}

- (UIInterfaceOrientationMask)application:(UIApplication *)application supportedInterfaceOrientationsForWindow:(UIWindow *)window {
	if(![[[UIDevice currentDevice] model] isEqualToString:@"iPad"] && g_iStartGameStatus == XGS_START)
		return UIInterfaceOrientationMaskLandscape;
	return UIInterfaceOrientationMaskAll;
}

@end
