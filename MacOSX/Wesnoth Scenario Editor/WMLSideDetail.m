//
//  WMLSideList.m
//  Wesnoth Scenario Editor
//
//  Created by Marcus Phillips on Wed Mar 31 2004.
//  Copyright (c) 2004 __MyCompanyName__. All rights reserved.
//

#import "Wesnoth_Scenario_Editor_Prefix.h"
#import "WMLSideDetail.h"
#import "WNUnits.h"


@implementation WMLSideDetail
-(WMLSideDetail *)init
{
    [super init];	// Init parent first
    leaderName = [[NSString alloc] initWithString:@"Oh Great One"];
    leaderType = [[NSString alloc] initWithString:[[WNUnits unitAtIndex:0] getSettingFor:@"name"]];
    leaderID = 0;
    [leaderName retain];
    [leaderType retain];
    type = @"Human";
    [type retain];
    canRecruit = YES;
	recruitTypes = [[NSMutableArray alloc] init];
	[recruitTypes retain];
    return self;
}

-(void)dealloc
{
    [leaderName autorelease];
    [leaderType autorelease];
    [super dealloc];	// Dealloc parent last
}

-(void)encodeWithCoder: (NSCoder *)coder
{
    [coder encodeInt: gold forKey:@"gold"];
    [coder encodeObject: leaderName forKey:@"leaderName"];
    [coder encodeObject: leaderType forKey:@"leaderType"];
    [coder encodeObject: [WNUnits convertIndexArray:recruitTypes] forKey:@"recruitTypes"];
    [coder encodeFloat: agression forKey:@"agression"];
    [coder encodeObject: type forKey:@"type"];
    [coder encodeInt: team forKey:@"team"];
    [coder encodeInt: canRecruit forKey:@"canRecruit"];
}    

- (id)initWithCoder:(NSCoder *)coder
{
	fprintf(stderr, "[WMLSideDetail initWithCoder]\n");
    [super init];
    
    gold = [coder decodeIntForKey:@"gold"];
    leaderName = [[coder decodeObjectForKey:@"leaderName"] retain];
    leaderType = [[coder decodeObjectForKey:@"leaderType"] retain];
    leaderID = [WNUnits unitIndexForName:leaderType];
    recruitTypes = [WNUnits convertNameArray:[coder decodeObjectForKey:@"recruitTypes"]];
    agression = [coder decodeFloatForKey:@"agression"];
    type = [[coder decodeObjectForKey:@"type"] retain];
    team = [coder decodeIntForKey:@"team"];
    canRecruit = [coder decodeIntForKey:@"canRecruit"];
    
    return self;
}



-(NSString *)getLeaderName
{
    return leaderName;
}

-(void)setLeaderName:(NSString *)newName
{
    [leaderName autorelease];
    leaderName = [NSString stringWithString: newName];
    [leaderName retain];
}

-(int)getLeaderID
{
    return leaderID;
}

-(void)setLeaderTypeByID:(int)newID
{
    WMLTag *leaderTag;
    
    leaderID = newID;
    [leaderType release];
    leaderTag = [WNUnits unitAtIndex:newID];
    leaderType = [NSString stringWithString:[leaderTag getSettingFor:@"name"]];
    [leaderType retain];
}

-(int)getGold
{
    return gold;
}

-(void)setGold:(int)newGold
{
    gold = newGold;
}

-(NSString *)getType
{
    return type;
}

-(void)setType:(NSString *)newType
{
    [type release];
    type = [NSString stringWithString:newType];
    [type retain];
}

-(float)getAgression
{
    return agression;
}

-(void)setAgression:(float)newAgression
{
    agression = newAgression;
}

-(BOOL)getCanRecruit
{
    return canRecruit;
}

-(void)setCanRecruit:(BOOL)newCanRecruit
{
    canRecruit = newCanRecruit;
}

-(NSArray *)getRecruitTypes
{
	return [NSArray arrayWithArray: recruitTypes];
}

-(void)setRecruitTypes:(NSArray *)newSet
{
	[recruitTypes setArray:newSet];
}

-(void)setSideDetailTeam:(int)newTeam
{
    team = newTeam;
}

-(int)getSideDetailTeam
{
    return team;
}

-(void)exportSideDetailToFile:(FILE *)file withTeams:(NSMutableArray *)teams withIndex:(int)index withMap:(WMLMap *)map withDifficulty:(NSString *)diff withSide:(int)mySide withInfo:(NSMutableString *)advancedInfo
{
	int tmploop;
	WNCharacters *characters = [[WNCampaign getMainCampaign] getCharacters];
    fprintf(stderr, "... exporting side detail");
    fprintf(file, "[side]\n");
	fprintf(file, "side=%d\n", index);
    fprintf(file, "type=%s\n", [[WNUnits unitNameAtIndex: [characters getUnitTypeAtIndex:leaderID]] UTF8String]);
	[characters setInPlay:leaderID];
    fprintf(file, "description=%s\n",[leaderName UTF8String]);
    fprintf(file, "team_name=%s\n", [[teams objectAtIndex:team] UTF8String]);
    fprintf(file, "gold=%d\n", gold);
    fprintf(file, "controller=%s\n", [[type lowercaseString] UTF8String]);
    if ([type isEqualTo:@"AI"]) fprintf(file, "agression=%g\n", agression);
	if (canRecruit==YES)
		{
		fprintf(file, "canrecruit=1\n");
		fprintf(file, "recruit=");
		for (tmploop=0; tmploop<[recruitTypes count] ;tmploop++)
			{
			if (tmploop!=0) fprintf(file, ",");
			fprintf(file, [[WNUnits unitNameAtIndex:[[recruitTypes objectAtIndex:tmploop] intValue]] UTF8String]);
			}
		fprintf(file, "\n");
		}else{
		fprintf(file, "canrecruit=0\n");
		}
	fprintf(file, [[map getUnitListForDifficulty:diff forSide:mySide] UTF8String]);
	fprintf(file, [advancedInfo UTF8String]);
    fprintf(file, "\n[/side]\n");
    fprintf(stderr, "...exported\n");
}
@end
