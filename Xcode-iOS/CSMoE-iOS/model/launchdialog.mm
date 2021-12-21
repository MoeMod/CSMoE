/*
 launchdialog.m - iOS lauch dialog
 Copyright (C) 2016 mittorn
 
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
#include <sys/stat.h>
#include "launcherdialog.h"

char *g_szLibrarySuffix = NULL;
float g_iOSVer;

enum XashGameStatus_e g_iStartGameStatus = XGS_SKIP;

void IOS_StartBackgroundTask()
{
	/**/
}

const char *IOS_GetDocsDir()
{
	static const char *dir = nil;
	
	if( dir )
		return dir;
	
	NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
	NSString *documentsDirctory = [paths objectAtIndex:0];
	[[NSFileManager defaultManager] createDirectoryAtPath:documentsDirctory withIntermediateDirectories:YES attributes:nil error:nil];
	
	dir = [documentsDirctory fileSystemRepresentation];
	// NSLog(@"IOS_GetDocsDir: %s", dir);
	
	return dir;
}

const char *IOS_GetBundleDir()
{
	NSString *path = [[NSBundle mainBundle] bundlePath];
	
	static char c_path[256];
	strcpy(c_path, [[path stringByAppendingPathComponent:@"/CSMoE-Full"] UTF8String]);
	
	//NSLog(@"IOS_GetBundleDir: %s", c_path);
	
	return c_path;
}

BOOL IOS_IsResourcesReady()
{
	NSString *doc = [NSString stringWithUTF8String:IOS_GetDocsDir()];
	NSString *bundle = [NSString stringWithUTF8String:IOS_GetBundleDir()];
	
	NSFileManager *fileManager = [NSFileManager defaultManager];
	return 	([fileManager fileExistsAtPath:[doc stringByAppendingPathComponent:@"csmoe"]] || [fileManager fileExistsAtPath:[bundle stringByAppendingPathComponent:@"csmoe"]]) &&
			([fileManager fileExistsAtPath:[doc stringByAppendingPathComponent:@"cstrike"]] || [fileManager fileExistsAtPath:[bundle stringByAppendingPathComponent:@"cstrike"]]) &&
			([fileManager fileExistsAtPath:[doc stringByAppendingPathComponent:@"valve"]] || [fileManager fileExistsAtPath:[bundle stringByAppendingPathComponent:@"valve"]]);
}

void IOS_PrepareView()
{
	UIWindow *window = [[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]];
	
	NSBundle *bundle = [NSBundle mainBundle];
	NSString *storyboardName = @"TutorStoryboard";// [bundle objectForInfoDictionaryKey:@"UIMainStoryboardFile"];
	
	UIStoryboard *storyboard = [UIStoryboard storyboardWithName:storyboardName bundle:bundle];
	UIViewController * controller = storyboard.instantiateInitialViewController;
	
	
	[window setRootViewController:controller];
	[window makeKeyAndVisible];
}

void IOS_SetDefaultArgs()
{
	static char width_str[32] = "0";
	static char height_str[32] = "0";
	static char *args[64] = { "xash", "-dev", "5", "-log", "-game", "csmoe", "-width", width_str, "-height", height_str };
	
	CGRect rect_screen = [[UIScreen mainScreen]bounds];
    CGSize size_screen = rect_screen.size;
	CGFloat scale_screen = [UIScreen mainScreen].scale;
	CGFloat width = size_screen.width * scale_screen;
	CGFloat height = size_screen.height * scale_screen;
	sprintf(width_str, "%d", (int)width);
	sprintf(height_str, "%d", (int)height);
	
	g_pszArgv = args;
	g_iArgc = 10;
}

void IOS_LaunchDialog( void )
{
	NSString *ver = [[UIDevice currentDevice] systemVersion];
	g_iOSVer = [ver floatValue];
	NSLog(@"System Version is %@",ver);
	
	if(!IOS_IsResourcesReady())
	{
		g_iStartGameStatus = XGS_WAITING;
		IOS_PrepareView();
	}

	// wating for starting
	@autoreleasepool {
		while( g_iStartGameStatus == XGS_WAITING ) {
			[[NSRunLoop currentRunLoop] runMode:NSDefaultRunLoopMode beforeDate:[NSDate distantFuture]];
		}
	}
	
	IOS_SetDefaultArgs();
	
	// return back to xash3d
}

char *IOS_GetUDID( void )
{
	static char udid[256];
	NSString *id = [[[UIDevice currentDevice]identifierForVendor] UUIDString];
	strncpy( udid, [id UTF8String], 255 );
	return udid;
}

void IOS_Log(const char *text)
{
	NSLog(@"Xash: %@", [NSString stringWithUTF8String:text]);
}
