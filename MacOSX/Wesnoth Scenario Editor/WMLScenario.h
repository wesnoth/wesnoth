//
//  WMLScenario.h
//  Wesnoth Scenario Editor
//
//  Created by Marcus on Sat Mar 27 2004.
//  Copyright (c) 2004 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "Wesnoth_Scenario_Editor_Prefix.h"
#import "WMLScenarioDetails.h"


@interface WMLScenario : NSObject {
	NSMutableString *name;
	int winID;
	int loseID;
	int continueID;
	int activeSideIndex;
	WMLSide *activeSide;
	NSMutableArray *sides;
	NSMutableArray *teams;
	NSMutableDictionary *victoryConditions;
	NSMutableDictionary *defeatConditions;
	NSMutableDictionary *details;
	WMLMap *map;
	NSMutableArray *events;
	NSMutableString *advancedInfo;
}
-(WMLScenario *)init;
-(void)dealloc;
-(void)encodeWithCoder: (NSCoder *)coder;
- (id)initWithCoder:(NSCoder *)coder;
-(NSString *)getName;
-(void)setName:(NSString *)newName;
-(WMLMap *)getMap;
-(int)getSidesCount;
-(WMLSide *)getSideAtIndex:(int)index;
-(int)addSide;
-(WMLSide *)getActiveSide;
-(void)setActiveSide:(int)sideNo;
-(void)deleteSide:(int)sideToDie;
-(NSMutableArray *)getDefeatConditionsFor:(NSString *)diffLevel;
-(NSMutableArray *)getVictoryConditionsFor:(NSString *)diffLevel;
-(WMLScenarioDetails *)getDetailsFor:(NSString *)diffLevel;
-(NSMutableArray *)getTeams;
-(void)setScenarioAdvancedInfo: (NSString *)newInfo;
-(NSString *)getScenarioAdvancedInfo;
-(void)exportToFolderWithPrefix:(NSString *)prefix scenarios:(NSMutableArray *)scenarios;
-(void)exportObjectivesForDifficulty:(NSString *)diff forDesc:(NSString *)diffDesc toFile:(FILE *)fp prefix:(NSString *)prefix;
-(NSMutableDictionary *)getNextScenarioText;
-(void)addNextScenarioInfoForDifficulty:(NSString *)diff toVictoryString:(NSMutableString *)vInfo toContinueString:(NSMutableString *)cInfo withPrefix:(NSString *)prefix;
-(void)checkDeletedScenario:(id)index;
-(NSMutableArray *)getEvents;
-(WMLEvent *)getEventAtIndex:(int)index;
-(int)noEvents;
-(void)addNewEvent;

@end
