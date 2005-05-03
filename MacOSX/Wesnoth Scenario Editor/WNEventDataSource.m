//
//  WNEventDataSource.m
//  Wesnoth Scenario Editor
//
//  Created by Marcus Phillips on Tue May 25 2004.
//  Copyright (c) 2004 __MyCompanyName__. All rights reserved.
//

#import "Wesnoth_Scenario_Editor_Prefix.h"
#import "WNEventDataSource.h"


@implementation WNEventDataSource
- (int)outlineView:(NSOutlineView *)outlineView numberOfChildrenOfItem:(WMLEvent *)item
{
	if (item == nil)
		{
		fprintf(stderr, "Checking no events in scenario for outline view:%d\n", [[WNCampaign getActiveScenario] noEvents]);
		return [[WNCampaign getActiveScenario] noEvents];
		}
	fprintf(stderr, "Checking no event children: %d\n", [item noEventChildren]);
	return [item noEventChildren];
}
- (id)outlineView:(NSOutlineView *)outlineView child:(int)index ofItem:(WMLEvent *)item
{
	if (item == nil)
		{
//		fprintf(stderr, "Getting child %d of main list = %d\n", index, [[WNCampaign getActiveScenario] getEventAtIndex:index]);
		return [[WNCampaign getActiveScenario] getEventAtIndex:index];
		}
//	fprintf(stderr, "Getting child %d of object %d = \n", index, item,[item getEventChildAtIndex:index] );
	return [item getEventChildAtIndex:index];
}

- (BOOL)outlineView:(NSOutlineView *)outlineView isItemExpandable:(WMLEvent *)item
{
	return [item isEventATag];
}

- (id)outlineView:(NSOutlineView *)outlineView objectValueForTableColumn:(NSTableColumn *)tableColumn byItem:(WMLEvent *)item
{
    NSString *identifier = [tableColumn identifier];
	//return @"TEST";

	if ([identifier isEqualTo:@"Name"])
		{
		return [item getEventName];
		}
	if ([identifier isEqualTo:@"Value"])
		{
		return [item getEventStringValue];
		}
	return @"Error Unknown identifier";
}

- (void)outlineView:(NSOutlineView *)outlineView setObjectValue:(id)object forTableColumn:(NSTableColumn *)tableColumn byItem:(id)item
 {
    NSString *identifier = [tableColumn identifier];

	if ([identifier isEqualTo:@"Name"])
		{
		[item setEventName:object];
		}else{
		fprintf(stderr, "Error - trying to set outlineview item illegally\n");
		}
 }
@end
