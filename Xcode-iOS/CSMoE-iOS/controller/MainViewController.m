/*
 MainViewController.m - iOS lauch dialog
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

#import "MainViewController.h"

#import "AppDelegate.h"

#include "launcherdialog.h"

@interface MainViewControllerDelegate : NSObject<UITabBarControllerDelegate>
- (BOOL)tabBarController:(UITabBarController *)tabBarController shouldSelectViewController:(UIViewController *)viewController NS_AVAILABLE_IOS(3_0);
- (void)tabBarController:(UITabBarController *)tabBarController didSelectViewController:(UIViewController *)viewController;
@end

@implementation MainViewControllerDelegate
- (BOOL)tabBarController:(UITabBarController *)tabBarController shouldSelectViewController:(UIViewController *)viewController NS_AVAILABLE_IOS(3_0)
{
	
	return YES;
}

- (void)tabBarController:(UITabBarController *)tabBarController didSelectViewController:(UIViewController *)viewController
{
	
}

@end

@interface MainTabBarController()
@property(strong) MainViewControllerDelegate *m_pMainViewControllerDelegate;
@end

@implementation MainTabBarController

- (void)viewDidLoad {
	
	[super setSelectedIndex:0];
	self.m_pMainViewControllerDelegate = [MainViewControllerDelegate new];
	self.delegate = self.m_pMainViewControllerDelegate;
	[super viewDidLoad];
    // Do any additional setup after loading the view.
#ifndef TARGET_IPHONE_SIMULATOR
	if(!IOS_IsResourcesReady())
	{
		// handle in another thread
		dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
			dispatch_async(dispatch_get_main_queue(), ^{
				
				[self performSegueWithIdentifier:@"ShowTutorScene" sender:self];
			});
		});
	}
#endif
}

@end
