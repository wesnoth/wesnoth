//
//  WMLScenarioDetails.h
//  Wesnoth Scenario Editor
//
//  Created by Marcus Phillips on Mon Apr 05 2004.
//  Copyright (c) 2004 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>


@interface WMLScenarioDetails : NSObject {
    int noTurns;
    int victoryScenarioIndex;
    int continueScenarioIndex;
}

- (WMLScenarioDetails *)init;
- (void)dealloc;
-(void)encodeWithCoder: (NSCoder *)coder;
- (id)initWithCoder:(NSCoder *)coder;
- (void)setNoTurns: (int)newTurns;
- (int)getNoTurns;
- (void)setVictoryScenarioIndex: (int)newIndex;
- (int)getVictoryScenarioIndex;
- (void)setContinueScenarioIndex: (int)newIndex;
- (int)getContinueScenarioIndex;
@end
