//
//  WNScenario.m
//  Wesnoth Scenario Editor
//
//	This class represents a single scenario
//
//  Created by Marcus Phillips on Fri Mar 26 2004.
//  Copyright (c) 2004 __MyCompanyName__. All rights reserved.
//

#import "Wesnoth_Scenario_Editor_Prefix.h"
#import "WNScenario.h"


@implementation WNScenario

-(void)init	// Initialises a scenario
{
    name = [[NSString alloc] initWithString:@"untitled"];
    [name retain];
    map = [WMLMap alloc];
    [map initWithSize: 50 by:50];
    [map retain];
    winScenario = nil;
    continueScenario = nil;
    loseScenario = nil;
}
@end
