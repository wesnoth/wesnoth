//
//  WMLCampaign.m
//  Wesnoth Scenario Editor
//
//  Created by Marcus on Sat Mar 27 2004.
//  Copyright (c) 2004 __MyCompanyName__. All rights reserved.
//

#import "Wesnoth_Scenario_Editor_Prefix.h"
#import "WMLScenario.h"
#import "WNUnits.h"
#import "WNFileLocations.h"
#include "WNUtils.h"
#import "WMLCampaign.h"


@implementation WMLCampaign
-(WMLCampaign *)init
	{
	//[HexUtils initWithWidth:70];
        fprintf(stderr, "WMLCampaign initialising\n");
        [super init];
	name = [[NSMutableString alloc] initWithString:@"untitled"];
	[name retain];
	prefix = [[NSMutableString alloc] initWithString:@"Replace Me"];
	[prefix retain];
	scenarios = [[NSMutableArray alloc] init];
	[scenarios retain];
	activeScenario = [[WMLScenario alloc] init];
        [activeScenario retain];
	activeScenarioIndex = 0;
	activeMap = [activeScenario getMap];
	[scenarios addObject:activeScenario];
        characters = [[WNCharacters alloc] init];
        fprintf(stderr, "WMLCampaign initialised\n");
	NSSize tmpSize;
	tmpSize.width = 16;
	tmpSize.height = 16;
	campaignIcon = [[NSImage alloc] initWithSize:tmpSize];
	difficultyIcons = [[NSMutableDictionary alloc] init];
	[difficultyIcons retain];
	[difficultyIcons setObject:[[NSImage alloc] initWithSize:tmpSize] forKey:@"Easy"];
	[difficultyIcons setObject:[[NSImage alloc] initWithSize:tmpSize] forKey:@"Normal"];
	[difficultyIcons setObject:[[NSImage alloc] initWithSize:tmpSize] forKey:@"Hard"];
	difficultyDescriptions = [[NSMutableDictionary alloc] init];
	[difficultyDescriptions retain];
	[difficultyDescriptions setObject:[[NSString alloc] initWithString:@"Easy"] forKey:@"Easy"];
	[difficultyDescriptions setObject:[[NSString alloc] initWithString:@"Normal"] forKey:@"Normal"];
	[difficultyDescriptions setObject:[[NSString alloc] initWithString:@"Hard"] forKey:@"Hard"];
	return self;
	}
        
-(void)dealloc
{
    [name autorelease];
	[prefix autorelease];
    [scenarios autorelease];
    [activeScenario autorelease];
	[campaignIcon autorelease];
	[difficultyIcons autorelease];
	[difficultyDescriptions autorelease];
	[super dealloc];
}

-(void)encodeWithCoder: (NSCoder *)coder
{
	fprintf(stderr, "Enconding WMLCampaign\n");
    [coder encodeObject: name forKey:@"name"];
    [coder encodeObject: prefix forKey:@"prefix"];
    [coder encodeObject: scenarios forKey:@"scenarios"];
    [coder encodeObject: characters forKey:@"characters"];
    [coder encodeInt: activeScenarioIndex forKey:@"activeScenarioIndex"];
    [coder encodeObject: activeMap forKey:@"activeMap"];
    [coder encodeObject: [WNUnits getUnitNames] forKey:@"unitLookup"];
    [coder encodeObject: campaignIcon forKey:@"campaignIcon"];
    [coder encodeObject: difficultyIcons forKey:@"difficultyIcons"];
    [coder encodeObject: difficultyDescriptions forKey:@"difficultyDescriptions"];
    fprintf(stderr, "WMLCampaign encoded\n");
}

- (id)initWithCoder:(NSCoder *)coder
{
	fprintf(stderr, "WMLCampaign loading...");
    [super init];
    
    name = [[coder decodeObjectForKey:@"name"] retain];
    prefix = [[coder decodeObjectForKey:@"prefix"] retain];
    if (prefix == nil) prefix = [[[NSMutableString alloc] initWithString:@"Unknown Campaign"] retain];
    scenarios = [[coder decodeObjectForKey:@"scenarios"] retain];
    characters = [[coder decodeObjectForKey:@"characters"] retain];
    activeScenarioIndex = [coder decodeIntForKey:@"activeScenarioIndex"];
    activeMap = [[coder decodeObjectForKey:@"activeMap"] retain];
    
    // OK, now we need to re-instate the map units
    NSMutableArray *unitLookup = [[coder decodeObjectForKey:@"unitLookup"] retain];
    NSMutableArray *unitConvert = [[[NSMutableArray alloc] init] retain];
    int looper;
    
    for (looper=0; looper<[unitLookup count]; looper++)
        [unitConvert addObject: 
            [[NSNumber alloc] initWithInt:[WNUnits unitIndexForName:[unitLookup objectAtIndex:looper]]]];
    for (looper=0; looper<[scenarios count] ;looper++)
        [[[scenarios objectAtIndex:looper] getMap] updateUnitsWithLookup: unitConvert];	
		
	
	// OK, now the icons... new, so check for non-valid
	NSSize tmpSize;
	tmpSize.width = 16;
	tmpSize.height = 16;
	if ((campaignIcon = [[coder decodeObjectForKey:@"campaignIcon"] retain])==nil)
		{
		fprintf(stderr, "campaignIcon = nil...");
		campaignIcon = [[[NSImage alloc] initWithSize:tmpSize] retain];
		}
	if ((difficultyIcons = [[coder decodeObjectForKey:@"difficultyIcons"] retain])==nil)
		{
		fprintf(stderr, "difficultyIcons = nil...");
		difficultyIcons = [[NSMutableDictionary alloc] init];
		[difficultyIcons retain];
		[difficultyIcons setObject:[[NSImage alloc] initWithSize:tmpSize] forKey:@"Easy"];
		[difficultyIcons setObject:[[NSImage alloc] initWithSize:tmpSize] forKey:@"Normal"];
		[difficultyIcons setObject:[[NSImage alloc] initWithSize:tmpSize] forKey:@"Hard"];
		}
	if ((difficultyDescriptions = [[coder decodeObjectForKey:@"difficultyDescriptions"] retain])==nil)
		{
		fprintf(stderr, "difficultyDescriptions = nil...");
		[difficultyDescriptions retain];
		[difficultyDescriptions setObject:[[NSString alloc] initWithString:@"Easy"] forKey:@"Easy"];
		[difficultyDescriptions setObject:[[NSString alloc] initWithString:@"Normal"] forKey:@"Normal"];
		[difficultyDescriptions setObject:[[NSString alloc] initWithString:@"Hard"] forKey:@"Hard"];
		}
	fprintf(stderr, "WMLCampaign loaded\n");
    return self;
}
	
-(int)count
	{
	return [scenarios count];
	}
	
-(WMLScenario *)getScenarioAtIndex: (int)index
	{
	return [scenarios objectAtIndex: index];
	}
	
-(int) addScenario
	{   // Adds a scenario and returns the index
	[scenarios addObject:[[WMLScenario alloc] init]];
	return ([scenarios count]-1);	
	}
	
-(void)deleteScenario:(int)index
	{
	[scenarios removeObjectAtIndex: index];
	NSNumber *num = [[NSNumber alloc] initWithInt:index];
	[scenarios makeObjectsPerformSelector:@selector(checkDeletedScenario:) withObject:num];
	}
        
-(WMLMap *)getActiveMap
{
	return activeMap;
}

-(void)setActiveScenario:(int)scenarioIndex
{
    fprintf(stderr, "Setting active scenario to: %d\n", scenarioIndex);
    activeScenarioIndex = scenarioIndex;
    activeScenario = [scenarios objectAtIndex: scenarioIndex];
    activeMap = [activeScenario getMap];
}

-(WMLScenario *)getActiveScenario
{
    return activeScenario;
}

-(WMLSide *)getActiveSide
{
    return [activeScenario getActiveSide];
}

-(WNCharacters *)getCharacters
{
    return characters;
}

-(void)setPrefix:(NSString *)newPrefix
{
	[prefix setString:newPrefix];
}

-(NSMutableString *)getPrefix
{
	return prefix;
}

-(void)setName:(NSString *)newName
{
	[name setString: newName];
}

-(NSMutableString *)getName
{
	return name;
}

-(void)setIcon: (NSImage *)newIcon
{
	campaignIcon = newIcon;
	[campaignIcon retain];
}

-(NSImage *)getIcon
{
	return campaignIcon;
}

-(void)setDifficultyIcon: (NSImage *)newIcon forDifficulty:(NSString *)newDiff
{
	[difficultyIcons setObject:newIcon forKey:newDiff];
}

-(NSImage *)getDifficultyIconForDifficulty:(NSString *)newDiff
{
	return [difficultyIcons objectForKey:newDiff];
}

-(void)setDifficultyDescription: (NSString *)newText forDifficulty:(NSString *)newDiff
{
	[difficultyDescriptions setObject: newText forKey:newDiff];
}

-(NSString *)getDifficultyDescriptionForDifficulty:(NSString *)newDiff
{
	return [difficultyDescriptions objectForKey:newDiff];
}

-(void)exportToFolder:(NSString *)folder withDialog:(NSWindow *)saveDialog andIndicator:(NSProgressIndicator *)saveProgress
// This is the whole reason d'etre of the editor :D
{
	FILE *fp;
	int scenarioCount = [scenarios count];
	NSMutableString *tmpString = [[NSMutableString alloc] init];
	NSMutableString *iconString = [[NSMutableString alloc] initWithString:[prefix stringByAppendingString:@"CampaignIcon.png"]];
	NSFileManager *fileManager = [NSFileManager defaultManager];
	WMLScenario *tmpScenario;
	if ([fileManager createDirectoryAtPath: folder attributes:nil])
		{
		// First we want to have an actual progress indicator, advancing with each scenario
		[saveProgress setMinValue: 0.0];
		[saveProgress setMaxValue: scenarioCount-1];
		[saveProgress setDoubleValue: 0.0];
		[saveProgress setIndeterminate: false];
		
		// Create packaging sub-folder
		NSString *tmpName = [WNUtils replaceSpaces:[prefix stringByAppendingString:name]];
		[tmpString setString:[folder stringByAppendingPathComponent:tmpName]];
		[fileManager createDirectoryAtPath: tmpString attributes:nil];
		
		// Now create sub-folders
		[WNFileLocations setExportDataLocation: [tmpString stringByAppendingPathComponent:@"data"]];
		[WNFileLocations setExportUnitsLocation:[tmpString stringByAppendingPathComponent:@"units"]];
		[WNFileLocations setExportScenariosLocation: [tmpString stringByAppendingPathComponent:@"scenarios"]];
		[WNFileLocations setExportImagesLocation: [folder stringByAppendingPathComponent:@"images"]];
		[fileManager createDirectoryAtPath: [WNFileLocations getExportDataLocation] attributes:nil];
		[fileManager createDirectoryAtPath: [WNFileLocations getExportUnitsLocation] attributes:nil];
		[fileManager createDirectoryAtPath: [WNFileLocations getExportScenariosLocation] attributes:nil];
		[fileManager createDirectoryAtPath: [WNFileLocations getExportImagesLocation] attributes:nil];
		
		// Now we export the campaign.cfg
		fp = fopen([[[[WNFileLocations getExportScenariosLocation]
                    stringByAppendingPathComponent:tmpName]
			stringByAppendingPathExtension:@"cfg"] UTF8String], "w\0");
		fprintf(fp, "[campaign]\n");
		fprintf(fp, "id=%s\n", [tmpName UTF8String]);
		fprintf(fp, "name=%s\n", [name UTF8String]);
		tmpScenario = [scenarios objectAtIndex:0];
		fprintf(fp, "icon=%s\n", [iconString UTF8String]);
		fprintf(fp, "define=%s\n", [[tmpName uppercaseString] UTF8String]);
		fprintf(fp, "first_scenario=%s\n", [[WNUtils replaceSpaces: [prefix stringByAppendingString:[tmpScenario getName]]] UTF8String]);
		fprintf(fp, "difficulties=EASY,NORMAL,HARD\n");
		fprintf(fp, "difficulty_descriptions=&%s,%s,(easiest);*&%s,%s;&%s,%s,(hardest)\n",
                    [[prefix stringByAppendingString:@"difficultyIcon1.png"] UTF8String], 
                        [[difficultyDescriptions objectForKey:@"Easy"] UTF8String],
                    [[prefix stringByAppendingString:@"difficultyIcon2.png"] UTF8String], 
                        [[difficultyDescriptions objectForKey:@"Normal"] UTF8String],
                    [[prefix stringByAppendingString:@"difficultyIcon3.png"] UTF8String], 
                        [[difficultyDescriptions objectForKey:@"Hard"] UTF8String]);
		fprintf(fp, "[/campaign]\n");
		
		// Now add extra packaging info
		char *tmpUTF8;
		tmpUTF8 = (char *)[tmpName UTF8String];
		fprintf (fp,"{campaigns/%s/units}\n{~campaigns/%s/units}\n", tmpUTF8, tmpUTF8);
		fprintf (fp,"#ifdef %s\n", [[tmpName uppercaseString] UTF8String]);
		fprintf (fp, "{campaigns/%s/}\n{~campaigns/%s/}\n", tmpUTF8, tmpUTF8);
		fprintf (fp, "{campaigns/%s/scenarios}\n{~campaigns/%s/scenarios}\n", tmpUTF8, tmpUTF8); 
		fprintf (fp, "[binary_path]\npath=data/campaigns/%s/\n[/binary_path]\n#endif\n", tmpUTF8);

		fclose(fp);
		// Now output the pngs
		[WNUtils writeImage:campaignIcon
			toPNGFile:[[WNFileLocations getExportImagesLocation]
				stringByAppendingPathComponent:iconString]];
		fprintf(stderr, "About to export difficulty PNGs\n");
		[WNUtils writeImage:[difficultyIcons objectForKey:@"Easy"]
			toPNGFile:[[[WNFileLocations getExportImagesLocation]
				stringByAppendingPathComponent:prefix]
					stringByAppendingString:@"difficultyIcon1.png"]];
		[WNUtils writeImage:[difficultyIcons objectForKey:@"Normal"]
			toPNGFile:[[[WNFileLocations getExportImagesLocation]
				stringByAppendingPathComponent:prefix]
					stringByAppendingString:@"difficultyIcon2.png"]];
		[WNUtils writeImage:[difficultyIcons objectForKey:@"Hard"]
			toPNGFile:[[[WNFileLocations getExportImagesLocation]
				stringByAppendingPathComponent:prefix]
					stringByAppendingString:@"difficultyIcon3.png"]];
		
		// Now go through and export each scenario
		int looper;
		for (looper = 0; looper<scenarioCount ;looper++)
			{
                        fprintf(stderr, "Exporting scenario:%d\n", looper);
			[saveProgress setDoubleValue: looper];
			[[scenarios objectAtIndex:looper] exportToFolderWithPrefix:prefix scenarios:scenarios];
			}
		}else{
		fprintf(stderr, "Folder creation failed\n");
		}
}

@end
