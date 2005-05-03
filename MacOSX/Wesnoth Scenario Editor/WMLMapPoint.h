//
//  WNMapPoint.h
//  Wesnoth Scenario Editor
//
//  Created by Marcus Phillips on Fri Mar 26 2004.
//  Copyright (c) 2004 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "WNCharacters.h"


@interface WMLMapPoint : NSObject {
    int terrainID;	// terrain ID
    char code;		// Ascii Code
    NSMutableDictionary *unitID;	// Unit IDs for any unit
    NSMutableDictionary *unitType;	// Unit type, 1 = standard, 2 = character 
    NSMutableDictionary *unitSide;	// Side for the unit
}
-(WMLMapPoint *)init;
-(WMLMapPoint *)initWithCode: (char)terrainCode index:(int)terrainIndex;
-(WMLMapPoint *)initFromCode: (char)terrainCode;
-(void)dealloc;
-(int)terrainID;
-(void)setTerrainID:(int)newID;
-(void)setCode:(char)newCode;
-(char)getCode;
-(void)setCodeWithString:(NSString *)newCode;
-(void)setUnitID: (int)myID type:(int)myType side:(int)mySide forDifficulty:(NSString *)myDiff;
-(void)clearUnitForDifficulty:(NSString *)myDiff;
-(NSNumber *)getUnitIDForDifficulty:(NSString *)myDiff;
-(NSNumber *)getUnitTypeForDifficulty:(NSString *)myDiff;
-(NSNumber *)getUnitSideForDifficulty:(NSString *)myDiff;
-(void)updateMapPointUnits:(NSMutableArray *)unitConvert forDifficulty:(NSString *)myDiff;
-(void)mapPointUnitInfoToString:(NSMutableString *)desc characterTo:(NSMutableString *)cDesc characters:(WNCharacters *)chars;
-(void)mapPointCharacterInfoForDifficulty:(NSString *)diff toString:(NSMutableString *)desc characterTo:(NSMutableString *)cDesc characters:(WNCharacters *)chars;
-(void)deleteSideWithDict:(NSMutableDictionary *)myDict;
@end
