#import "Wesnoth_Scenario_Editor_Prefix.h"
#import "WNScenarioList.h"

@implementation WNScenarioList
- (int)numberOfRowsInTableView:(NSTableView *)tableView
{
	WMLCampaign *myCamp;
	
	myCamp = [WNCampaign getMainCampaign];
	if (myCamp != nil)
		{
		return [myCamp count];
		}
	fprintf(stderr,"*** Value of nil returned for myCamp\n");
	return 0;
	
}

- (id)tableView:(NSTableView *)tableView 
        objectValueForTableColumn:(NSTableColumn *)tableColumn row:(int)row
{
	WMLCampaign *myCamp;
	WMLScenario *myScen, *nextScen;
	int nextIndex;
	
	NSString *identifier = [tableColumn identifier];
	myCamp = [WNCampaign getMainCampaign];
	if (myCamp != nil)
		{
		myScen = [myCamp getScenarioAtIndex: row];
		if ([identifier isEqualTo:@"name"])
			{
			return [myScen getName];
			}
		if ([identifier isEqualTo:@"On Win"])
			{
			nextIndex = [[myScen getDetailsFor:@"Easy"] getVictoryScenarioIndex];
			if (nextIndex == -1) return @"None";
			nextScen = [myCamp getScenarioAtIndex: nextIndex];
			return [nextScen getName];
			}
		if ([identifier isEqualTo:@"On Continue"])
			{
			nextIndex = [[myScen getDetailsFor:@"Easy"] getContinueScenarioIndex];
			if (nextIndex == -1) return @"None";
			nextScen = [myCamp getScenarioAtIndex: nextIndex];
			return [nextScen getName];
			}
		return @"unknown identifier";
		}
	fprintf (stderr,"Value of mainCampaign returned as nil\n");
    return @"ERROR - Value Not Assigned";
}

- (void)tableView:(NSTableView *)aTableView 
	setObjectValue:(id)anObject forTableColumn:(NSTableColumn *)tableColumn row:(int)rowIndex
{
	WMLCampaign *myCamp;
	WMLScenario *myScen;
	
    NSString *identifier = [tableColumn identifier];
	if ([identifier isEqualTo:@"name"])
		{
		myCamp = [WNCampaign getMainCampaign];
		myScen = [myCamp getScenarioAtIndex: rowIndex];
		[myScen setName:anObject];
		}

}
@end
