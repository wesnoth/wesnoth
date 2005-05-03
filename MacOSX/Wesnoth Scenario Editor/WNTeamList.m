#import "Wesnoth_Scenario_Editor_Prefix.h"
#import "WNTeamList.h"

@implementation WNTeamList
- (int)numberOfRowsInTableView:(NSTableView *)tableView
{
	NSMutableArray *myTeams;
	
	myTeams = [[[WNCampaign getMainCampaign] getActiveScenario] getTeams];
	if (myTeams != nil)
		{
		return [myTeams count];
		}
	fprintf(stderr,"*** Value of nil returned for myTeams\n");
	return 0;
}

- (id)tableView:(NSTableView *)tableView 
        objectValueForTableColumn:(NSTableColumn *)tableColumn row:(int)row
{
    NSMutableArray *myTeams;
    
    myTeams = [[[WNCampaign getMainCampaign] getActiveScenario] getTeams];
    
    if (myTeams !=nil)
        {
        return [myTeams objectAtIndex: row];
        }
    return @"ERROR - Value Not Assigned";
}

- (void)tableView:(NSTableView *)aTableView 
	setObjectValue:(id)anObject forTableColumn:(NSTableColumn *)tableColumn row:(int)rowIndex
{
    NSMutableArray *myTeams;
    
    myTeams = [[[WNCampaign getMainCampaign] getActiveScenario] getTeams];
    
    if (myTeams !=nil)
        {
        [myTeams replaceObjectAtIndex: rowIndex withObject: anObject];
        }
}



@end
