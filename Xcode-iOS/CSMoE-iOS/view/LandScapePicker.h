//
//  LandScapePicker.h
//  PickView
//
//  Created by A$CE on 2018/1/25.
//  Copyright © 2018 A$CE. All rights reserved.
//

#import <UIKit/UIKit.h>
/**
 *  type(NSInteger) ===> Selected row in picker
 type(NSString *) ===> Title show in pikcer
 */
typedef void(^LandScapePickerSelected)(NSInteger,NSString *);

@interface LandScapePicker : UIView

@property (nonatomic ,strong) NSArray<NSString *> *pTitles;

@property (nonatomic ,strong) UIColor *titleColor;

@property (nonatomic ,copy) LandScapePickerSelected lspSelected;
@end
