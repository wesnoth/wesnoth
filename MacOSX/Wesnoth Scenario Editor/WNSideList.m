#import "Wesnoth_Scenario_Editor_Prefix.h"
#import "WNSideList.h"
#import "WMLSide.h"
#import "WMLCampaign.h"

@implementation WNSideList
- (int)numberOfRowsInTableView:(NSTableView *)tableView
{
	WMLCampaign *myCamp;
        WMLScenario *myScenario;
	
	myCamp = [WNCampaign getMainCampaign];
        myScenario = [myCamp getActiveScenario];
	if (myCamp != nil)
		{
		return [myScenario getSidesCount];
		}
	fprintf(stderr,"*** Value of nil returned for myScenario\n");
	return 0;
}

- (id)tableView:(NSTableView *)tableView 
        objectValueForTableColumn:(NSTableColumn *)tableColumn row:(int)row
{
	WMLCampaign *myCamp;
	WMLScenario *myScen;
        WMLSide *mySide;
	
	NSString *identifier = [tableColumn identifier];
	myCamp = [WNCampaign getMainCampaign];
        if (myCamp !=nil)
            {
            myScen = [myCamp getActiveScenario];
            if (myScen != nil)
                    {
                    mySide = [myScen getSideAtIndex: row];
                    if ([identifier isEqualTo:@"name"])
                        {
                        return [mySide getName];
                        }
                    if ([identifier isEqualTo:@"number"])
                        {
                        NSNumber *tmpNum = [[NSNumber alloc] initWithInt:(row+1)];
                        return tmpNum;
                        }

                    return @"unknown identifier";
                    }
            fprintf (stderr,"Value of activeScenario returned as nil\n");
            }
    return @"ERROR - Value Not Assigned";
}

- (void)tableView:(NSTableView *)aTableView 
	setObjectValue:(id)anObject forTableColumn:(NSTableColumn *)tableColumn row:(int)rowIndex
{
    WMLCampaign *myCamp;
    WMLScenario *myScen;
    WMLSide *mySide;
	
    NSString *identifier = [tableColumn identifier];
    if ([identifier isEqualTo:@"name"])
            {
            myCamp = [WNCampaign getMainCampaign];
            myScen = [myCamp getActiveScenario];
            mySide = [myScen getActiveSide];
            [mySide setName:anObject];
            }
    [[NSNotificationCenter defaultCenter] postNotificationName:@"sidesChanged" object:self];
}


@end
