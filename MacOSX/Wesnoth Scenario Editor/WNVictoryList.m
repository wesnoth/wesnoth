#import "Wesnoth_Scenario_Editor_Prefix.h"
#import "WNVictoryList.h"

@implementation WNVictoryList

- (int)numberOfRowsInTableView:(NSTableView *)tableView
{
    return [[[WNCampaign getActiveScenario] getVictoryConditionsFor: [WNCampaign getDifficulty]] count];
}

- (id)tableView:(NSTableView *)tableView objectValueForTableColumn:(NSTableColumn *)tableColumn row:(int)row
{
    return [[[WNCampaign getActiveScenario] getVictoryConditionsFor: [WNCampaign getDifficulty]] objectAtIndex: row];
}

- (void)tableView:(NSTableView *)aTableView 
	setObjectValue:(id)anObject forTableColumn:(NSTableColumn *)tableColumn row:(int)rowIndex
{
    [[[[WNCampaign getActiveScenario] getVictoryConditionsFor: [WNCampaign getDifficulty]] objectAtIndex: rowIndex] setString: anObject];
}



@end
