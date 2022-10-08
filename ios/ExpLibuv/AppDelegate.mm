#import "AppDelegate.h"

#import <React/RCTBridge.h>
#import <React/RCTBundleURLProvider.h>
#import <React/RCTRootView.h>

#import <React/RCTAppSetupUtils.h>

#import <reacthermes/HermesExecutorFactory.h>
#import <React/RCTCxxBridgeDelegate.h>
#import <React/RCTJSIExecutorRuntimeInstaller.h>
#import <React/RCTLinkingManager.h>

#import "uv.hpp"
#import "fs.hpp"

#import "UIView+IAY.h"

@interface AppDelegate () <RCTCxxBridgeDelegate> {}

@property (nonatomic, strong) UIWindow* launchWindow;
@property (nonatomic, strong) UIViewController* rootViewController;

@end

@interface RCTBridge (Holepunch)

-(std::shared_ptr<facebook::react::CallInvoker>)jsCallInvoker;

@end

@implementation AppDelegate

// https://github.com/facebook/react-native/blob/05aaba95145df0b7f541e391a9f64ba3402cac35/packages/rn-tester/NativeComponentExample/ios/RNTMyNativeViewComponentView.mm#L46

+(UIColor*)UIColorFromHexString:(NSString*)colorString
{
  unsigned rgbValue = 0;
  NSScanner *scanner = [NSScanner scannerWithString:colorString];
  [scanner setScanLocation:1]; // bypass '#' character
  [scanner scanHexInt:&rgbValue];
  return [UIColor colorWithRed:((rgbValue & 0xFF0000) >> 16) / 255.0
                         green:((rgbValue & 0xFF00) >> 8) / 255.0
                          blue:(rgbValue & 0xFF) / 255.0
                         alpha:1.0];
}

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions
{
  RCTAppSetupPrepareApp(application);

  RCTBridge* bridge = [[RCTBridge alloc] initWithDelegate:self launchOptions:launchOptions];

  UIColor* bgColor = [AppDelegate UIColorFromHexString:@"#141925"];
  CGRect bounds = [UIScreen mainScreen].bounds;

  self.launchWindow = [[UIWindow alloc] initWithFrame:bounds];
  UIStoryboard* storyboard = [UIStoryboard storyboardWithName:@"LaunchScreen" bundle:nil];
  self.launchWindow.rootViewController = [storyboard instantiateInitialViewController];
  self.launchWindow.rootViewController.view.backgroundColor = bgColor;
  self.launchWindow.windowLevel = UIWindowLevelNormal + 1;
  [self.launchWindow setHidden:NO];

  self.window = [[UIWindow alloc] initWithFrame:bounds];
  NSDictionary* initProps = [self prepareInitialProps];
  UIView* rootView = RCTAppSetupDefaultRootView(bridge, @"ExpLibuv", initProps);
  rootView.backgroundColor = bgColor;
  self.rootViewController = [UIViewController new];
  self.rootViewController.view = rootView;
  self.window.rootViewController = self.rootViewController;
  self.window.windowLevel = UIWindowLevelNormal;

  [self.window makeKeyAndVisible];

  return YES;
}

- (BOOL)concurrentRootEnabled
{
  // Switch this bool to turn on and off the concurrent root
  return true;
}

- (NSDictionary *)prepareInitialProps
{
  NSMutableDictionary *initProps = [NSMutableDictionary new];

  return initProps;
}

- (NSURL *)sourceURLForBridge:(RCTBridge *)bridge
{
#if DEBUG
  NSString* bundle = @"index";
  return [[RCTBundleURLProvider sharedSettings] jsBundleURLForBundleRoot:bundle];
#else
  return [[NSBundle mainBundle] URLForResource:@"main" withExtension:@"jsbundle"];
#endif
}

#pragma mark - RCTCxxBridgeDelegate

using namespace facebook;
using namespace facebook::react;

struct IOSCallInvokerGetter : holepunch::CallInvokerGetter {
  __weak RCTBridge* bridge;

  bool isValid() override {
    if (!bridge) {
      return false;
    }
    return bridge.isValid;
  }

  std::shared_ptr<react::CallInvoker> get() override {
    return bridge.jsCallInvoker;
  }
};

-(std::unique_ptr<facebook::react::JSExecutorFactory>)jsExecutorFactoryForBridge:(RCTBridge*)bridge
{
  const auto installer = RCTJSIExecutorRuntimeInstaller([bridge](jsi::Runtime &runtime) {
    if (!bridge) {
      return;
    }

    __weak RCTBridge* wbridge = bridge;
    auto getter = std::make_shared<IOSCallInvokerGetter>();
    getter->bridge = wbridge;

    holepunch::uv::install(runtime, getter);
    holepunch::fs::install(runtime, getter);
  });

  return std::make_unique<HermesExecutorFactory>(installer);
}

#pragma mark - Linking

-(BOOL)application:(UIApplication*)application openURL:(NSURL*)url options:(NSDictionary<UIApplicationOpenURLOptionsKey,id>*)options
{
  return [RCTLinkingManager application:application openURL:url options:options];
}

#pragma mark - App

-(void)hideLaunchScreen
{
  [self.launchWindow setHidden:YES];
  self.launchWindow = nil;
}

@end
