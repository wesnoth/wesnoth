#import "Wesnoth_Scenario_Editor_Prefix.h"
#import "WNTerrainView.h"
#import "WNTerrains.h"
#import "WNUnits.h"

@implementation WNTerrainView
-(void)awakeFromNib
{
    [WNTerrains init];
    [WNUnits init];
    NSTableColumn* theColumn = [self tableColumnWithIdentifier:@"image"]; 
    [theColumn setDataCell:[[NSImageCell alloc] initImageCell:nil]]; 
    [self reloadData];
}
@end
