#import "Wesnoth_Scenario_Editor_Prefix.h"
#import "WNMainTabViewDelegate.h"

@implementation WNMainTabViewDelegate
-(void)tabView:(NSTabView *)tabView didSelectTabViewItem:(NSTabViewItem *)tabViewItem
{
    fprintf(stderr, "didSelectTabViewItem called\n");
    NSString *thisTab = [tabViewItem identifier];
    if ([thisTab isEqualTo:@"mapTab"])
	[[NSNotificationCenter defaultCenter] postNotificationName:@"mapTabSelected" object:self];
    fprintf(stderr, "Now viewing tab %s\n", [thisTab UTF8String]);
}
@end
