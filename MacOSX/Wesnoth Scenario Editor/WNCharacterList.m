#import "Wesnoth_Scenario_Editor_Prefix.h"
#import "WNCharacterList.h"
#import "WNUnits.h"

@implementation WNCharacterList
- (int)numberOfRowsInTableView:(NSTableView *)tableView
{
    return [[[WNCampaign getMainCampaign] getCharacters] count];
}

- (id)tableView:(NSTableView *)tableView 
        objectValueForTableColumn:(NSTableColumn *)tableColumn row:(int)row
{
    NSString *identifier = [tableColumn identifier];
    WNCharacters *tmpChars = [[WNCampaign getMainCampaign] getCharacters];
    if ([identifier isEqualTo:@"image"])
        {
        return [WNUnits imageAtIndex:[tmpChars getUnitTypeAtIndex:row]];
        }else if ([identifier isEqualTo:@"name"]){
        return [tmpChars getNameAtIndex:row];
        }
    return @"";
}

- (void)tableView:(NSTableView *)aTableView 
	setObjectValue:(id)anObject forTableColumn:(NSTableColumn *)tableColumn row:(int)rowIndex
{
	WNCharacters *tmpChar = [[WNCampaign getMainCampaign] getCharacters];
	[tmpChar setNameAtIndex: rowIndex to: anObject];
}

@end
