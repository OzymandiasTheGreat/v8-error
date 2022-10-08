#import <React/RCTBridgeDelegate.h>
#import <UIKit/UIKit.h>

@interface AppDelegate : UIResponder <UIApplicationDelegate, RCTBridgeDelegate>

@property (nonatomic, strong) UIWindow* window;
@property (nonatomic, readonly) UIViewController* rootViewController;

-(void)hideLaunchScreen;

@end
