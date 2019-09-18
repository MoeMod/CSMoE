//
//  LandScapePicker.m
//  PickView
//
//  Created by A$CE on 2018/1/25.
//  Copyright © 2018 A$CE. All rights reserved.
//

#import "LandScapePicker.h"

@interface LandScapePicker()<UIPickerViewDelegate,UIPickerViewDataSource>
@property (nonatomic ,strong) UIPickerView *pickerView;

@end

@implementation LandScapePicker

- (instancetype)initWithFrame:(CGRect)frame {
	self = [super initWithFrame:frame];
	if (self) {
		self.backgroundColor = [UIColor clearColor];
		self.pickerView = [[UIPickerView alloc] init];
		self.pickerView.delegate = self;
		self.pickerView.dataSource = self;
	}
	return self;
}

// Only override drawRect: if you perform custom drawing.
// An empty implementation adversely affects performance during animation.
- (void)drawRect:(CGRect)rect {
	CGFloat w = rect.size.height;
	CGFloat h = rect.size.width;
	CGFloat centerX = w * 0.5;
	CGFloat centerY = h * 0.5;
	CGFloat y = centerX - h * 0.5;
	CGFloat x = centerY - w * 0.5;
	self.pickerView.frame = CGRectMake(x, y,w,h);
	self.pickerView.transform = CGAffineTransformMakeRotation(M_PI*3/2);
	[self addSubview:self.pickerView];
}

- (NSInteger)numberOfComponentsInPickerView:(UIPickerView *)pickerView {
	return 1;
}

- (NSInteger)pickerView:(UIPickerView *)pickerView numberOfRowsInComponent:(NSInteger)component {
	return self.pTitles.count;
}

- (UIView *)pickerView:(UIPickerView *)pickerView viewForRow:(NSInteger)row forComponent:(NSInteger)component reusingView:(UIView *)view {
	
	for(UIView *singleLine in pickerView.subviews) {
		if (singleLine.frame.size.height < 1) {
			singleLine.backgroundColor = [UIColor clearColor];}}
	
	UILabel *label = [[UILabel alloc] init];
	label.text = self.pTitles[row];
	label.textColor = self.titleColor;
	label.textAlignment = NSTextAlignmentCenter;
	label.adjustsFontSizeToFitWidth = YES;
	label.transform = CGAffineTransformMakeRotation(M_PI_2);
	return label;
}

- (CGFloat)pickerView:(UIPickerView *)pickerView widthForComponent:(NSInteger)component {
	return 44;
}

- (CGFloat)pickerView:(UIPickerView *)pickerView rowHeightForComponent:(NSInteger)component {
	return 88;
}

- (void)pickerView:(UIPickerView *)pickerView didSelectRow:(NSInteger)row inComponent:(NSInteger)component {
	if (self.lspSelected) {
		self.lspSelected(row,self.pTitles[row]);
	}
}


@end
