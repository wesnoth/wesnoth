//
//  WNMap.m
//  Wesnoth Scenario Editor
//
//  Created by Marcus Phillips on Fri Mar 26 2004.
//  Copyright (c) 2004 __MyCompanyName__. All rights reserved.
//

#import "Wesnoth_Scenario_Editor_Prefix.h"
#import "WMLMap.h"
#import "WMLMapPoint.h"
#import "WNTerrains.h"
#import "WNUnits.h"


@implementation WMLMap

-(WMLMap *)initWithSize: (int)xsize by: (int)ysize
{
    int loopx=0, loopy=0, grassIndex=0;
    NSMutableArray *row=nil;
    WMLMapPoint *point=nil;
    
    [super init];
    fprintf(stderr, "WMLMap initWithSize\n");
    grassIndex = [WNTerrains terrainForCode:@"g"];
    mapData = [[NSMutableArray alloc]init];
    [mapData retain];
    width = xsize;
    height = ysize;
    for (loopy=0 ;loopy<height ;loopy++)
        {
        row = [[NSMutableArray alloc] init];
        for (loopx=0; loopx<width ;loopx++)
            {
            point = [[WMLMapPoint alloc] initWithCode: 'g' index:grassIndex];
            [row addObject: point];
            }
        [mapData addObject: row];
        } 
    fprintf(stderr, "WMLMap initialised\n");
    return self;
}

-(void)dealloc
{
    [mapData autorelease];
    [super dealloc];
}

-(void)encodeWithCoder: (NSCoder *)coder
{
    [coder encodeInt: width forKey:@"width"];
    [coder encodeInt: height forKey:@"height"];
    [coder encodeObject: mapData forKey:@"mapData"];
}

- (id)initWithCoder:(NSCoder *)coder
{
	fprintf(stderr, "[WMLMap initWithCoder]\n");
    [super init];
    
    width = [coder decodeIntForKey:@"width"];
    height = [coder decodeIntForKey:@"height"];
    mapData = [[coder decodeObjectForKey:@"mapData"] retain];
    
    return self;
}


-(void)setWidth: (int)newWidth
{
	[self resizeTo: newWidth by: height];
}

-(void)setHeight: (int)newHeight
{
//	fprintf(stderr, "Setting map height to %d\n", newHeight);
	[self resizeTo: width  by: newHeight];
}
-(void)resizeTo: (int)xSize by: (int)ySize
{
	int loopX=0, loopY=0, grassIndex=0;
	NSMutableArray *row;
	NSRange rangeToRemove;

	grassIndex = [WNTerrains terrainForCode:@"g"];
        
        fprintf(stderr, "Resizing map to %d by %d\n", xSize, ySize);
	
	// first remove any extra rows
	if (ySize<height)
		{
		rangeToRemove.location = ySize;
		rangeToRemove.length = (height - ySize);
		[mapData removeObjectsInRange: rangeToRemove];
		height = ySize;
		}
	
	// Now let us go through each row
	for (loopY=0; loopY<height ;loopY++)
		{
		row = [mapData objectAtIndex:loopY];
		if (xSize<width)
			{
			rangeToRemove.location = xSize;
			rangeToRemove.length = (width - xSize);
			[row removeObjectsInRange: rangeToRemove];
			}
		if (xSize>width)
			for (loopX=width; loopX<xSize ;loopX++) [row addObject:[[WMLMapPoint alloc] initWithCode: 'g' index:grassIndex]];
		}
		
	// Now add new rows if needed
	if (ySize>height)
		for (loopY = height; loopY<ySize ;loopY++)
			{
			row = [[NSMutableArray alloc] init];
			for (loopX=0; loopX<xSize ;loopX++) [row addObject:[[WMLMapPoint alloc] initWithCode: 'g' index:grassIndex]];
			[mapData addObject: row];
			}
	height = ySize;
	width = xSize;
	[[NSNotificationCenter defaultCenter] postNotificationName:@"mapChanged" object:self];
	
}

-(NSMutableArray *)getRow: (int)index
{
	if (index < [mapData count])
		{
		return [mapData objectAtIndex: index];
		}
	return nil;
}

-(int)getWidth
{
	return width;
}

-(int)getHeight
{
	return height;
}

-(void)setPointToTerrain:(int)idNo x:(int)x y:(int)y
{
	WMLTerrain *tmpTerrain = [WNTerrains terrainAtIndex:idNo];
	NSMutableArray *row = [mapData objectAtIndex:y];
	WMLMapPoint *mapItem = [row objectAtIndex: x];
	[mapItem setTerrainID: idNo];
	[mapItem setCodeWithString: [tmpTerrain terrainCode]];	
}

-(void)setPointToUnit:(int)idNo type:(int)newType side:(int)newSide x:(int)x y:(int)y
{
	NSMutableArray *row = [mapData objectAtIndex:y];
	WMLMapPoint *mapItem = [row objectAtIndex: x];
        if ([WNPreferences getPlaceEasy] == YES) 
            [mapItem setUnitID: idNo type:newType side:newSide forDifficulty:@"Easy"];
        if ([WNPreferences getPlaceNormal] == YES)
            [mapItem setUnitID: idNo type:newType side:newSide forDifficulty:@"Normal"];
        if ([WNPreferences getPlaceHard] == YES) 
            [mapItem setUnitID: idNo type:newType side:newSide forDifficulty:@"Hard"];
        fprintf(stderr, "Placed unit at %d x %d\n", x, y);
}

-(void)clearUnitAtPointX:(int) x y:(int)y
{
    NSMutableArray *row = [mapData objectAtIndex:y];
    WMLMapPoint *mapItem = [row objectAtIndex: x];
    if ([WNPreferences getPlaceEasy] == YES)
        [mapItem clearUnitForDifficulty:@"Easy"];
    if ([WNPreferences getPlaceNormal] == YES)
        [mapItem clearUnitForDifficulty:@"Normal"];
    if ([WNPreferences getPlaceHard] == YES)
        [mapItem clearUnitForDifficulty:@"Hard"];
    fprintf(stderr, "Clearing units from x:%d, y:%d\n", x, y);
}

-(void)updateMapPointUnitsWith:(NSArray *)lookup
{
    int y,x;
    NSMutableArray *row;
    WMLMapPoint *mapItem;
    
    for (y=0; y<height ;y++)
        {
        row = [mapData objectAtIndex:y];
        for (x=0; x<width ; x++)
            {
            mapItem = [row objectAtIndex:x];
            }
        }
}

-(void)exportToFile: (NSString *)saveFile
{
    int loopX=0,loopY=0;
    NSMutableArray *row;
    WMLMapPoint *tmpPoint;
    fprintf(stderr, "about to export map to:%s\n", [saveFile UTF8String]);
    FILE *efp = fopen([saveFile fileSystemRepresentation], "w");
    
    for (loopY=0; loopY<height ;loopY++)
        {
        row = [mapData objectAtIndex: loopY];
        for (loopX=0; loopX<width ;loopX++)
            {
            tmpPoint = [row objectAtIndex: loopX];
            fputc([tmpPoint getCode], efp);
            }
        fputc('\n', efp);
        }
    fclose(efp);
}

-(NSMutableString *)exportToString
{
    int loopX=0,loopY=0;
    NSMutableArray *row;
    WMLMapPoint *tmpPoint;
    char buffer[20000];
    int curLoc = 0;
    fprintf(stderr, "about to export map to string\n");
    
    for (loopY=0; loopY<height ;loopY++)
        {
        row = [mapData objectAtIndex: loopY];
        for (loopX=0; loopX<width ;loopX++)
            {
            tmpPoint = [row objectAtIndex: loopX];
            buffer[curLoc++] = [tmpPoint getCode];
            }
        buffer[curLoc++] = '\n';
        }
    curLoc = curLoc - 1;
    buffer[curLoc] = 0;
    NSMutableString *tmpMap = [[NSMutableString alloc] initWithUTF8String:buffer];
    [tmpMap retain];
    return tmpMap;
}

-(NSPoint)importFromFile: (NSString *)importFile
{
    int xs=0, ys=0, tmpLen=0, xpos=0,ypos=0;
    char tmpStr[1024];
    NSMutableArray *row;
    
    FILE *fp = fopen([importFile fileSystemRepresentation], "r");
    fprintf(stderr, "Examining file: %s  for import\n", [importFile UTF8String]);
    while(fscanf(fp,"%s",tmpStr)!=EOF)
        {
        tmpLen = strlen(tmpStr);
        if ((tmpLen>xs)&&(tmpLen>5)&&(xs==0)) xs = tmpLen;
        if (tmpLen == xs) ys++;
        }
    fclose(fp);
    fprintf(stderr,"Calculated size: %d by %d\n", xs, ys);
    [self resizeTo: xs by: ys];
    
    // second pass parsing :D
    fp = fopen([importFile fileSystemRepresentation], "r");
    fprintf(stderr, "Parsing file: %s  for import\n", [importFile UTF8String]);
    while((fscanf(fp,"%s",tmpStr)!=EOF)&&(ypos<ys))
        {
        row = [mapData objectAtIndex: ypos];
        tmpLen = strlen(tmpStr);
        fprintf(stderr, "Parsing: %s\n", tmpStr);
        if (tmpLen == xs)
            {
            fprintf(stderr,"Assigning\n");
            for (xpos=0; xpos<xs ;xpos++)
                {
                [row replaceObjectAtIndex: xpos withObject:[[WMLMapPoint alloc] initFromCode:tmpStr[xpos]]];
                }
            }
        ypos++;
        }
    fclose(fp);
    fprintf(stderr, "Parsing complete\n");
    [[NSNotificationCenter defaultCenter] postNotificationName:@"mapChanged" object:self];
    NSPoint returnPoint;
    returnPoint.x = xs;
    returnPoint.y = ys;
    return returnPoint;
}

-(void)updateUnitsWithLookup:(NSMutableArray *)unitConvert
{
    int x,y;
    WMLMapPoint *tmpPoint;
    NSMutableArray *row;
    
    for (y=0; y<height ;y++)
        {
        row = [mapData objectAtIndex:y];
        for (x=0; x<width ;x++)
            {
            tmpPoint = [row objectAtIndex:x];
            [tmpPoint updateMapPointUnits: unitConvert forDifficulty:@"Easy"];
            [tmpPoint updateMapPointUnits: unitConvert forDifficulty:@"Normal"];
            [tmpPoint updateMapPointUnits: unitConvert forDifficulty:@"Hard"];
            }
        }
}

-(NSMutableString *)getUnitListForDifficulty:(NSString *)diff forSide:(int)side
{
    int loopX=0,loopY=0, tmpUnit, tmpType, tmpSide;
    NSMutableArray *row;
	NSMutableString *data = [[[NSMutableString alloc] init] retain];
	NSMutableString *t1 = [[NSMutableString alloc] init];
	NSMutableString *t2 = [[NSMutableString alloc] init];
	NSMutableString *t3 = [[NSMutableString alloc] init];
    WMLMapPoint *tmpPoint;
	WNCharacters *chars = [[WNCampaign getMainCampaign] getCharacters];
    char buffer[20000];
    fprintf(stderr, "about to compile unit info to string\n");
    
    for (loopY=0; loopY<height ;loopY++)
        {
        row = [mapData objectAtIndex: loopY];
        for (loopX=0; loopX<width ;loopX++)
            {
            tmpPoint = [row objectAtIndex: loopX];
            tmpUnit = [[tmpPoint getUnitIDForDifficulty:diff] intValue];
			tmpType = [[tmpPoint getUnitTypeForDifficulty:diff] intValue];
			tmpSide = [[tmpPoint getUnitSideForDifficulty:diff] intValue];
			if (tmpSide == side)
				{
				switch(tmpType)
					{
					case 1: // This is a unit
						sprintf(buffer, "[unit]\ntype=%s\nx=%d\ny=%d\nside=%d\n[/unit]\n\n",
							[[WNUnits unitNameAtIndex:tmpUnit] UTF8String], (loopX+1), (loopY+1), (tmpSide+1));
						[data appendString:[NSString stringWithUTF8String: buffer]];
						break;
					case 2: // This is a character
						if ([[chars getAIAtIndex:tmpUnit] isEqualTo:@"default"])
							{
							[t3 setString:@""];
							}else{
							[t3 setString:@"ai_special=\""];
							[t3 appendString:[chars getAIAtIndex:tmpUnit]];
							[t3 appendString:@"\"\n"];
							}
						
						[WNPreferences fillWithTraitTags: t1 forTrait:[chars getTrait:1 atIndex:tmpUnit]];
						[WNPreferences fillWithTraitTags: t2 forTrait:[chars getTrait:2 atIndex:tmpUnit]];
						sprintf(buffer, "[unit]\ndescription=%s\ntype=%s\nx=%d\ny=%d\nside=%d\ntraits_description=\"%s,%s\"\n[modifications]\n%s%s[/modifications]\n%s[/unit]\n\n",
							[[chars getNameAtIndex:tmpUnit] UTF8String],
							[[WNUnits unitNameAtIndex:[chars getUnitTypeAtIndex:tmpUnit]] UTF8String], 
							(loopX+1), (loopY+1), (tmpSide+1),
							[[chars getTrait:1 atIndex:tmpUnit] UTF8String],
							[[chars getTrait:2 atIndex:tmpUnit] UTF8String],
							[t1 UTF8String],[t2 UTF8String],[t3 UTF8String]);
						[data appendString:[NSString stringWithUTF8String: buffer]];						
						[chars setInPlay:tmpUnit];
						break;
					}
				}
            }
        }
    return data;
}


-(WMLMapPoint *)getMapPointAtX:(int)x Y:(int)y
{
	return [[mapData objectAtIndex: y] objectAtIndex:x];
}

-(void)iterateSelector:(SEL)mySelector withObject:(id)myObject
{
	int x,y;
	
	NSMutableArray *row;
	
	for(y=0; y<height ;y++)
		{
		row = [mapData objectAtIndex:y];
		for (x=0; x<width ;x++) [[row objectAtIndex:x] performSelector:mySelector withObject:myObject];
		}
}

@end
