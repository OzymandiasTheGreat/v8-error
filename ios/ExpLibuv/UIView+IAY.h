#import <UIKit/UIKit.h>

NS_ASSUME_NONNULL_BEGIN

@interface UIView (IAY)

-(UIView*)iay_getParentWhichMatchBlock:(BOOL(^)(UIView* view))block;

-(NSArray*)iay_getAllSubviewsRecursively;
-(NSArray*)iay_getAllSubviewsRecursivelyWithPredicate:(NSPredicate*)predicate;
-(UIView*)iay_findFirstSubviewRecursivelyWithPredicate:(NSPredicate*)predicate;

@end

NS_ASSUME_NONNULL_END
