#import "Wesnoth_Scenario_Editor_Prefix.h"
#import "WNUnitList.h"
#import "WNFileLocations.h"
#import "WMLTag.h"
#import "WNUnits.h"

@implementation WNUnitList
    
- (void)awakeFromNib
    {
    }

- (int)numberOfRowsInTableView:(NSTableView *)tableView
{
    if ([WNUnits initialised] != YES)
        {
        fprintf(stderr,"Units not initialised... initialising\n");
		[WNUnitList init];
        return 0;
        }
    return [WNUnits count];
}

- (id)tableView:(NSTableView *)tableView 
        objectValueForTableColumn:(NSTableColumn *)tableColumn row:(int)row
{
    NSString *identifier = [tableColumn identifier];
    WMLTag  *ThisUnit =[WNUnits unitAtIndex:row];
    if ([identifier isEqualTo:@"image"])
        {
        return [WNUnits imageAtIndex:row];
		}else{
        return [ThisUnit getSettingFor:identifier];
        }
    return [ThisUnit getSettingFor:@"id"];
}

@end
