/* WNMapTabViewDelegate */

#import <Cocoa/Cocoa.h>

@interface WNMapTabViewDelegate : NSTabView
{
}
- (void)awakeFromNib;
-(void)tabView:(NSTabView *)tabView didSelectTabViewItem:(NSTabViewItem *)tabViewItem;
+(NSMutableString *)getTab;
@end
