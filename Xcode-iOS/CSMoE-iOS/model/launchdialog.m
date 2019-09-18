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

#include <SDL_hints.h>

char *g_szLibrarySuffix = NULL;
float g_iOSVer;

enum XashGameStatus_e g_iStartGameStatus = XGS_WAITING;

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

BOOL IOS_IsResourcesReady()
{
	static NSString *path = nil;
	if(!path)
		path = [NSString stringWithUTF8String:IOS_GetDocsDir()];
	
	NSFileManager *fileManager = [NSFileManager defaultManager];
	return	[fileManager fileExistsAtPath:[path stringByAppendingPathComponent:@"csmoe"]] &&
			[fileManager fileExistsAtPath:[path stringByAppendingPathComponent:@"cstrike"]] &&
			[fileManager fileExistsAtPath:[path stringByAppendingPathComponent:@"valve"]];
}

void IOS_PrepareView()
{
	UIWindow *window = [[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]];
	
	NSBundle *bundle = [NSBundle mainBundle];
	NSString *storyboardName = [bundle objectForInfoDictionaryKey:@"UIMainStoryboardFile"];
	
	UIStoryboard *storyboard = [UIStoryboard storyboardWithName:storyboardName bundle:bundle];
	UIViewController * controller = storyboard.instantiateInitialViewController;
	
	
	[window setRootViewController:controller];
	[window makeKeyAndVisible];
}

void IOS_SetDefaultArgs()
{
	static char *args[64] = { "xash", "-dev", "5", "-log", "-game", "csmoe"};
	
	g_pszArgv = args;
	g_iArgc = 6;
}

void IOS_LaunchDialog( void )
{
	NSString *ver = [[UIDevice currentDevice] systemVersion];
	g_iOSVer = [ver floatValue];
	NSLog(@"System Version is %@",ver);
	
	if(g_iStartGameStatus == XGS_WAITING)
	{
		IOS_PrepareView();
	}
	else
	{
		IOS_SetDefaultArgs();
	}
	
	// wating for starting
	@autoreleasepool {
		while( g_iStartGameStatus == XGS_WAITING ) {
			[[NSRunLoop currentRunLoop] runMode:NSDefaultRunLoopMode beforeDate:[NSDate distantFuture]];
		}
	}
	
	// iOS Settings...
	SDL_SetHint(SDL_HINT_IOS_HIDE_HOME_INDICATOR, "2");
	
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
