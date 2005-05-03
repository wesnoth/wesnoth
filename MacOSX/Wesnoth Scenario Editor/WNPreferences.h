//
//  WNPreferences.h
//  Wesnoth Scenario Editor
//
//  Created by Marcus on Sun Mar 28 2004.
//  Copyright (c) 2004 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "WMLTerrain.h"


@interface WNPreferences : NSObject {

}
+(void)setTerrain:(WMLTerrain *)setTerrain;
+(void)setTerrainID:(int)setTerrainID;
+(WMLTerrain *)getTerrain;
+(int)getTerrainID;
+(void)setUnitID:(int)setUnitID;
+(int)getUnitID;
+(void)setCharacterID:(int)setCharacterID;
+(int)getCharacterID;
+(void)setPlaceEasy:(BOOL)newVal;
+(BOOL)getPlaceEasy;
+(void)setPlaceNormal:(BOOL)newVal;
+(BOOL)getPlaceNormal;
+(void)setPlaceHard:(BOOL)newVal;
+(BOOL)getPlaceHard;
+(void)setUnitSide:(int)newVal;
+(int)getUnitSide;
+(void)setLoadIndicator:(NSProgressIndicator *)newIndicator;
+(NSProgressIndicator *)getLoadIndicator;
+(void)fillWithTraitTags:(NSMutableString *)dest forTrait:(NSString *)trait;
+(void)setDocControl:(NSDocumentController *)newDoc;
+(NSDocumentController *)getDocControl;
+(void)setUnitOpacity:(float)newOp;
+(float)getUnitOpacity;
@end
