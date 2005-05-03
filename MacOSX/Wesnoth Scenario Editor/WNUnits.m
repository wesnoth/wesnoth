//
//  WNUnits.m
//  Wesnoth Scenario Editor
//
//  Created by Marcus Phillips on Fri Mar 26 2004.
//  Copyright (c) 2004 __MyCompanyName__. All rights reserved.
//

#import "Wesnoth_Scenario_Editor_Prefix.h"
#import "WNUnits.h"
#import "WNFileLocations.h"
#import "WMLTag.h"


@implementation WNUnits
    NSMutableArray *UnitTags;
    NSMutableArray *Units;
    NSMutableArray *UnitImages;
    NSMutableArray *ScaledImages;
    BOOL UnitsInitialised=NO;
    
+(void)init
{
    if (UnitsInitialised == NO)
        {
        WMLTag *tmpTag, *unitTag, *imageTag;
        NSString *unitsDir = [WNFileLocations unitDataLoc];
        NSDirectoryEnumerator *direnum = [[NSFileManager defaultManager] enumeratorAtPath:unitsDir];
        NSString *fileName;
        UnitTags = [[NSMutableArray alloc] init];
        [UnitTags retain];
        Units = [[NSMutableArray alloc] init];
        [Units retain];
        UnitImages = [[NSMutableArray alloc] init];
        [UnitImages retain];
        ScaledImages = [[NSMutableArray alloc] init];
        [ScaledImages retain];
        while (fileName = [direnum nextObject]) 
            {
            if (([[fileName pathExtension] isEqualToString:@"rtfd"])
					||([fileName characterAtIndex:0]=='.')
					||([fileName isEqualTo:@"CVS"])
                                        ||([fileName hasPrefix:@"PaxHeaders"]))
                {
                [direnum skipDescendents]; /* don't enumerate this directory */
                fprintf(stderr,"Skipping: %s\n", [fileName cString]);
                }else {
                fprintf(stderr, "Unit file:%s\n", [fileName cString]);
                tmpTag = [WMLTag alloc];
                [tmpTag initWithFile: [unitsDir stringByAppendingPathComponent:fileName] setTag:@"[file]"];
                [UnitTags addObject:tmpTag];
                unitTag = [tmpTag getChildAtIndex:0];
                [Units addObject: unitTag];
				NSMutableString  *ImageFile=[[NSMutableString alloc] initWithString: [[WNFileLocations imageLoc] stringByAppendingPathComponent:[unitTag getSettingFor:@"image"]]];
				[ImageFile replaceOccurrencesOfString:@"\"" withString:@"" options:nil range:NSMakeRange(0, [ImageFile length])];
                imageTag = [[NSImage alloc] initWithContentsOfFile:ImageFile];
                if (imageTag == nil) imageTag = [[NSImage alloc] initWithContentsOfFile:[[[NSBundle mainBundle] resourcePath] stringByAppendingPathComponent:@"missing-image.png"]];
                [UnitImages addObject: imageTag];
                [ScaledImages addObject: [imageTag copy]];
                }
            }
        UnitsInitialised = YES;
        }
}

+(BOOL)initialised
{
    return UnitsInitialised;
}

+(int)count
{
    return [Units count];
}

+(WMLTag *)unitAtIndex:(int)index
{
    return [Units objectAtIndex:index];
}

+(NSString *)unitNameAtIndex:(int)index
{
    return [[Units objectAtIndex:index] getSettingFor:@"id"];
}

+(int)unitIndexForName:(NSString *)findName
{
    int no = [Units count];
    int looper;
    WMLTag *tmpUnit;
    
    for (looper=0; looper < no ;looper++)
        {
        tmpUnit = [Units objectAtIndex:looper];
        if ([[tmpUnit getSettingFor:@"name"] isEqualTo: findName])
            {
            return looper;
            }
        }
    fprintf(stderr, "Error... can't find index for *%s* so substituting\n", [findName cString]);
    return 0;
}

+(NSMutableArray *)convertIndexArray: (NSMutableArray *)inArray
{
    NSMutableArray *outArray = [[NSMutableArray alloc] init];
    [outArray retain];
    
    int no = [inArray count];
    int looper;
    
    for (looper=0; looper<no ;looper++)
        [outArray addObject:
            [[NSString alloc] initWithString:
                [self unitNameAtIndex:[[inArray objectAtIndex:looper] intValue]]]];
    return outArray;
}

+(NSMutableArray *)convertNameArray: (NSMutableArray *)inArray
{
    NSMutableArray *outArray = [[NSMutableArray alloc] init];
    [outArray retain];
    
    int no = [inArray count];
    int looper;
    
    for (looper=0; looper<no ;looper++)
        [outArray addObject:
            [[NSNumber alloc] initWithInt:
                [self unitIndexForName:[inArray objectAtIndex:looper]]]];
    
    return outArray;
}

+(NSMutableArray *)getUnitNames
{
    NSMutableArray *outArray = [[NSMutableArray alloc] init];
    [outArray retain];
    
    int no = [Units count];
    int looper;
    
    for (looper=0; looper<no ;looper++)
        [outArray addObject:
            [[NSString alloc] initWithString:
                [self unitNameAtIndex:looper]]];
    
    return outArray;   
}

+(NSImage *)imageAtIndex:(int)index
{
    return [UnitImages objectAtIndex:index];
}

+(NSImage *)scaledImageAtIndex:(int)index
{
    return [ScaledImages objectAtIndex:index];
}

+(void)resizeTo:(NSSize)newSize
{
    int i;
    for (i=0; i<[ScaledImages count] ;i++)
        {
        [ScaledImages replaceObjectAtIndex:i withObject:[[UnitImages objectAtIndex:i] copy]];	// make a copy
        [[ScaledImages objectAtIndex:i] setScalesWhenResized:YES];	// Now scale        
        [[ScaledImages objectAtIndex:i] setSize:newSize];	// Now scale
        }
}
@end
