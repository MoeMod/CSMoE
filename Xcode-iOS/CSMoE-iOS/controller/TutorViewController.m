//
//  TutorViewController.m
//  xash3d-ios
//
//  Created by 小白白 on 2019/5/13.
//

#import "TutorViewController.h"
#include "string.h"
#include "launcherdialog.h"

@interface TutorScrollViewDelegate : NSObject<UIScrollViewDelegate>

@property (assign) TutorViewController *m_pTutorViewController;
@end

@implementation TutorScrollViewDelegate

- (void)scrollViewDidScroll:(UIScrollView *)scrollView{
	UIPageControl *ppc = self.m_pTutorViewController.PageControl;
	UIScrollView *psv = scrollView;
	const NSInteger i = round(psv.contentOffset.x / psv.frame.size.width);
	ppc.currentPage = i;
}

@end

@interface TutorViewController()
@property (retain, nonatomic) IBOutlet NSLayoutConstraint *PowerCableCenterXConstraint;
@property (retain, nonatomic) IBOutlet NSLayoutConstraint *PowerCableCenterYConstraint;
@property (retain, nonatomic) IBOutlet NSLayoutConstraint *PowerCablePortraitConstraint;
@property (retain, nonatomic) IBOutlet NSLayoutConstraint *PowerCableLandscapeLeftConstraint;
@property (retain, nonatomic) IBOutlet NSLayoutConstraint *PowerCableLandscapeRightConstraint;
@property (retain, nonatomic) IBOutlet NSLayoutConstraint *PowerCablePortraitUpsideDownConstraint;
@property (retain, nonatomic) IBOutlet UIActivityIndicatorView *WaitingForResourcesActivityIndicator;
@property (retain, nonatomic) IBOutlet UILabel *WaitingForResourcesLabel;
@property (retain, nonatomic) IBOutlet UILabel *InstallationFinishLabel;
@property (retain, nonatomic) IBOutlet UIButton *InstallationFinishComfirmButton;
@property (retain, nonatomic) IBOutlet UIImageView *PowerCableImageView;
@end

@implementation TutorViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view.
	
	UIScrollView *psv = self.ScrollView;
   	UIStackView *ptc = self.TutorContent;
	UIPageControl *ppc = self.PageControl;
	
	[psv addSubview:ptc];
	TutorScrollViewDelegate *tsvd = [[TutorScrollViewDelegate alloc] init];
	tsvd.m_pTutorViewController = self;
	psv.delegate = tsvd;
	psv.pagingEnabled = YES;
	
	const NSInteger n = ptc.arrangedSubviews.count;
	ppc.numberOfPages = n;
	ppc.currentPage = 0;	
	// ptc.width = psv.width * n
	//size_t n = ptc.arrangedSubviews.count;
	//[psv addConstraint:[NSLayoutConstraint constraintWithItem:ptc attribute:NSLayoutAttributeWidth relatedBy:NSLayoutRelationEqual toItem:psv attribute:NSLayoutAttributeWidth multiplier:n constant:0]];
	
	// iPad Pro 2018 have USB-C cables...
	{
		CGSize screenSize = [[UIScreen mainScreen] bounds].size;
		if([[[UIDevice currentDevice] model] isEqualToString:@"iPad"])
		{
			float w = screenSize.width, h = screenSize.height;
			if(	!(h < w && h / w == 0.75f)
			 &&	!(w < h && w / h == 0.75f))
				[self.PowerCableImageView setImage:[UIImage imageNamed:@"PowerCable2.png"]];
		}
	}
	
#ifndef TARGET_IPHONE_SIMULATOR
		self.InstallationFinishComfirmButton.hidden = YES;
#endif
	
	dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
		// waiting for resources
		while( !IOS_IsResourcesReady() ) {
			[[NSRunLoop currentRunLoop] runMode:NSDefaultRunLoopMode beforeDate:[NSDate distantFuture]];
		}
		[self OnResourcesReady:nil];
	});
}

- (void)viewDidLayoutSubviews
{
	//[self shouldRotateToOrientation:(UIDeviceOrientation)[UIApplication sharedApplication].statusBarOrientation];
	
	UIScrollView *psv = self.ScrollView;
	UIStackView *ptc = self.TutorContent;
	UIPageControl *ppc = self.PageControl;
	
	const NSInteger n = ptc.arrangedSubviews.count;
	const NSInteger i = ppc.currentPage;
	
	[super viewDidLayoutSubviews];
	
	ptc.frame = CGRectMake(psv.frame.origin.x, psv.frame.origin.y, psv.frame.size.width * n, psv.frame.size.height);
	psv.contentSize = CGSizeMake(psv.frame.size.width * n, 0);
	psv.contentOffset = CGPointMake(psv.frame.size.width * i, psv.contentOffset.y);
	
	{
		//UIDeviceOrientation orientation = [[UIDevice currentDevice] orientation];
		UIDeviceOrientation orientation = [UIApplication sharedApplication].statusBarOrientation;
		
		[self.PowerCableCenterXConstraint setActive:false];
		[self.PowerCableCenterYConstraint setActive:false];
		[self.PowerCablePortraitConstraint setActive:false];
		[self.PowerCableLandscapeLeftConstraint setActive:false];
		[self.PowerCableLandscapeRightConstraint setActive:false];
		[self.PowerCablePortraitUpsideDownConstraint setActive:false];
		
		switch(orientation) // HOME at down
		{
			case UIDeviceOrientationPortrait: {
				[self.PowerCableCenterXConstraint setActive:true];
				[self.PowerCablePortraitConstraint setActive:true];
				[self.PowerCableImageView setTransform:CGAffineTransformMakeRotation(0)];
				break;
			}
			case UIDeviceOrientationPortraitUpsideDown: {
				[self.PowerCableCenterXConstraint setActive:true];
				[self.PowerCablePortraitUpsideDownConstraint setActive:true];
				[self.PowerCableImageView setTransform:CGAffineTransformMakeRotation(M_PI)];
				break;
			}
			case UIDeviceOrientationLandscapeRight: {
				[self.PowerCableCenterYConstraint setActive:true];
				[self.PowerCableCenterYConstraint setConstant:[[UIScreen mainScreen] bounds].size.height / 2 - ptc.center.y];
				[self.PowerCableLandscapeLeftConstraint setActive:true];
				[self.PowerCableImageView setTransform:CGAffineTransformMakeRotation(M_PI / 2)];
				break;
			}
			case UIDeviceOrientationLandscapeLeft: {
				[self.PowerCableCenterYConstraint setActive:true];
				[self.PowerCableCenterYConstraint setConstant:[[UIScreen mainScreen] bounds].size.height / 2 - ptc.center.y];
				[self.PowerCableLandscapeRightConstraint setActive:true];
				[self.PowerCableImageView setTransform:CGAffineTransformMakeRotation(M_PI * 3 / 2)];
				break;
			}
			default: {
				[self.PowerCableCenterXConstraint setActive:true];
				[self.PowerCablePortraitConstraint setActive:true];
				[self.PowerCableImageView setTransform:CGAffineTransformMakeRotation(0)];
				break;
			}
		}
		
		[self.PowerCableImageView invalidateIntrinsicContentSize];
	}
}

- (IBAction)ResourceDownloadLinkButtonActivated:(UIButton *)sender
{
	[[UIApplication sharedApplication] openURL:[NSURL URLWithString:@"https://github.com/MoeMod/CSMoE-GameDir"]];
}

- (IBAction)OnResourcesReady:(id)sender
{
    self.WaitingForResourcesActivityIndicator.hidden = YES;
	self.WaitingForResourcesLabel.hidden = YES;
	self.InstallationFinishLabel.hidden = NO;
	self.InstallationFinishComfirmButton.hidden = NO;
}

- (IBAction)OnInstallationFinishComfirmed:(id)sender
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
