#import "UIView+IAY.h"

#import <React/UIView+React.h>

@implementation UIView (IAY)

-(UIView*)iay_getParentWhichMatchBlock:(BOOL(^)(UIView* view))block
{
  if (!self.superview) {
    return nil;
  }
  else if (block(self.superview)) {
    return self.superview;
  }
  return [self.superview iay_getParentWhichMatchBlock:block];
}

-(NSArray*)iay_getAllSubviewsRecursively
{
  NSMutableArray *subviews = [NSMutableArray new];
  
  for (UIView *subview in self.subviews) {
    [subviews addObject:subview];
    [subviews addObjectsFromArray:[subview iay_getAllSubviewsRecursively]];
  }

  return subviews;
}

-(NSArray*)iay_getAllSubviewsRecursivelyWithPredicate:(NSPredicate*)predicate;
{
  NSArray* all = [self iay_getAllSubviewsRecursively];
  return [all filteredArrayUsingPredicate:predicate];
}

-(UIView*)iay_findFirstSubviewRecursivelyWithPredicate:(NSPredicate*)predicate;
{
  NSUInteger index = [self.subviews indexOfObjectPassingTest:^BOOL(__kindof UIView * _Nonnull obj, NSUInteger idx, BOOL * _Nonnull stop) {
    if ([predicate evaluateWithObject:obj]) {
      *stop = YES;
      return YES;
    }
    return NO;
  }];
  
  if (index != NSNotFound) {
    return self.subviews[index];
  }
  
  for (UIView* subview in self.subviews) {
    UIView* ret = [subview iay_findFirstSubviewRecursivelyWithPredicate:predicate];
    if (ret) {
      return ret;
    }
  }
  
  return nil;
}


@end
