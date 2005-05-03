#import "Wesnoth_Scenario_Editor_Prefix.h"
#import "WNIconListView.h"
#import "WNTerrains.h"
#import "WNUnits.h"
#import "HexUtils.h";

@implementation WNIconListView
-(void)awakeFromNib
{
//    [WNTerrains init];
//    [WNUnits init];
	[HexUtils initWithWidth: 70];
    NSTableColumn* theColumn = [self tableColumnWithIdentifier:@"image"]; 
    [theColumn setDataCell:[[NSImageCell alloc] initImageCell:nil]]; 
    [self reloadData];
}
@end
