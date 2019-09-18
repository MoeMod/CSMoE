//
//  MapDetailViewController.m
//  xash3d-ios
//
//  Created by 小白白 on 2019/5/18.
//

#import "MapDetailViewController.h"

@interface MapDetailViewController ()

@end

@implementation MapDetailViewController

- (IBAction)OnCloseMapPage:(id)sender
{
	UIViewController *vc = self;
	while (vc.presentingViewController) {
		vc = vc.presentingViewController;
	}
	[vc dismissViewControllerAnimated:YES completion:nil];
	
}

- (IBAction)OnConfirmMapPage:(id)sender
{
	[self OnCloseMapPage:sender];
}

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view.
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
