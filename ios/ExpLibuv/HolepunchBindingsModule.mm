#import "HolepunchBindingsModule.h"

#import <React/RCTBridge.h>
#import <React/UIView+React.h>

#import "AppDelegate.h"
#import "uv.hpp"
#import "UIView+IAY.h"

using namespace facebook;

@interface RCTBridge (Holepunch)
+(instancetype)currentBridge;
-(std::shared_ptr<react::CallInvoker>)jsCallInvoker;
-(void*)runtime;
@end

@implementation HolepunchBindingsModule

RCT_EXPORT_MODULE(HolepunchBindings)

+(BOOL)requiresMainQueueSetup
{
  return YES;
}

-(NSDictionary*)constantsToExport
{
  return @{
    @"FS_BASE": NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask,YES).lastObject,
    @"FS_TMP_BASE": NSTemporaryDirectory()
  };
}

-(void)initialize
{
  RCTRegisterReloadCommandListener(self);
}

-(void)didReceiveReloadCommand
{
  RCTBridge* bridge = [RCTBridge currentBridge];
  __weak RCTBridge* wbridge = bridge;
  bridge.jsCallInvoker->invokeAsync([=]() {
    if (!wbridge) {
      return;
    }

    auto rt = (jsi::Runtime*)wbridge.runtime;
    if (!rt) {
      return;
    }

    holepunch::uv::reloadSignal(*rt);
  });
}

RCT_EXPORT_METHOD(hideLaunchScreen)
{
  dispatch_async(dispatch_get_main_queue(), ^{
    AppDelegate* app = (AppDelegate*)[UIApplication sharedApplication].delegate;
    [app hideLaunchScreen];
  });
}

@end
