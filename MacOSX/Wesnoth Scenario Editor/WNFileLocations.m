//
//  WNFileLocations.m
//  Wesnoth Scenario Editor
//
//  Created by Marcus Phillips on Thu Mar 25 2004.
//  Copyright (c) 2004 __MyCompanyName__. All rights reserved.
//

#import "Wesnoth_Scenario_Editor_Prefix.h"
#import "WNFileLocations.h"


@implementation WNFileLocations
    BOOL initialised=NO;
    NSString *imageLoc;
    NSString *dataLoc;
    NSString *unitDataLoc;
    NSString *scenarioLoc;
    NSString *terrainImageLoc;
	NSMutableString *saveLocation;
	NSMutableString *exportData;
	NSMutableString *exportUnits;
	NSMutableString *exportScenarios;
	NSMutableString *exportImages;

+(BOOL)init
{
NSString *MyLoc;
NSString *BFWLoc;
NSString *ContentsLoc;
NSString *ResourcesLoc;

if (initialised == NO)	// OK, we haven't been set up yet
    {
    fprintf(stderr, "Initialising LOCATION Info\n");
    MyLoc = [[NSBundle mainBundle] bundlePath];
    fprintf(stderr, "Bundle loc: %s\n", [MyLoc fileSystemRepresentation]);
	#ifdef embeddedEditor
	ResourcesLoc = [[NSBundle mainBundle] resourcePath];
	#else
    BFWLoc = [[MyLoc stringByDeletingLastPathComponent] stringByAppendingPathComponent:@"Battle For Wesnoth.app"];
    ContentsLoc = [BFWLoc stringByAppendingPathComponent:@"Contents"];
    ResourcesLoc = [ContentsLoc stringByAppendingPathComponent:@"Resources"];
	#endif
    dataLoc = [ResourcesLoc stringByAppendingPathComponent:@"data"];
    [dataLoc retain];
    unitDataLoc = [dataLoc stringByAppendingPathComponent:@"units"];
    [unitDataLoc retain];
    fprintf(stderr, "dataLoc:%s\n", [dataLoc fileSystemRepresentation]);
    imageLoc = [ResourcesLoc stringByAppendingPathComponent:@"images"];
    [imageLoc retain];
    terrainImageLoc = [imageLoc stringByAppendingPathComponent:@"terrain"];
    [terrainImageLoc retain];
    scenarioLoc = [dataLoc stringByAppendingPathComponent:@"scenarios"];
    [scenarioLoc retain];
	saveLocation = [[NSMutableString alloc] initWithString:@""];
	[saveLocation retain];
	exportData = [[NSMutableString alloc] init];
	[exportData retain];
	exportUnits = [[NSMutableString alloc] init];
	[exportUnits retain];
	exportScenarios = [[NSMutableString alloc] init];
	[exportScenarios retain];
	exportImages = [[NSMutableString alloc] init];
	[exportImages retain];
    initialised = YES;
    [super init];
    }

return YES;
}

+(NSString *)dataLoc
{
    if (initialised == NO) [self init];
    return dataLoc;
}

+(NSString *)unitDataLoc
{
    if (initialised == NO) [self init];
    return unitDataLoc;
}

+(NSString *)imageLoc
{
    if (initialised == NO) [self init];
    return imageLoc;
}

+(NSString *)terrainImageLoc
{
    if (initialised == NO) [self init];
    return terrainImageLoc;
}

+(void)setSaveLocation:(NSString *)saveLoc
{
	[saveLocation setString:saveLoc];
}

+(NSString *)getSaveLocation
{
	return saveLocation;
}

+(void)setExportDataLocation:(NSString *)newLoc
{
	[exportData setString:newLoc];
}

+(NSMutableString *)getExportDataLocation
{
	return exportData;
}

+(void)setExportUnitsLocation:(NSString *)newLoc
{
	[exportUnits setString:newLoc];
}

+(NSMutableString *)getExportUnitsLocation
{
	return exportUnits;
}

+(void)setExportScenariosLocation:(NSString *)newLoc
{
	[exportScenarios setString:newLoc];
}

+(NSMutableString *)getExportScenariosLocation
{
	return exportScenarios;
}

+(void)setExportImagesLocation:(NSString *)newLoc
{
	[exportImages setString:newLoc];
}

+(NSMutableString *)getExportImagesLocation
{
	return exportImages;
}

@end
