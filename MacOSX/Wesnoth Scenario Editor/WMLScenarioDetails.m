//
//  WMLScenarioDetails.m
//  Wesnoth Scenario Editor
//
//  Created by Marcus Phillips on Mon Apr 05 2004.
//  Copyright (c) 2004 __MyCompanyName__. All rights reserved.
//

#import "Wesnoth_Scenario_Editor_Prefix.h"
#import "WMLScenarioDetails.h"


@implementation WMLScenarioDetails

- (WMLScenarioDetails *)init
{
    noTurns = 10;
    victoryScenarioIndex = -1;
    continueScenarioIndex = -1;
    return self;
}

- (void)dealloc
{
}

-(void)encodeWithCoder: (NSCoder *)coder
{
    [coder encodeInt: noTurns forKey:@"noTurns"];
    [coder encodeInt: victoryScenarioIndex forKey:@"victoryScenarioIndex"];
    [coder encodeInt: continueScenarioIndex forKey:@"continueScenarioIndex"];
}

- (id)initWithCoder:(NSCoder *)coder
{
    [super init];
    
    noTurns = [coder decodeIntForKey:@"noTurns"];
    victoryScenarioIndex  = [coder decodeIntForKey:@"victoryScenarioIndex"];
    continueScenarioIndex = [coder decodeIntForKey:@"continueScenarioIndex"];
    
    return self;
}

- (void)setNoTurns: (int)newTurns
{
    noTurns = newTurns;
}

- (int)getNoTurns
{
    return noTurns;
}

- (void)setVictoryScenarioIndex: (int)newIndex
{
    victoryScenarioIndex = newIndex;
}

- (int)getVictoryScenarioIndex
{
    return victoryScenarioIndex;
}

- (void)setContinueScenarioIndex: (int)newIndex
{
    continueScenarioIndex = newIndex;
}

- (int)getContinueScenarioIndex
{
    return continueScenarioIndex;
}

@end
