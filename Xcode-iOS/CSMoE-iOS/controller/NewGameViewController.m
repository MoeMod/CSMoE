//
//  NewGameViewController.m
//  xash3d-ios
//
//  Created by 小白白 on 2019/5/13.
//

#import "NewGameViewController.h"

#include "launcherdialog.h"

#import "../util/TGAlib.h"

typedef struct gamemode_s
{
	const char *name;
	const char *code;
} gamemode_t;

gamemode_t g_GameModes[] = {
	{ "Original", "none" },
	{ "DeathMatch", "dm" },
	{ "TeamDeathMatch", "tdm" },
	{ "ZombieMod1", "zb1" },
	{ "ZombieMod2", "zb2" },
	{ "ZombieModHero", "zb3" },
	{ "ZombieScenario", "zbs" },
	{ "GunDeath", "gd" },
	{ "ZombieBaseBuilder", "zbb" }
};

@interface GameModeViewDelegateAndDataSource : NSObject<UIPickerViewDelegate, UIPickerViewDataSource>

@end

@implementation GameModeViewDelegateAndDataSource

- (NSInteger)numberOfComponentsInPickerView:(UIPickerView *)pickerView
{
	return 1;
}

- (NSInteger)pickerView:(UIPickerView *)pickerView numberOfRowsInComponent:(NSInteger)component
{
	return sizeof(g_GameModes)/sizeof(gamemode_t);
}

- (NSString *)pickerView:(UIPickerView *)pickerView
			 titleForRow:(NSInteger)row forComponent:(NSInteger)component
{
	return [NSString stringWithUTF8String:g_GameModes[row].name];
}

- (void)pickerView:(UIPickerView *)pickerView didSelectRow:
(NSInteger)row inComponent:(NSInteger)component
{
	//...
}

@end

@interface NewGameViewController()
@property(strong) GameModeViewDelegateAndDataSource *m_pGameModeDelegate;
@end

@implementation NewGameViewController

- (void)viewDidLoad {
	[self loadSettings];
	
	{
		GameModeViewDelegateAndDataSource *p = [[GameModeViewDelegateAndDataSource alloc] init];
		self.GameplayPickerView.delegate = p;
		self.GameplayPickerView.dataSource = p;
		self.m_pGameModeDelegate = p;
	}
	
	
	[super viewDidLoad];
	
	//UIApplication *app = [UIApplication sharedApplication];
	@autoreleasepool {
		
		NSString *imagePath = [[NSBundle mainBundle] pathForResource:@"maps/random_cso" ofType:@"tga"];
		tImageTGA *tga = tgaLoad([imagePath UTF8String]);
		
		CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
		
		CGContextRef contextRef = CGBitmapContextCreate(tga->imageData, tga->width, tga->height, 8, 4 * tga->width, colorSpace, kCGImageAlphaPremultipliedLast);
		CGImageRef cgImage2 = CGBitmapContextCreateImage(contextRef);
		
		UIImage *mapImage = [UIImage imageWithCGImage:cgImage2 scale:1.0 orientation:UIImageOrientationDownMirrored];
		
		//[self.MapButton setBackgroundImage:mapImage forState:UIControlStateNormal];
		//[self.MapButton.heightAnchor constraintEqualToAnchor:self.MapButton.widthAnchor multiplier:(CGFloat)tga->height/tga->width].active = YES;
		//[self.MapButton setTitle:nil forState:UIControlStateNormal];
		
		CGImageRelease(cgImage2);
        CGContextRelease(contextRef);
        CGColorSpaceRelease(colorSpace);
		tgaDestroy(tga);
	}
	
	
}

- (IBAction)StartGameButtonActivated:(UIButton *)sender {
	
	[self saveSettings];
	
	NSArray *argv = [ _ArgsInput.text componentsSeparatedByString:@" " ];
	
	size_t count = [argv count];
	char *arg1 = "arg1";
	g_pszArgv = calloc( count + 2, sizeof( char* ) );
	g_pszArgv[0] = arg1;
	for( int i = 0; i < count; i++ )
	{
		g_pszArgv[i + 1] = strdup( [argv[i] UTF8String] );
	}
	g_iArgc = (int)count + 1;
	g_pszArgv[count + 1] = 0;
	
	g_iStartGameStatus = XGS_START;
	
}

- (void) loadSettings
{
	NSString *savestr = [[NSUserDefaults standardUserDefaults] objectForKey:@"argsKey"];
	
	if( !savestr || ![savestr length] )
		savestr = @"-dev 5 -log -game csmoe";
	
	[_ArgsInput setText:savestr];
}

- (void) saveSettings
{
	[[NSUserDefaults standardUserDefaults] setObject:[_ArgsInput text] forKey:@"argsKey"];
	[[NSUserDefaults standardUserDefaults] synchronize];
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
