//
//  NewGameViewController.h
//  xash3d-ios
//
//  Created by 小白白 on 2019/5/13.
//

#import <UIKit/UIKit.h>
#import "../view/LandScapePicker.h"

@interface NewGameViewController : UIViewController
@property (retain, nonatomic) IBOutlet UITextField *ArgsInput;
@property (retain, nonatomic) IBOutlet UIButton *StartGameButton;
@property (retain, nonatomic) IBOutlet UIButton *MapButton;
@property (retain, nonatomic) IBOutlet UIPickerView *GameplayPickerView;

- (IBAction)StartGameButtonActivated:(UIButton *)sender;
- (void) viewDidLoad;

- (void) loadSettings;
- (void) saveSettings;

@end

