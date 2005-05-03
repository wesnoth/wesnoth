//
//  WNEventLookup.m
//  Wesnoth Scenario Editor
//
//  Created by Marcus Phillips on Wed May 26 2004.
//  Copyright (c) 2004 __MyCompanyName__. All rights reserved.
//

#import "Wesnoth_Scenario_Editor_Prefix.h"
#import "WNEventLookup.h"
#import "WMLEvent.h"


@implementation WNEventLookup
	WMLEvent *currentEvent;
	NSMutableDictionary *dict;
	NSMutableArray *types;
	int WNEinitialised;
	NSPoint mapLoc;
+(void)init
{
	dict = [[[NSMutableDictionary alloc] init] retain];
	[dict setObject:[[NSNumber alloc] initWithInt:0] forKey:@"None"];
	[dict setObject:[[NSNumber alloc] initWithInt:1] forKey:@"character"];
	[dict setObject:[[NSNumber alloc] initWithInt:2] forKey:@"unit"];
	[dict setObject:[[NSNumber alloc] initWithInt:3] forKey:@"side"];
	[dict setObject:[[NSNumber alloc] initWithInt:4] forKey:@"maploc"];
	[dict setObject:[[NSNumber alloc] initWithInt:5] forKey:@"info"];
	[dict setObject:[[NSNumber alloc] initWithInt:6] forKey:@"text"];
	types = [[[NSMutableArray alloc] init] retain];
	[types addObject:@"none"];
	[types addObject:@"character"];
	[types addObject:@"unit"];
	[types addObject:@"side"];
	[types addObject:@"maploc"];
	[types addObject:@"info"];
	[types addObject:@"text"];
}

+(int)typeForSetting:(NSString *)setting
{
	return [[dict objectForKey:setting] intValue];
}

+(NSString *)settingForType:(int)type
{
	return [types objectAtIndex:type];
}

+(void)setMapLoc:(NSPoint)mapPos
{
	mapLoc = mapPos;
}

+(NSPoint)getMapLoc
{
	return mapLoc;
}

+(void)setCurrentEvent:(WMLEvent *)thisEvent
{
	currentEvent = thisEvent;
}

+(WMLEvent *)getCurrentEvent
{
	return currentEvent;
}

@end

