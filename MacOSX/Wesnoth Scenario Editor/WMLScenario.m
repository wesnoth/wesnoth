//
//  WMLScenario.m
//  Wesnoth Scenario Editor
//
//  Created by Marcus on Sat Mar 27 2004.
//  Copyright (c) 2004 __MyCompanyName__. All rights reserved.
//

#import "Wesnoth_Scenario_Editor_Prefix.h"
#import "WMLScenario.h"
#import "WMLMap.h"
#import "WMLSide.h"
#import "WNFileLocations.h"
#import "WNUtils.h"


@implementation WMLScenario
-(WMLScenario *)init
{
	[super init];	// init parent first
	fprintf(stderr, "WMLScenario initialising\n");
	name = [[NSMutableString alloc] initWithString:@"untitled scenario"];
	[name retain];
	winID=0;
	loseID = 0;
	continueID=0;
	activeSide = 0;
	map = [[WMLMap alloc] initWithSize: 50 by:50];
	[map retain];
	sides = [[NSMutableArray alloc] init];
	[sides retain];
	activeSide = [[WMLSide alloc] init];
	[sides addObject: activeSide];	// add a blank side
	fprintf(stderr, "WMLScenario - initialising conditions\n");
	victoryConditions = [[NSMutableDictionary alloc] init];
	[victoryConditions retain];
	defeatConditions = [[NSMutableDictionary alloc] init];
	[defeatConditions retain];
	[defeatConditions setObject: [[NSMutableArray alloc] init] forKey:@"Easy"];
	[defeatConditions setObject: [[NSMutableArray alloc] init] forKey:@"Normal"];
	[defeatConditions setObject: [[NSMutableArray alloc] init] forKey:@"Hard"];
	[victoryConditions setObject: [[NSMutableArray alloc] init] forKey:@"Easy"];
	[victoryConditions setObject: [[NSMutableArray alloc] init] forKey:@"Normal"];
	[victoryConditions setObject: [[NSMutableArray alloc] init] forKey:@"Hard"];
	
	fprintf(stderr, "WMLScenario - initialising conditions\n");
	details = [[NSMutableDictionary alloc] init];
	[details retain];
	[details setObject: [[WMLScenarioDetails alloc] init] forKey:@"Easy"];
	[details setObject: [[WMLScenarioDetails alloc] init] forKey:@"Normal"];
	[details setObject: [[WMLScenarioDetails alloc] init] forKey:@"Hard"];
	teams = [[NSMutableArray alloc] init];
	[teams retain];
	[teams addObject:@"Wesnoth Team"];
	advancedInfo = [[[NSMutableString alloc] init] retain];
	events = [[[NSMutableArray alloc] init] retain];
	return self;
}

-(void)dealloc
{
    [name autorelease];
    [map autorelease];
    [sides autorelease];
    [defeatConditions autorelease];
    [victoryConditions autorelease];
    [details autorelease];
	[events autorelease];
    [super dealloc];	// dealloc parent first
}

-(void)encodeWithCoder: (NSCoder *)coder
{
    [coder encodeObject: name forKey:@"name"];
    [coder encodeInt: winID forKey:@"winID"];
    [coder encodeInt: loseID forKey:@"loseID"];
    [coder encodeInt: continueID forKey:@"continueID"];
    [coder encodeInt: activeSideIndex forKey:@"activeSideIndex"];
    [coder encodeObject: activeSide forKey:@"activeSide"];
    [coder encodeObject: sides forKey:@"sides"];
    [coder encodeObject: teams forKey:@"teams"];
    [coder encodeObject: victoryConditions forKey:@"victoryConditions"];
    [coder encodeObject: defeatConditions forKey:@"defeatConditions"];
    [coder encodeObject: details forKey:@"details"];
    [coder encodeObject: map forKey:@"map"];
	[coder encodeObject: advancedInfo forKey:@"advancedInfo"];
	[coder encodeObject: events forKey:@"events"];
}

- (id)initWithCoder:(NSCoder *)coder
{
	fprintf(stderr, "[WMLScenario] initWithCoder\n");
    [super init];
    
    name = [[coder decodeObjectForKey:@"name"] retain];
    winID = [coder decodeIntForKey:@"winID"];
    loseID = [coder decodeIntForKey:@"loseID"];
    continueID = [coder decodeIntForKey:@"continueID"];
    activeSideIndex = [coder decodeIntForKey:@"activeSideIndex"];
    activeSide = [[coder decodeObjectForKey:@"activeSide"] retain];
    sides = [[coder decodeObjectForKey:@"sides"] retain];
    teams = [[coder decodeObjectForKey:@"teams"] retain];
    victoryConditions = [[coder decodeObjectForKey:@"victoryConditions"] retain];
    defeatConditions = [[coder decodeObjectForKey:@"defeatConditions"] retain];
    details = [[coder decodeObjectForKey:@"details"] retain];
    map = [[coder decodeObjectForKey:@"map"] retain];
	advancedInfo = [coder decodeObjectForKey:@"advancedInfo"];
	if (advancedInfo == nil) advancedInfo = [[NSMutableString alloc] init];
	[advancedInfo retain];
	events = [coder decodeObjectForKey:@"events"];
	if (events == nil) 	events = [[[NSMutableArray alloc] init] retain];
	[events retain];
    return self;
}


-(NSString *)getName
{
	return name;
}

-(void)setName:(NSString *)newName
{
	[name autorelease];
	name = [newName copy];
	[name retain];
}

-(WMLMap *)getMap
{
	return map;
}

-(int)getSidesCount
{
    return [sides count];
}

-(WMLSide *)getSideAtIndex:(int)index
{
    return [sides objectAtIndex:index];
}

-(int)addSide
{
    [sides addObject: [[WMLSide alloc] init]];	// add a blank side
    return ([sides count]-1);
}

-(WMLSide *)getActiveSide
{
    return activeSide;
}

-(void)setActiveSide:(int)sideNo
{
    activeSideIndex = sideNo;
    activeSide = [sides objectAtIndex: activeSideIndex];
}

-(NSMutableArray *)getDefeatConditionsFor:(NSString *)diffLevel
{
    return [defeatConditions objectForKey: diffLevel];
}

-(void)deleteSide:(int)sideToDie
{
	[sides removeObjectAtIndex:sideToDie];
}
-(NSMutableArray *)getVictoryConditionsFor:(NSString *)diffLevel
{
    return [victoryConditions objectForKey: diffLevel];
}

-(WMLScenarioDetails *)getDetailsFor:(NSString *)diffLevel
{
    return [details objectForKey: diffLevel];
}

-(NSMutableArray *)getTeams
{
    return teams;
}


-(void)setScenarioAdvancedInfo: (NSString *)newInfo
{
	[advancedInfo setString:newInfo];
}

-(NSString *)getScenarioAdvancedInfo
{
	return advancedInfo;
}

-(void)exportToFolderWithPrefix:(NSString *)prefix scenarios:(NSMutableArray *)scenarios
{
	[[[WNCampaign getMainCampaign] getCharacters] initInPlay];
    NSString *scenarioId = [WNUtils replaceSpaces: [prefix stringByAppendingString:name]];
    NSString *fileName = [[[WNFileLocations getExportScenariosLocation] 
    stringByAppendingPathComponent: scenarioId]
    stringByAppendingPathExtension:@"cfg"];
    fprintf(stderr, "About to export scenario to:%s\n", [fileName UTF8String]);
    FILE *sfp = fopen([fileName fileSystemRepresentation], "w\0");
    if (sfp == nil) fprintf(stderr, "ERROR - Failed to open for save\n");
    fprintf(sfp, "[scenario]\n");
    fprintf(stderr, "Setting id to %s\n", [scenarioId UTF8String]);
    fprintf(sfp, "id=%s\n", [scenarioId UTF8String]);
    fprintf(sfp, "{DAWN}\n{MORNING}\n{AFTERNOON}\n{DUSK}\n{FIRST_WATCH}\n{SECOND_WATCH}\n\n");
    
    // Next scenario on success
    fprintf(stderr, "... about to export progressions\n");
    WMLScenario *tmpScen = [scenarios objectAtIndex:winID];
    NSString *nextScen = [WNUtils replaceSpaces:[prefix stringByAppendingString:[tmpScen getName]]];
    fprintf(sfp, "next_scenario=%s\n", [nextScen UTF8String]);
    fprintf(stderr, "...about to export map\n");
    NSString *myMap = [map exportToString];
    fprintf(sfp, "map_data=\"%s\"\n", [myMap UTF8String]);
    [myMap autorelease];
    
    // Now do each side
    int sideLoop;
    for (sideLoop = 0; sideLoop<[sides count] ; sideLoop++)
        {
        fprintf(stderr, "About to export side:%d\n", sideLoop);
        [[sides objectAtIndex:sideLoop] exportSideToFile:sfp withTeams:teams withIndex:(sideLoop+1) withMap:map withSide:sideLoop];
        }
	// Now the victory conditions to display
	[self exportObjectivesForDifficulty:@"Easy" forDesc:@"EASY" toFile:sfp prefix:prefix];
	[self exportObjectivesForDifficulty:@"Normal" forDesc:@"NORMAL" toFile: sfp prefix:prefix];
	[self exportObjectivesForDifficulty:@"Hard" forDesc:@"HARD" toFile: sfp prefix:prefix];
	
	// Now export character-specific events
	fprintf(stderr, "Processing character events\n");
	NSString *charEvents = [[[WNCampaign getMainCampaign] getCharacters] exportCharacterEventsFromScenario:self];
	fprintf(sfp, "%s\n", [charEvents UTF8String]);
	fprintf(stderr, "Character events processed\n");
    
	// Now we just dump the advanced info
	fprintf(sfp, "%s\n", [advancedInfo UTF8String]);
	
	// Now we export the specific events
	fprintf(stderr, "Processing scenario events\n");
	NSMutableString *scenEvents = [[NSMutableString alloc] init];
	int eventLoop;
	for (eventLoop = 0; eventLoop<[events count] ;eventLoop++) [[events objectAtIndex:eventLoop] exportEventToString:scenEvents];
	fprintf(sfp, [scenEvents UTF8String]);
	fprintf(sfp, "\n");
			
	// Done so close and return
    fprintf(sfp, "[/scenario]\n");
    fclose(sfp);
	[[[WNCampaign getMainCampaign] getCharacters] clearInPlay];
}

-(void)exportObjectivesForDifficulty:(NSString *)diff forDesc:(NSString *)diffDesc toFile:(FILE *)fp prefix:(NSString *)prefix
{
	NSMutableArray *myVictory = [victoryConditions objectForKey:diff];
	NSMutableArray *myDefeat = [defeatConditions objectForKey:diff];
	WMLScenarioDetails *myDetail = [details objectForKey:diff];
	WMLScenario *nextScen;
	
	fprintf(fp, "#ifdef %s\n", [diffDesc UTF8String]);
	fprintf(fp, "turns=%d\n", [myDetail getNoTurns]);
	fprintf(fp, "objectives=\"\n");
	int vtmp;
	fprintf(fp, "Victory:\n");
	for (vtmp=0; vtmp<[myVictory count] ;vtmp++)
		fprintf(fp, "@%s\n", [[myVictory objectAtIndex:vtmp] UTF8String]);
	fprintf(fp, "Defeat:\n");
	for (vtmp=0; vtmp<[myDefeat count] ;vtmp++)
		fprintf(fp, "#%s\n", [[myDefeat objectAtIndex:vtmp] UTF8String]);
	
	fprintf(fp, "\"\n");
	if ([myDetail getVictoryScenarioIndex]!=-1)
		{
		nextScen = [[WNCampaign getMainCampaign] getScenarioAtIndex: [myDetail getVictoryScenarioIndex]];
		fprintf(fp, "next_scenario=%s\n",[[WNUtils replaceSpaces:[prefix stringByAppendingString:[nextScen getName]]] UTF8String]);
		}
	fprintf(fp, "#endif\n\n");
}

-(NSMutableDictionary *)getNextScenarioText
{
	fprintf(stderr, "Setting next scenario text\n");
	NSMutableString *vText = [[[NSMutableString alloc] init] retain];
	NSMutableString *cText = [[[NSMutableString alloc] init] retain];
	NSMutableString *prefix = [[WNCampaign getMainCampaign] getPrefix];
	
	[self addNextScenarioInfoForDifficulty:@"Easy" toVictoryString:vText toContinueString:cText withPrefix:prefix];
	[self addNextScenarioInfoForDifficulty:@"Normal" toVictoryString:vText toContinueString:cText withPrefix:prefix];
	[self addNextScenarioInfoForDifficulty:@"Hard" toVictoryString:vText toContinueString:cText withPrefix:prefix];
	
	NSMutableDictionary *nextDict = [[NSMutableDictionary alloc] init];
	[nextDict setObject:vText forKey:@"Victory"];
	[nextDict setObject:cText forKey:@"Continue"];
	fprintf(stderr, "Next scenario text set:\n%s\n\n%s\n\n", [vText cString], [cText cString]);
	return nextDict;
}

-(void)addNextScenarioInfoForDifficulty:(NSString *)diff toVictoryString:(NSMutableString *)vInfo toContinueString:(NSMutableString *)cInfo withPrefix:(NSString *)prefix
{
	WMLScenarioDetails *myDetail = [details objectForKey:diff];
	WMLScenario *nextScen;
	
	if ([myDetail getVictoryScenarioIndex]!=-1)
		{
		[vInfo appendString:@"#ifdef "];
		[vInfo appendString:[diff uppercaseString]];
		[vInfo appendString:@"\nnext_scenario="];
		nextScen = [[WNCampaign getMainCampaign] getScenarioAtIndex: [myDetail getVictoryScenarioIndex]];
		[vInfo appendString:[WNUtils replaceSpaces:[prefix stringByAppendingString:[nextScen getName]]]];
		[vInfo appendString:@"\n#endif\n\n"];
		}
	if ([myDetail getContinueScenarioIndex]!=-1)
		{
		[cInfo appendString:@"#ifdef "];
		[cInfo appendString:[diff uppercaseString]];
		[cInfo appendString:@"\nnext_scenario="];
		nextScen = [[WNCampaign getMainCampaign] getScenarioAtIndex: [myDetail getVictoryScenarioIndex]];
		[cInfo appendString:[WNUtils replaceSpaces:[prefix stringByAppendingString:[nextScen getName]]]];
		[cInfo appendString:@"\n#endif\n\n"];
		}
}

-(void)checkDeletedScenario:(id)index
{
	int deadScenario = [index intValue];
	
	if (winID==deadScenario) winID = -1;
	if (winID>deadScenario) winID--;
	
	if (loseID==deadScenario) loseID = -1;
	if (loseID>deadScenario) loseID--;
	
	if (continueID==deadScenario) continueID=-1;
	if (continueID>deadScenario) continueID--;
}

-(NSMutableArray *)getEvents
{
	return events;
}

-(WMLEvent *)getEventAtIndex:(int)index
{
	return [events objectAtIndex:index];
}

-(int)noEvents
{
	return [events count];
}

-(void)addNewEvent
{
	fprintf(stderr, "Adding event...\n");
	[events addObject:[[[WMLEvent alloc] initAsTag:YES withName:@"[Event]" withStringValue:@"" withIntValue:0 withType:0 withParent:nil] retain]];
	fprintf(stderr, "Event Count:%d\n", [events count]);
}
@end
