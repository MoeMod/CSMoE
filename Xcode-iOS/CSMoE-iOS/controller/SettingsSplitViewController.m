//
//  SettingsSplitViewController.m
//  xash3d-ios
//
//  Created by 小白白 on 2019/5/19.
//

#import "SettingsSplitViewController.h"

@interface SettingsSplitViewControllerDelegate : NSObject<UISplitViewControllerDelegate>
@end

@implementation SettingsSplitViewControllerDelegate

- (BOOL)splitViewController:(UISplitViewController *)svc shouldHideViewController:(UIViewController *)vc inOrientation:(UIInterfaceOrientation)orientation
{
	return NO;
}

- (BOOL)splitViewController:(UISplitViewController *)splitViewController collapseSecondaryViewController:(UIViewController *)secondaryViewController ontoPrimaryViewController:(UIViewController *)primaryViewController
{
	return YES;
}

@end

@interface SettingsSplitViewController ()
@property(strong) SettingsSplitViewControllerDelegate *m_pSettingsSplitViewControllerDelegate;
@end

@implementation SettingsSplitViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view.
	self.m_pSettingsSplitViewControllerDelegate = [[SettingsSplitViewControllerDelegate alloc] init];
	self.delegate = self.m_pSettingsSplitViewControllerDelegate;
}

/*
#pragma mark - Navigation

// In a storyboard-based application, you will often want to do a little preparation before navigation
- (void)prepareForSegue:(UIStoryboardSegue *)segue sender:(id)sender {
    // Get the new view controller using [segue destinationViewController].
    // Pass the selected object to the new view controller.
}
*/

@end
