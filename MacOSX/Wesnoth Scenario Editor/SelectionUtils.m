//
//  SelectionUtils.m
//  Wesnoth Scenario Editor
//  This is a collection of utilities to make handling list selections more transparent from 10.2 to 10.3+
//  At a later date the code is easily replacable to ensure compatability
//
//  Created by Marcus on Sat Apr 03 2004.
//  Copyright (c) 2004 __MyCompanyName__. All rights reserved.
//

#import "Wesnoth_Scenario_Editor_Prefix.h"
#import "SelectionUtils.h"


@implementation SelectionUtils
+(NSArray *)selectionsFromView:(NSTableView *)myView
{
	fprintf(stderr, "selectionsFromView\n");
	return [[myView selectedRowEnumerator] allObjects];
}

+(void)selectionsToView:(NSTableView *)myView indexArray:(NSArray *)newSelections
{
	int count = [newSelections count];
	int rowSelected;
	int loop;
	
	[myView deselectAll: self];
	fprintf(stderr, "selectionsToView\n");
	for (loop=0; loop<count ;loop++)
		{
		rowSelected = [[newSelections objectAtIndex:loop] intValue];
		[myView selectRow:rowSelected byExtendingSelection:YES];
		}
}

@end
