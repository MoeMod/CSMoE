//
//  UnzipViewController.m
//  CSMoE-iOS
//
//  Created by 小白白 on 2019/9/20.
//

#import "UnzipViewController.h"

@interface UnzipViewController ()
@property (weak, nonatomic) IBOutlet UILabel *ProgressTipLabel;
@property (weak, nonatomic) IBOutlet UIProgressView *ProgressBar;

- (void)OnInstallationFinishComfirmed:(id)sender;
@end

@implementation UnzipViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view.
}

- (void)OnInstallationFinishComfirmed:(id)sender
{
	UIViewController *vc = self;
	while (vc.presentingViewController) {
		vc = vc.presentingViewController;
	}
	[vc dismissViewControllerAnimated:YES completion:nil];
	
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
