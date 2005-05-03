#import "Wesnoth_Scenario_Editor_Prefix.h"
#import "WNMapTabViewDelegate.h"

@implementation WNMapTabViewDelegate
    NSMutableString *myTab;
- (void)awakeFromNib
{
    myTab = [[NSMutableString alloc] initWithString:@"Terrain"];
    [myTab retain];
}

-(void)tabView:(NSTabView *)tabView didSelectTabViewItem:(NSTabViewItem *)tabViewItem
{
    fprintf(stderr, "didSelectTabViewItem called\n");
    [myTab setString:[tabViewItem identifier]];
}

+(NSMutableString *)getTab
{
    return myTab;
}

@end
