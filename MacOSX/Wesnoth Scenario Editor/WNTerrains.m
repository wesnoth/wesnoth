//
//  WNTerrains.m
//  Wesnoth Scenario Editor
//
//  Created by Marcus Phillips on Fri Mar 26 2004.
//  Copyright (c) 2004 __MyCompanyName__. All rights reserved.
//

#import "Wesnoth_Scenario_Editor_Prefix.h"
#import "WNTerrains.h"
#import "WMLTag.h"
#import "WNFileLocations.h"


@implementation WNTerrains
    static WMLTag *BuiltInTerrains=nil;
    static BOOL TerrainInitialised = NO;
    static NSMutableArray *Terrains=nil; // An array to the terrains

+(void)init
{
    int tmpCount, tmpLoop;
    WMLTag *tmpTag;
    NSString *Name=nil, *Image=nil, *Code=nil, *ImageFile=nil;
    WMLTerrain *tmpTerrain=nil;
    NSRange commaPos, cutPos;
    if (TerrainInitialised == NO)
        {
        [WNFileLocations init];
        BuiltInTerrains = [WMLTag alloc];
        [BuiltInTerrains retain];
        Terrains = [[NSMutableArray alloc] init];
        [Terrains retain];
        NSString *terrainPath = [[WNFileLocations dataLoc] stringByAppendingPathComponent:@"terrain.cfg"];
        fprintf(stderr, "Opening %s...", [terrainPath fileSystemRepresentation]);
        FILE *terrainCfg = fopen([terrainPath fileSystemRepresentation], "r");
        if (terrainCfg == nil)
            {
            fprintf(stderr, "failed\n");
			NSRunAlertPanel(@"Can't Find Data", @"I can't find Battle For Wesnoth, please run me from the same folder as the game. I shall have to quit now. Sorry.", @"Quit", nil, nil);
            [NSApp terminate];
			}else{
            fprintf(stderr, "opened\n");
            // Now we need to start reading in the file
            fclose(terrainCfg);
            }
        [BuiltInTerrains initWithFile: terrainPath setTag:@"[File]"];	// Initialise Terrains WML array
        tmpCount = [BuiltInTerrains childrenCount];
        //fprintf(stderr, "Children count for BuiltInterrains:%d\n", [BuiltInTerrains childrenCount]);
        for (tmpLoop = 0; tmpLoop<tmpCount; tmpLoop++)
            {
            tmpTag =[BuiltInTerrains getChildAtIndex:tmpLoop];
            if ([[tmpTag getTag] isEqualToString:@"[terrain]"])
                {
                Name = [tmpTag getSettingFor:@"name"];
                Image = [tmpTag getSettingFor:@"symbol_image"];
                Code = [tmpTag getSettingFor:@"char"];
                commaPos = [Image rangeOfString:@","];
                if (commaPos.length == 0)
                    {
                    ImageFile = [[[WNFileLocations terrainImageLoc] stringByAppendingPathComponent: Image] stringByAppendingPathExtension:@"png"];
                    }else{
                    cutPos.location=0;
                    cutPos.length = commaPos.location;
                    ImageFile = [[[WNFileLocations terrainImageLoc] stringByAppendingPathComponent:[Image substringWithRange:cutPos]] stringByAppendingPathExtension:@"png"];
                    }
                tmpTerrain = [WMLTerrain alloc];
                [tmpTerrain init:Name image:ImageFile code:Code userDefined:NO];
                [Terrains addObject:tmpTerrain];
                if ([Code isEqualTo:@"K"])
                    {
                    [Terrains addObject:[[WMLTerrain alloc] init:@"Keep for Side 1" 
                        image:ImageFile code:@"1" userDefined:NO]];
                    [Terrains addObject:[[WMLTerrain alloc] init:@"Keep for Side 2" 
                        image:ImageFile code:@"2" userDefined:NO]];
                    [Terrains addObject:[[WMLTerrain alloc] init:@"Keep for Side 3" 
                        image:ImageFile code:@"3" userDefined:NO]];
                    [Terrains addObject:[[WMLTerrain alloc] init:@"Keep for Side 4" 
                        image:ImageFile code:@"4" userDefined:NO]];
                    [Terrains addObject:[[WMLTerrain alloc] init:@"Keep for Side 5" 
                        image:ImageFile code:@"5" userDefined:NO]];
                    [Terrains addObject:[[WMLTerrain alloc] init:@"Keep for Side 6" 
                        image:ImageFile code:@"6" userDefined:NO]];
                    [Terrains addObject:[[WMLTerrain alloc] init:@"Keep for Side 7" 
                        image:ImageFile code:@"7" userDefined:NO]];
                    [Terrains addObject:[[WMLTerrain alloc] init:@"Keep for Side 8" 
                        image:ImageFile code:@"8" userDefined:NO]];
                    }
                }
            }
        TerrainInitialised = YES;
        fprintf(stderr, "Terrains initialised\n");
        }
}

+(WMLTerrain *)terrainAtIndex:(int)terrainNo
{
    return [Terrains objectAtIndex:terrainNo];
}

+(BOOL)initialised
{
    return TerrainInitialised;
}

+(int)count
{
    return [Terrains count];
}

+(int)terrainForCode: (NSString *)codeString
{
	int noTerrains = [Terrains count];
	int loop;
	WMLTerrain *tmpTerrain = nil;
	
	for (loop = 0; loop<noTerrains ;loop++)
		{
		tmpTerrain = [Terrains objectAtIndex: loop];
		if ([[tmpTerrain terrainCode] isEqualTo: codeString]) return loop;
		}
        fprintf(stderr, "Can't find terrain for code: %s\n", [codeString UTF8String]);
	return -1;
}

+(int)terrainForChar: (char)tmpCode
{
    char tmpBuf[2];
    tmpBuf[0] = tmpCode;
    tmpBuf[1] = 0;
    return [self terrainForCode:[NSString stringWithUTF8String:tmpBuf]];
}

+(NSString *)codeAtIndex:(int)index
{
    return [[Terrains objectAtIndex: index] terrainCode];
}

+(void)resizeTo:(NSSize)newSize
{
	[Terrains makeObjectsPerformSelector: @selector(resizeToID:) withObject:(id)&newSize];
}
@end
