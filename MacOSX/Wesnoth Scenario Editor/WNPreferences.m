//
//  WNPreferences.m
//  Wesnoth Scenario Editor
//
//  Created by Marcus on Sun Mar 28 2004.
//  Copyright (c) 2004 __MyCompanyName__. All rights reserved.
//

#import "Wesnoth_Scenario_Editor_Prefix.h"
#import "WNPreferences.h"
#import "WMLTerrain.h"


@implementation WNPreferences
	WMLTerrain *selectedTerrain;
	int selectedTerrainID;
	int selectedUnitID;
	int selectedUnitSide;
	int selectedCharacterID;
	BOOL placeEasy;
	BOOL placeNormal;
	BOOL placeHard;
	NSProgressIndicator *loadIndicator;
	NSDocumentController *docControl;
	float unitOpacityLevel;

+(void)setTerrain:(WMLTerrain *)setTerrain
{
	selectedTerrain = setTerrain;
}

+(void)setTerrainID:(int)setTerrainID
{
	selectedTerrainID = setTerrainID;
}

+(WMLTerrain *)getTerrain
{
	return selectedTerrain;
}
+(int)getTerrainID
{
	return selectedTerrainID;
}

+(void)setUnitID:(int)setUnitID
{
    selectedUnitID = setUnitID;
}

+(int)getUnitID
{
    return selectedUnitID;
}

+(void)setCharacterID:(int)setCharacterID
{
    selectedCharacterID = setCharacterID;
}

+(int)getCharacterID
{
    return selectedCharacterID;
}

+(void)setPlaceEasy:(BOOL)newVal
{
    placeEasy = newVal;
}

+(BOOL)getPlaceEasy
{
    return placeEasy;
}

+(void)setPlaceNormal:(BOOL)newVal
{
    placeNormal = newVal;
}

+(BOOL)getPlaceNormal
{
    return placeNormal;
}

+(void)setPlaceHard:(BOOL)newVal
{
    placeHard = newVal;
}

+(BOOL)getPlaceHard
{
    return placeHard;
}

+(void)setUnitSide:(int)newVal
{
    selectedUnitSide = newVal;
}

+(int)getUnitSide
{
    return selectedUnitSide;
}

+(void)setLoadIndicator:(NSProgressIndicator *)newIndicator
{
	loadIndicator = newIndicator;
	fprintf(stderr, "Load indicator set\n");
}

+(NSProgressIndicator *)getLoadIndicator
{
	return loadIndicator;
}

+(void)fillWithTraitTags:(NSMutableString *)dest forTrait:(NSString *)trait
{
	// currently long winded, should really automate
	
	if ([trait isEqualTo:@"loyal"])
		[dest setString:@"[trait]\nid=\"loyal\"\nname=\"loyal\"\n[effect]\napply_to=\"loyal\"\n[/effect]\n[/trait]\n"];
	if ([trait isEqualTo:@"intelligent"])
		[dest setString:@"[trait]\nid=\"intelligent\"\nname=\"intelligent\"\n[effect]\napply_to=\"max_experience\"\nincrease=\"-20%\"\n[/effect]\n[/trait]\n"];
	if ([trait isEqualTo:@"strong"])
		[dest setString:@"[trait]\nid=\"strong\"\nname=\"strong\"\n[effect]\napply_to=\"attack\"\nincrease_damage=\"1\"\nrange=\"short\"\n[/effect]\n[effect]\napply_to=\"hitpoints\"\nheal_full=\"yes\"\nincrease_total=\"2\"\n[/effect]\n[/trait]\n"];
	if ([trait isEqualTo:@"quick"])
		[dest setString:@"[trait]\nid=\"quick\"\nname=\"quick\"\n[effect]\napply_to=\"movement\"\nincrease=\"1\"\n[/effect]\n[effect]\napply_to=\"hitpoints\"\nheal_full=\"yes\"\nincrease_total=\"-10%\"\n[/effect]\n[/trait]\n"];
	if ([trait isEqualTo:@"resilient"])
		[dest setString:@"[trait]\nid=\"resilient\"\nname=\"resilient\"\n[effect]\napply_to=\"hitpoints\"\nheal_full=\"yes\"\nincrease_total=\"7\"\n[/effect]\n[/trait]\n"];
}

+(void)setDocControl:(NSDocumentController *)newDoc
{
	[docControl autorelease];
	docControl = newDoc;
}

+(NSDocumentController *)getDocControl
{
	return docControl;
}

+(void)setUnitOpacity:(float)newOp
{
	unitOpacityLevel = newOp;
}

+(float)getUnitOpacity
{
	return unitOpacityLevel;
}

@end
