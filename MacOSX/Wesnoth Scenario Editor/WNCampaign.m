#import "Wesnoth_Scenario_Editor_Prefix.h"
#import "WNCampaign.h"
#import "WMLCampaign.h"
#import "WNPreferences.h"
#import "WNTerrains.h"
#import "WNUnits.h"

#import "SelectionUtils.h"

@implementation WNCampaign
	WMLCampaign *MainCampaign;
	BOOL mainCampaignInitialised;
	int selectedScenario;
	float mapZoomLevel=1.0;
	NSMutableString *difficulty;
	NSMutableDictionary *windowDict;
	WMLTag *eventTags;
	NSMutableDictionary *eventTagsDict;
	NSMenu *mainMenu=nil;
	NSMenu *editorMenu=nil;
	int initStatus=0;
	NSWindow *editorWindow;
	
- (IBAction)aggressionSelect:(id)sender
{
    [[self getActiveSideDetail]  setAgression:[sender floatValue]];
}

- (IBAction)campaignIDSet:(id)sender
{
}

- (IBAction)campaignNameSet:(id)sender
{
	[MainCampaign setName: [sender stringValue]];
}

- (IBAction)campaignPrefixSet:(id)sender
{
	[MainCampaign setPrefix: [sender stringValue]];
}

- (IBAction)canRecruit:(id)sender
{
    int state = [sender state];
    WMLSideDetail *myDetail;
    
    myDetail = [self getActiveSideDetail];
    if (state == NSOnState)
        {
        [myDetail setCanRecruit:YES];
        }else{
        [myDetail setCanRecruit:NO];
        }
}

- (IBAction)defeatAdd:(id)sender
{
    [[[MainCampaign getActiveScenario] getDefeatConditionsFor: [WNCampaign getDifficulty]] addObject: [[NSMutableString alloc] initWithString:@"Another Defeat Condition"]];
    [defeatList reloadData];
}

- (IBAction)defeatRemove:(id)sender
{
    int itemToDelete = [defeatList selectedRow];
    if (itemToDelete != -1)
        [[[MainCampaign getActiveScenario] getDefeatConditionsFor: [WNCampaign getDifficulty]] removeObjectAtIndex: itemToDelete];
    [defeatList reloadData];
}

- (IBAction)defeatSelect:(id)sender
{
}

- (IBAction)difficultyDescChange:(id)sender
{
	int tag = [sender tag]; // Use to ID which textfield sent the request
	switch(tag)
		{
		case 1:
			[MainCampaign setDifficultyDescription:[difficultyDescEasy stringValue] forDifficulty:@"Easy"];
			break;
		case 2:
			[MainCampaign setDifficultyDescription:[difficultyDescNormal stringValue] forDifficulty:@"Normal"];
			break;
		case 3:
			[MainCampaign setDifficultyDescription:[difficultyDescHard stringValue] forDifficulty:@"Hard"];
			break;
		}
}

- (IBAction)difficultyEasyCheck:(id)sender
{
}

- (IBAction)difficultyHardCheck:(id)sender
{
}

- (IBAction)difficultyMediumCheck:(id)sender
{
}

- (IBAction)doNothing:(id)sender	// Does nothing, just for connections :D
{	
}

- (IBAction)goldSet:(id)sender
{
    [[self getActiveSideDetail] setGold:[sender intValue]];
}

- (IBAction)keepCancel:(id)sender
{
}

- (IBAction)keepChoose:(id)sender
{
}

- (IBAction)leaderSelect:(id)sender
{
    fprintf(stderr, "At leader select\n");
    int leaderID = [sender selectedRow];
    
    fprintf(stderr, "Setting leader type to: %d\n", leaderID); 
    [[[MainCampaign getActiveSide] detailForKey: [sidesShowDetailForDifficulty titleOfSelectedItem]] setLeaderTypeByID: leaderID];
	[self populateSidesData];
}

- (IBAction)mapCharacterSelect:(id)sender
{
    [WNPreferences setCharacterID: [sender selectedRow]];
}

- (IBAction)mapExport:(id)sender
{
    NSSavePanel *saveDialog = [NSSavePanel savePanel];
    int saveResult = [saveDialog runModalForDirectory:NSHomeDirectory() file:@""];
    if (saveResult == NSOKButton) [[MainCampaign getActiveMap] exportToFile: [saveDialog filename]];
}

- (IBAction)mapHeightSet:(id)sender
{
	fprintf(stderr, "Changing map height\n");
	[[MainCampaign getActiveMap] setHeight:[sender intValue]];
}

- (IBAction)mapImport:(id)sender
{
    NSPoint tmpPoint;
    int x,y;
    NSSavePanel *openDialog = [NSOpenPanel openPanel];
    int openResult = [openDialog runModalForDirectory:NSHomeDirectory() file:@""];
    if (openResult == NSOKButton) tmpPoint = [[MainCampaign getActiveMap] importFromFile: [openDialog filename]];
    x = tmpPoint.x;
    y = tmpPoint.y;
    [mapWidth setIntValue:x];
    [mapHeight setIntValue:y];
}

- (IBAction)mapUnitSide:(id)sender
{
	[WNPreferences setUnitSide:[mapUnitSideMenu indexOfSelectedItem]];
}

- (IBAction)mapViewSliderSet:(id)sender
{
	[WNPreferences setUnitOpacity:[sender floatValue]];
}

- (IBAction)mapWidthSet:(id)sender
{
	[[MainCampaign getActiveMap] setWidth:[sender intValue]];
}

- (IBAction)mapZoomSet:(id)sender
{
	mapZoomLevel = [sender floatValue];
	[HexUtils initWithWidth:(70*mapZoomLevel)];
	NSSize nzSize;
	nzSize.width = (70*mapZoomLevel);
	nzSize.height = (70*mapZoomLevel);
	[WNTerrains resizeTo:nzSize];
        [WNUnits resizeTo:nzSize];
	[[NSNotificationCenter defaultCenter] postNotificationName:@"mapChanged" object:self];
}

- (IBAction)nextScenarioVictorySelect:(id)sender
{
    [[WNCampaign getActiveScenarioDetail] setVictoryScenarioIndex:[nextScenarioVictory selectedRow]];
}

- (IBAction)nextScenarioContinueSelect:(id)sender
{
    [[WNCampaign getActiveScenarioDetail] setContinueScenarioIndex:[nextScenarioContinue selectedRow]];
}

- (IBAction)noTurnsSet:(id)sender;
{
    [[WNCampaign getActiveScenarioDetail] setNoTurns:[sender intValue]];
}

- (IBAction)recruitSelect:(id)sender
{
	WMLSideDetail *myDetail = [self getActiveSideDetail];
	[myDetail setRecruitTypes: [SelectionUtils selectionsFromView:sender]]; 
}

- (IBAction)scenarioDelete:(id)sender
{
	 if ([MainCampaign count]>1)
		{
		[MainCampaign deleteScenario: [scenarioList selectedRow]];
		[self scenarioSelect:0];
		[scenarioList reloadData];
		}
}

- (IBAction)scenarioNew:(id)sender
{
	int scenIndex = [MainCampaign addScenario];
	[scenarioList reloadData];
	[self selectScenario: scenIndex];
}

- (IBAction)scenarioSelect:(id)sender
{
    [self selectScenario: [sender selectedRow]];
}

- (IBAction)showConditionsForDifficulty:(id)sender
{
    [difficulty setString: [showConditionsForDifficulty titleOfSelectedItem]];
    [self populateSidesData];
    [self populateScenarioInfo];
    [[NSNotificationCenter defaultCenter] postNotificationName:@"scenarioInfoRefresh" object:self];
}

- (IBAction)sidesAdd:(id)sender
{
    WMLScenario *myScenario = [MainCampaign getActiveScenario];
    int sideIndex = [myScenario addSide];
    [sidesList reloadData];
    [self selectSide: sideIndex];
}

-(void)sidesRefresh
{
    int tmpIndex = [mapUnitSideMenu indexOfSelectedItem];
    WMLScenario *myScenario = [MainCampaign getActiveScenario];
    [mapUnitSideMenu removeAllItems];
    int i;
    for (i=0; i<[myScenario getSidesCount] ;i++) 
        [mapUnitSideMenu addItemWithTitle:[[myScenario getSideAtIndex:i] getName]];
    [mapUnitSideMenu selectItemAtIndex:tmpIndex];
    
}

- (IBAction)sidesRemove:(id)sender
{

    WMLScenario *myScenario = [MainCampaign getActiveScenario];
	WMLMap *myMap = [myScenario getMap];
	int curSide = [sidesList selectedRow];
	fprintf(stderr, "Removing side:%d\n", curSide);
	NSMutableDictionary *delDict = [[NSMutableDictionary alloc] init];
	[delDict setObject:[[NSNumber alloc] initWithInt:1] forKey:@"mode"];
	[delDict setObject:[[NSNumber alloc] initWithInt:curSide] forKey:@"side"];
	[delDict setObject:[[NSNumber alloc] initWithInt:0] forKey:@"count"];
	fprintf(stderr, "First iteration, to check\n");
	[myMap iterateSelector:@selector(deleteSideWithDict:) withObject:delDict];
	NSNumber *count = [delDict objectForKey:@"count"];
	fprintf(stderr, "First iteration complete, occurrances found:%d\n", [count intValue]);
	if ([count intValue]>0)
		{
		int choice = NSRunAlertPanel(@"Warning", @"Found %i units on maps", @"cancel", @"delete anyway", nil, [count intValue]);
		fprintf(stderr, "Response to delete:%d\n", choice);
		if (choice == NSAlertDefaultReturn) return;
		}
	[delDict setObject:[[NSNumber alloc] initWithInt:2] forKey:@"mode"];
	[myMap iterateSelector:@selector(deleteSideWithDict:) withObject:delDict];
	[myScenario deleteSide:curSide];
	[self selectSide:0];
	[sidesList reloadData];
}

- (IBAction)sidesShowDetailFor:(id)sender
{
    [difficulty setString: [sidesShowDetailForDifficulty titleOfSelectedItem]];
    [showConditionsForDifficulty selectItemWithTitle: difficulty];
    [self populateSidesData];
    [self populateScenarioInfo];
}

- (void)selectSide: (int)index
{
    WMLScenario *myScenario = [MainCampaign getActiveScenario];
    
    fprintf(stderr, "Changing selected side to no:%d\n", index);

    [sidesList selectRow: index byExtendingSelection:NO];
    [myScenario setActiveSide: index];
    [self populateSidesData];
}

-(void)sidesChanged:(NSNotification *)notification
{
    [self sidesRefresh];
}

- (IBAction)sidesSelect:(id)sender
{
    int newSide = [sidesList selectedRow];
    fprintf(stderr, "Side %d selected\n", newSide);
    [[MainCampaign getActiveScenario] setActiveSide:newSide];
    [self populateSidesData];
}

- (IBAction)teamAdd:(id)sender
{
    [[[WNCampaign getActiveScenario] getTeams] addObject:@"New Team"];
    [teamList reloadData];
}

- (IBAction)teamRemove:(id)sender
{
}

- (IBAction)teamSelect:(id)sender
{
    [[self getActiveSideDetail] setSideDetailTeam: [sender selectedRow]];
}

- (void)populateScenarioInfo
{
    [showConditionsForDifficulty selectItemWithTitle: difficulty];
    [victoryList reloadData];
    [defeatList reloadData];
    
    WMLScenarioDetails *myDetails = [WNCampaign getActiveScenarioDetail];
    [noTurns setIntValue: [myDetails getNoTurns]];
    [nextScenarioVictory deselectAll: self];
    [nextScenarioContinue deselectAll: self];
    [nextScenarioDefeat deselectAll: self];
    [nextScenarioVictory selectRow: [myDetails getVictoryScenarioIndex] byExtendingSelection:NO];
    [nextScenarioContinue selectRow: [myDetails getContinueScenarioIndex] byExtendingSelection:NO];
}

- (void)populateSidesData
{
    WMLScenario *myScenario = [MainCampaign getActiveScenario];
    WMLSide *mySide = [myScenario getActiveSide];
    fprintf(stderr,"Populating side data for difficulty: %s\n", [difficulty UTF8String]); 
    [sidesShowDetailForDifficulty selectItemWithTitle: difficulty];
    WMLSideDetail *myDetail = [mySide detailForKey: difficulty];    
    [leaderList selectRow: [myDetail getLeaderID] byExtendingSelection:NO];
    [goldAmount setIntValue: [[self getActiveSideDetail] getGold]];
    NSString *type = [myDetail getType];
    if ([type isEqualTo:@"AI"])
        {
        [typeAI setState:NSOnState];
        [typeHuman setState:NSOffState];
        }else if ([type isEqualTo:@"Human"]){
        [typeAI setState:NSOffState];
        [typeHuman setState:NSOnState];
        }
    [agressionLevel setFloatValue:[myDetail getAgression]];
    if ([myDetail getCanRecruit] == YES)
        {
        [canRecruit setState:NSOnState];
        }else{
        [canRecruit setState:NSOffState];
        }
    [SelectionUtils selectionsToView:recruitList indexArray:[myDetail getRecruitTypes]];
    [teamList selectRow:[myDetail getSideDetailTeam] byExtendingSelection:NO];
	[sidesAdvancedInfo setString:[mySide getSideAdvancedInfo]];
    [self sidesRefresh];
}

- (IBAction)terrainSelect:(id)sender
{
	[WNPreferences setTerrainID: [sender selectedRow]];
	[WNPreferences setTerrain: [WNTerrains terrainAtIndex:[sender selectedRow]]];
}

- (IBAction)typeAI:(id)sender
{
    [[self getActiveSideDetail] setType:@"AI"];
}

- (IBAction)typeHuman:(id)sender
{
    [[self getActiveSideDetail] setType:@"Human"];
}

- (IBAction)unitPlacementSetEasy:(id)sender
{
    [WNPreferences setPlaceEasy:([sender state]==NSOnState)];
}

- (IBAction)unitPlacementSetNormal:(id)sender
{
    [WNPreferences setPlaceNormal:([sender state]==NSOnState)];
}

- (IBAction)unitPlacementSetHard:(id)sender
{
    [WNPreferences setPlaceHard:([sender state]==NSOnState)];
}

- (IBAction)unitSelect:(id)sender
{
    [WNPreferences setUnitID:[sender selectedRow]];
}

- (IBAction)victoryAdd:(id)sender
{
    [[[MainCampaign getActiveScenario] getVictoryConditionsFor: [WNCampaign getDifficulty]] addObject: [[NSMutableString alloc] initWithString:@"Another Victory Condition"]];
    [victoryList reloadData];

}

- (IBAction)victoryRemove:(id)sender
{
    int itemToDelete = [victoryList selectedRow];
    if (itemToDelete != -1)
        [[[MainCampaign getActiveScenario] getVictoryConditionsFor: [WNCampaign getDifficulty]] removeObjectAtIndex: itemToDelete];
    [victoryList reloadData];

}

- (IBAction)victorySelect:(id)sender
{
}

-(void)campaignReset:(NSNotification *)notification
{
    fprintf(stderr, "Campaign refresh...");
    [scenarioList reloadData];
    [self selectScenario:0];
    [[MainCampaign getActiveScenario] setActiveSide:0];
    [self populateSidesData];
    [self populateScenarioInfo];
    [mainTabView selectFirstTabViewItem: self];
    [self sidesRefresh];
    [campaignName setStringValue: [MainCampaign getName]];
    [campaignPrefix setStringValue: [MainCampaign getPrefix]];
    [campaignIcon setImage: [MainCampaign getIcon]];
    [difficultyIconEasy setImage: [MainCampaign getDifficultyIconForDifficulty:@"Easy"]];
    [difficultyIconNormal setImage: [MainCampaign getDifficultyIconForDifficulty:@"Normal"]];
    [difficultyIconHard setImage: [MainCampaign getDifficultyIconForDifficulty:@"Hard"]];
    [difficultyDescEasy setStringValue:[MainCampaign getDifficultyDescriptionForDifficulty:@"Easy"]];
    [difficultyDescNormal setStringValue:[MainCampaign getDifficultyDescriptionForDifficulty:@"Normal"]];
    [difficultyDescHard setStringValue:[MainCampaign getDifficultyDescriptionForDifficulty:@"Hard"]];
    [[NSNotificationCenter defaultCenter] postNotificationName:@"characterListsReset" object:self];
}


-(void)awakeFromNib
{   // OK, time to set up some basic info
	#ifdef embeddedEditor
	if (initStatus==0)
		{
		NSLog(@"Launching doInit from awakeFromNib");
		[self doInit];
		}
	#endif
}

-(void)applicationWillFinishLaunching: (NSNotification *)notification
{
	#ifndef embeddedEditor
	if (initStatus==0)
		{
		NSLog(@"Launching doInit from applicationWillFinishLaunching");
		[self doInit];
		}
	#endif
}

-(void)doInit
{   // Moved from awakeFromNib
	NSLog (@"At doInit *****************");
	fprintf(stderr, "WNCampaign - Creating windowDict\n");
	windowDict = [[NSMutableDictionary alloc] init];
	[windowDict retain];
	NSArray *windowList = [NSApp windows];
	int i;
	for (i=0; i< [windowList count] ;i++) [windowDict setObject:[windowList objectAtIndex: i] forKey:[[windowList objectAtIndex:i] title]];

	NSWindow *loadWin = [WNCampaign windowForTitle:@"Initialising Reference Data"];
	[loadWin center];
	[loadWin makeKeyAndOrderFront: self];
    [WNTerrains init];
    [WNUnits init];
	fprintf(stderr, "Initialising Campaign class\n");
	if ([WNTerrains initialised] == NO) [WNTerrains init];
	MainCampaign = [[WMLCampaign alloc] init];
	[MainCampaign retain];
	fprintf(stderr, "MainCampaign created\n");
	selectedScenario = 0;
	mainCampaignInitialised = YES;
	difficulty = [[NSMutableString alloc] initWithString:@"Easy"];
	[difficulty retain];
	NSLog(@"Initialiasing lists");
	[scenarioList reloadData];
	[scenarioList selectRow: 0 byExtendingSelection:NO];
	[self selectScenario:0];
	[sidesList reloadData];
	[sidesList selectRow: 0 byExtendingSelection:NO];
	[self selectSide:0];
	fprintf(stderr, "WN Campaign - Notifications\n");
	[[NSNotificationCenter defaultCenter] postNotificationName:@"mapChanged" object:self];
	[[NSNotificationCenter defaultCenter] addObserver:self
		selector:@selector(sidesChanged:)
		name:@"sidesChanged" object:nil];
	[[NSNotificationCenter defaultCenter] addObserver:self
		selector:@selector(campaignReset:)
		name:@"campaignReset" object:nil];
	[[NSNotificationCenter defaultCenter] addObserver:self
		selector:@selector(newCampaignIcon:)
		name:@"newCampaignIcon" object:nil];

	NSLog(@"Campaign class initialised\n");
	NSLog(@"Setting Prefs");
	[WNPreferences setPlaceEasy:YES];
	[WNPreferences setPlaceNormal:YES];
	[WNPreferences setPlaceHard:YES];
	[WNPreferences setDocControl:[[NSDocumentController sharedDocumentController] retain]];
	NSLog(@"Prefs Set");
	
	NSLog(@"Resetting Lists");
	// Now reset all lists
	[leaderList reloadData];
	[recruitList reloadData];
	[unitList reloadData];
	[terrainList reloadData];
	NSLog(@"Lists Reset");
	
	// Now send the mapView its info
	NSLog(@"Setting and Sending mapView Info");
	NSMutableDictionary *mapDict = [[[NSMutableDictionary alloc] init] retain];
	[mapDict setObject:mapInfoTerrain forKey:@"Terrain"];
	[mapDict setObject:mapInfoUnitList forKey:@"Unit"];
	[mapDict setObject:mapInfoCharacterList forKey:@"Character"];
	[[NSNotificationCenter defaultCenter] postNotificationName:@"mapInfo" object:self userInfo:mapDict];
	NSLog(@"mapView Info Sent");
	
	[[NSNotificationCenter defaultCenter] postNotificationName:@"campaignReset" object:self];
	NSString *eventConfigFile = [[[NSBundle mainBundle] resourcePath] stringByAppendingPathComponent: @"eventConfig.info"];
	
	NSLog(@"Setting Event Tags");
	eventTags = [WMLTag alloc];
	[eventTags initWithFile:eventConfigFile  setTag:@"[EventActions]"];
	[eventTags retain];
	eventTagsDict = [[[NSMutableDictionary alloc] init] retain];
	int tagLoop;
	WMLTag *tmpTag;
	for (tagLoop = 0; tagLoop<[eventTags childrenCount] ;tagLoop++)
		{
		tmpTag = [eventTags getChildAtIndex:tagLoop];
		[eventTagsDict setObject:tmpTag forKey:[tmpTag getTag]];
		}
		
	NSLog(@"Posting finishedLaunching notification");
	[[NSNotificationCenter defaultCenter] postNotificationName:@"finishedLaunching" object:self];
	[loadWin orderOut: self];
	initStatus=1;
	editorWindow = [WNCampaign windowForTitle:@"Scenario Designer"];
	NSLog(@"doInit completed");
}

- (BOOL)application:(NSApplication *)theApplication openFile:(NSString *)filename
{
	fprintf(stderr, "Opening file...");
	[loadSave doLoadFile:filename];
	return YES;
}

+(WMLCampaign *)getMainCampaign
{
	if (mainCampaignInitialised==YES)
		{
		return MainCampaign;
		}
	return nil;
}

+(void)setMainCampaign:(WMLCampaign *)newCampaign
{
    [MainCampaign autorelease];
    MainCampaign = newCampaign;
    [MainCampaign retain];
	fprintf(stderr, "MainCampaign set\n");
}

-(void)selectScenario: (int) scenarioIndex
{
	WMLScenario *tmpScenario=nil;
	WMLMap *tmpMap;
	fprintf(stderr, "Activating Scenario:%d\n", scenarioIndex);
	[scenarioList selectRow: scenarioIndex byExtendingSelection:NO];
	selectedScenario = scenarioIndex;
	[MainCampaign setActiveScenario: scenarioIndex];
	[[NSNotificationCenter defaultCenter] postNotificationName:@"mapChanged" object:self];
	
	tmpScenario = [MainCampaign getScenarioAtIndex: scenarioIndex];
	tmpMap = [tmpScenario getMap];
	[mapWidth setIntValue:[tmpMap getWidth]];
	[mapHeight setIntValue:[tmpMap getHeight]];
	[self populateSidesData];
	[self populateScenarioInfo];
	[scenarioAdvancedInfo setString: [tmpScenario getScenarioAdvancedInfo]];
	[[NSNotificationCenter defaultCenter] postNotificationName:@"eventListReset" object:self];
	

}

+(WMLScenario *)getActiveScenario
{
	return [MainCampaign getActiveScenario];
}

+(WMLMap *)getActiveMap
{
	return [MainCampaign getActiveMap];
}

-(WMLSideDetail *)getActiveSideDetail
{
    return [[MainCampaign getActiveSide] detailForKey:[sidesShowDetailForDifficulty titleOfSelectedItem]];
}

+(float)getMapZoom
{
	return mapZoomLevel;
}

+(NSString *)getDifficulty
{
    return difficulty;
}

+(WMLScenarioDetails *)getActiveScenarioDetail
{
    return [[MainCampaign getActiveScenario] getDetailsFor:difficulty];
}

+(NSWindow *)windowForTitle:(NSString *)titleToFind
{
    return [windowDict objectForKey:titleToFind];
}

-(void)newCampaignIcon:(NSNotification *)notification
{
	fprintf(stderr,"At newCampaignIcon\n");
    int tag = [[notification object] tag];
    NSDictionary *tmpDict = [notification userInfo];
	NSImage *newImage = [tmpDict objectForKey:@"image"];
    // Process
	switch(tag)
		{
		case 0: // Main Campaign Icon
			[MainCampaign setIcon: newImage];
			break;
		case 1: // Easy Icon
			[MainCampaign setDifficultyIcon: newImage forDifficulty:@"Easy"];
			break;
		case 2: // Normal Icon
			[MainCampaign setDifficultyIcon: newImage forDifficulty:@"Normal"];
			break;
		case 3: // Hard Icon
			[MainCampaign setDifficultyIcon: newImage forDifficulty:@"Hard"];
			break;
		}
    [tmpDict autorelease];
	NSNotification *fred = [[NSNotification alloc] init];
	[self campaignReset: fred];
}

-(void)textDidChange:(NSNotification *)notification
{
	//fprintf(stderr,"At textDidChange\n");
	NSTextView *fromObj = [notification object];
	if (fromObj == scenarioAdvancedInfo)
		[[MainCampaign getActiveScenario] setScenarioAdvancedInfo:[scenarioAdvancedInfo string]];
	if (fromObj == sidesAdvancedInfo)
		[[[MainCampaign getActiveScenario] getActiveSide] setSideAdvancedInfo:[sidesAdvancedInfo string]];
}

+(WMLTag *)getEventTags
{
	return eventTags;
}

+(WMLTag *)getEventTagForTag:(NSString *)tag
{
	return [eventTagsDict objectForKey:tag];
}

+(void)setMainMenu:(NSMenu *)newMainMenu
{
	mainMenu = newMainMenu;
	[mainMenu retain];
}

+(NSMenu *)getMainMenu
{
	return mainMenu;
}

+(void)setEditorMenu:(NSMenu *)newEditorMenu
{
	editorMenu = newEditorMenu;
	[editorMenu retain];
}

+(NSMenu *)getEditorMenu
{
	return editorMenu;
}

+(void)switchToMainMenu
{
	if (editorMenu != nil)
		{
		[[NSApplication sharedApplication] setMainMenu:mainMenu];
		}
}

+(void)switchToEditorMenu;
{
	if (mainMenu != nil)
		{
		[[NSApplication sharedApplication] setMainMenu:editorMenu];
		}
}

-(void)showEditorWindow
{
	NSLog(@"Making Editor window key");
	fprintf(stderr, "Editor window:%d\n", editorWindow);
	[editorWindow makeKeyAndOrderFront:self];
	[editorWindow makeKeyWindow];
	[editorWindow makeMainWindow];
}

-(NSWindow *)getEditorWindow
{
	return editorWindow;
}

@end
