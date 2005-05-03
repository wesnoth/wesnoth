/* WNMainTabViewDelegate */

#import <Cocoa/Cocoa.h>

@interface WNMainTabViewDelegate : NSTabView
{
}
- (void)tabView:(NSTabView *)tabView didSelectTabViewItem:(NSTabViewItem *)tabViewItem;
@end
