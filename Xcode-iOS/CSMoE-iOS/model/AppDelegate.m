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
- (UIInterfaceOrientationMask)application:(UIApplication *)application supportedInterfaceOrientationsForWindow:(UIWindow *)window;
@end

@implementation SDLUIKitDelegate(category)

- (UIInterfaceOrientationMask)application:(UIApplication *)application supportedInterfaceOrientationsForWindow:(UIWindow *)window {
	if(![[[UIDevice currentDevice] model] isEqualToString:@"iPad"] && g_iStartGameStatus == XGS_START)
		return UIInterfaceOrientationMaskLandscape;
	return UIInterfaceOrientationMaskAll;
}

@end
