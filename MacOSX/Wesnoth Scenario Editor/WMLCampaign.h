//
//  WMLCampaign.h
//  Wesnoth Scenario Editor
//
//  Created by Marcus on Sat Mar 27 2004.
//  Copyright (c) 2004 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "WNCharacters.h"
#import "WMLSide.h"
#import "WMLMap.h"
#import "WMLScenario.h"

@interface WMLCampaign : NSObject {
	NSMutableString *name;  // name of campaign
	NSMutableString *prefix;	// prefix code
	NSMutableArray *scenarios;  // scenario list
	WNCharacters *characters;	// Characters
	int activeScenarioIndex;
	WMLScenario *activeScenario;
	WMLMap *activeMap;
	NSImage *campaignIcon;
	NSMutableDictionary *difficultyIcons;
	NSMutableDictionary *difficultyDescriptions;
	
}

-(WMLCampaign *)init;
-(void)dealloc;
-(void)encodeWithCoder: (NSCoder *)coder;
-(int)count;
-(WMLScenario *)getScenarioAtIndex: (int)index;
-(int) addScenario;
-(void)deleteScenario:(int)index;
-(WMLMap *)getActiveMap;
-(void)setActiveScenario:(int)scenarioIndex;
-(WMLScenario *)getActiveScenario;
-(WMLSide *)getActiveSide;
-(WNCharacters *)getCharacters;
-(void)setPrefix:(NSString *)newPrefix;
-(NSMutableString *)getPrefix;
-(void)setName:(NSString *)newName;
-(NSMutableString *)getName;
-(void)setIcon: (NSImage *)newIcon;
-(NSImage *)getIcon;
-(void)setDifficultyIcon: (NSImage *)newIcon forDifficulty:(NSString *)newDiff;
-(NSImage *)getDifficultyIconForDifficulty:(NSString *)newDiff;
-(void)setDifficultyDescription: (NSString *)newText forDifficulty:(NSString *)newDiff;
-(NSString *)getDifficultyDescriptionForDifficulty:(NSString *)newDiff;
-(void)exportToFolder:(NSString *)folder withDialog:(NSWindow *)saveDialog andIndicator:(NSProgressIndicator *)saveProgress;
@end
