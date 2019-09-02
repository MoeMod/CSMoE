//
//  TutorViewController.h
//  xash3d-ios
//
//  Created by 小白白 on 2019/5/13.
//

#import <UIKit/UIKit.h>

NS_ASSUME_NONNULL_BEGIN

@interface TutorViewController : UIViewController
@property (retain, nonatomic) IBOutlet UIScrollView *ScrollView;
@property (retain, nonatomic) IBOutlet UIPageControl *PageControl;
@property (retain, nonatomic) IBOutlet UIStackView *TutorContent;

- (IBAction)ResourceDownloadLinkButtonActivated:(UIButton *)sender;
- (IBAction)OnResourcesReady:(nullable id)sender;
- (IBAction)OnInstallationFinishComfirmed:(id)sender;

@end

NS_ASSUME_NONNULL_END
