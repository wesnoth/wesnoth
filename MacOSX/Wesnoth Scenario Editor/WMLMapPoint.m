//
//  WNMapPoint.m
//  Wesnoth Scenario Editor
//
//  Created by Marcus Phillips on Fri Mar 26 2004.
//  Copyright (c) 2004 __MyCompanyName__. All rights reserved.
//

#import "Wesnoth_Scenario_Editor_Prefix.h"
#import "WMLMapPoint.h"
#import "WNTerrains.h"
#import "WNUnits.h"
#import "WNCharacters.h"

@implementation WMLMapPoint
-(WMLMapPoint *)init
{
    [self initWithCode: 0 index: 0];
    return self;
}

-(WMLMapPoint *)initWithCode: (char)terrainCode index:(int)terrainIndex
{
    [super init];
    terrainID = terrainIndex;
    code = terrainCode;
    unitID = [[NSMutableDictionary alloc] init];
    unitType = [[NSMutableDictionary alloc] init];
    unitSide = [[NSMutableDictionary alloc] init];
    
    [unitID retain];
    [unitType retain];
    [unitSide retain];
    return self;
}

-(WMLMapPoint *)initFromCode: (char)terrainCode
{
    char tmpBuf[2];
    int terrainNo;
    
    tmpBuf[0] = terrainCode;
    tmpBuf[1] = 0;
    NSString *tmpString = [NSString stringWithUTF8String: tmpBuf];
    terrainNo = [WNTerrains terrainForCode: tmpString];
    if (terrainNo == -1) terrainNo = [WNTerrains terrainForCode:@"g"];
    return [self initWithCode: terrainCode index: terrainNo];    
}

-(void)dealloc
{
    [unitID autorelease];
    [unitType autorelease];
    [unitSide autorelease];
    [super dealloc];
}

-(void)encodeWithCoder: (NSCoder *)coder
{
    [coder encodeInt: code forKey:@"code"];
    [coder encodeObject: unitID forKey:@"unitID"];
    [coder encodeObject: unitType forKey:@"unitType"];
    [coder encodeObject: unitSide forKey:@"unitSide"];
}

- (id)initWithCoder:(NSCoder *)coder
{
    [super init];
    
    code = [coder decodeIntForKey:@"code"];
    terrainID = [WNTerrains terrainForChar:code];
    unitID = [[coder decodeObjectForKey:@"unitID"] retain];
    unitType = [[coder decodeObjectForKey:@"unitType"] retain];
    unitSide = [[coder decodeObjectForKey:@"unitSide"] retain];

    return self;
}

-(int)terrainID
{
	return terrainID;
}

-(void)setTerrainID:(int)newID
{
	terrainID = newID;
}

-(void)setCode:(char)newCode
{
	code = newCode;
}

-(char)getCode
{
    return code;
}

-(void)setCodeWithString:(NSString *)newCode
{
    if ([newCode characterAtIndex:0]=='"')
            {
            }else{
            code = [newCode characterAtIndex:0];
            }
}

-(void)setUnitID: (int)myID type:(int)myType side:(int)mySide forDifficulty:(NSString *)myDiff
{
    [unitID setObject:[[NSNumber alloc] initWithInt:myID] forKey:myDiff];
    [unitType setObject:[[NSNumber alloc] initWithInt:myType] forKey:myDiff];
    [unitSide setObject:[[NSNumber alloc] initWithInt:mySide] forKey:myDiff];
    fprintf(stderr, "MapPoint updated for unit:%d and difficulty: %s\n", myID, [myDiff UTF8String]);
}

-(void)clearUnitForDifficulty:(NSString *)myDiff
{
    [unitID removeObjectForKey:myDiff];
    [unitType removeObjectForKey:myDiff];
    [unitSide removeObjectForKey:myDiff];
    fprintf(stderr, "MapPoint updated, unit removed\n");
}

-(NSNumber *)getUnitIDForDifficulty:(NSString *)myDiff
{
    return [unitID objectForKey:myDiff];
}

-(NSNumber *)getUnitTypeForDifficulty:(NSString *)myDiff
{
    return [unitType objectForKey:myDiff];
}

-(NSNumber *)getUnitSideForDifficulty:(NSString *)myDiff
{
    return [unitSide objectForKey:myDiff];
}

-(void)updateMapPointUnits:(NSMutableArray *)unitConvert forDifficulty:(NSString *)myDiff
{
    NSNumber *tmpNum;
    tmpNum = [unitType objectForKey: myDiff];
    if (tmpNum !=nil)
        if ([tmpNum intValue]==1) 
            [unitID setObject:[unitConvert objectAtIndex:
                [[unitID objectForKey:myDiff] intValue]] forKey:myDiff];
}

-(void)mapPointUnitInfoToString:(NSMutableString *)desc characterTo:(NSMutableString *)cDesc characters:(WNCharacters *)chars
{
	[desc setString:@""];
	[cDesc setString:@""];
	[self mapPointCharacterInfoForDifficulty:@"Easy" toString:desc characterTo: cDesc characters:chars];
	[self mapPointCharacterInfoForDifficulty:@"Normal" toString:desc characterTo: cDesc characters:chars];
	[self mapPointCharacterInfoForDifficulty:@"Hard" toString:desc characterTo: cDesc characters:chars];
}

-(void)mapPointCharacterInfoForDifficulty:(NSString *)diff toString:(NSMutableString *)desc characterTo:(NSMutableString *)cDesc characters:(WNCharacters *)chars
{
	int tmpType, tmpID, tmpSide;
	
	NSNumber *tmpTypeNum = [self getUnitTypeForDifficulty:diff];
	
	if (tmpTypeNum == nil) return;
	
	tmpType = [tmpTypeNum intValue];
	tmpID = [[self getUnitIDForDifficulty:diff] intValue];
	tmpSide = [[self getUnitSideForDifficulty:diff] intValue];
	
	fprintf(stderr, "Map Info: type=%d, ID=%d, Side=%d\n", tmpType, tmpID, tmpSide);
	
	switch(tmpType)
		{
		case 1: // Unit
			[desc appendString:diff];
			[desc appendString:@": "];
			[desc appendString:[WNUnits unitNameAtIndex: tmpID]];
			[desc appendString:@" side: "];
			[desc appendString:[[[WNCampaign getActiveScenario] getSideAtIndex: tmpSide] getName]];
			[desc appendString:@"\n"];
			break;
		case 2: // Character
			[cDesc appendString:diff];
			[cDesc appendString:@": "];
			[cDesc appendString:[chars getNameAtIndex: tmpID]];
			[cDesc appendString:@" side: "];
			[cDesc appendString:[[[WNCampaign getActiveScenario] getSideAtIndex: tmpSide] getName]];
			[cDesc appendString:@"\n"];
			break;
		}
}

-(void)deleteSideWithDict:(NSMutableDictionary *)myDict
{
	int myMode = [[myDict objectForKey:@"mode"] intValue];  // 1= look, 2 = delete
	int side = [[myDict objectForKey:@"side"] intValue];
	int count = [[myDict objectForKey:@"count"] intValue];
	
	NSNumber *myNum;
	NSArray *diffs = [[NSArray alloc] initWithObjects:@"Easy",@"Normal",@"Hard",nil];
	
	int loop, thisSide;
	for (loop=0; loop<[diffs count]; loop++)
			{
			myNum = [unitSide objectForKey:[diffs objectAtIndex:loop]];
			if (myNum != nil)
				{
				thisSide = [myNum intValue];
				if (thisSide == side)
					{
					[myDict setObject:[[NSNumber alloc] initWithInt:(count+1)] forKey:@"count"];
					if (myMode==2) [self clearUnitForDifficulty: [diffs objectAtIndex:loop]];
					}else if (thisSide> side){
					[unitSide setObject:[[NSNumber alloc] initWithInt:(thisSide-1)] forKey:[diffs objectAtIndex:loop]];
					}
				}
			}
}

@end
