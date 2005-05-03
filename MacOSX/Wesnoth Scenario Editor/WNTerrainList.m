#import "Wesnoth_Scenario_Editor_Prefix.h"
#import "WNTerrainList.h"
#import "WMLTag.h";
#import "WNFileLocations.h";
#import "WMLTerrain.h";
#import "WNTerrains.h";

@implementation WNTerrainList
-(void)awakeFromNib	// When we start we need to load the terrain info
    {
    }
    
- (int)numberOfRowsInTableView:(NSTableView *)tableView
{
    if ([WNTerrains initialised] != YES)
        {
        fprintf(stderr,"Terrains not initialised initialising\n");
		[WNTerrains init];
        }
    return [WNTerrains count];
}

- (id)tableView:(NSTableView *)tableView 
        objectValueForTableColumn:(NSTableColumn *)tableColumn row:(int)row
{
    NSString *identifier = [tableColumn identifier];
    WMLTerrain  *ThisTerrain =[WNTerrains terrainAtIndex:row];
    if ([identifier isEqualTo:@"name"])
        {
        return [ThisTerrain name];
        }
    if ([identifier isEqualTo:@"image"])
        {
        return [ThisTerrain image];
        }
    return [ThisTerrain name];
    
}
@end
