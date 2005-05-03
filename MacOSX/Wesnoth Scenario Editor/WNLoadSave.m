#import "Wesnoth_Scenario_Editor_Prefix.h"
#import "WNLoadSave.h"
#import "WNCampaign.h"
#import "WNFileLocations.h"

@implementation WNLoadSave

- (IBAction)export:(id)sender
{
    NSSavePanel *saveDialog = [NSSavePanel savePanel];
    int saveResult = [saveDialog runModalForDirectory:NSHomeDirectory() file:@""];
    if (saveResult == NSOKButton)
		{
		NSWindow *tmpSaveWin = [self startSaveWindow];
        [[WNCampaign getMainCampaign] exportToFolder:[saveDialog filename] withDialog:tmpSaveWin andIndicator: saveProgress];
		[self stopSaveWindow];
		}
}

- (IBAction)load:(id)sender
{
    NSSavePanel *openDialog = [NSOpenPanel openPanel];
    int openResult = [openDialog runModalForDirectory:NSHomeDirectory() file:@""];
    if (openResult == NSOKButton) [self doLoadFile:[openDialog filename]];
}

- (void)doLoadFile:(NSString *)fileToLoad
{
	NSWindow *loadWin = [WNCampaign windowForTitle:@"Loading Data"];
	[loadWin center];
	[loadWin makeKeyAndOrderFront: self];
	[loadProgress setUsesThreadedAnimation:YES];
	[loadProgress startAnimation: self];
	//[NSApp runModalForWindow: loadWin];
	[WNCampaign setMainCampaign: [NSKeyedUnarchiver unarchiveObjectWithFile:fileToLoad]];
	fprintf(stderr, "Unarchive compelte\n");
	//[NSApp stopModal];
	[loadWin orderOut: self];
	[loadProgress stopAnimation: self];
	[WNFileLocations setSaveLocation: fileToLoad];
    [[NSNotificationCenter defaultCenter] postNotificationName:@"campaignReset" object:self];
	[[WNPreferences getDocControl] noteNewRecentDocumentURL:[[NSURL alloc] initFileURLWithPath:fileToLoad]];
	fprintf(stderr, "FileLoad complete\n");
}

- (IBAction)revert:(id)sender
{
	NSString *lastFile = [WNFileLocations getSaveLocation];
	if (lastFile != nil)
		{
		[self doLoadFile: lastFile];
		}
	
}

- (IBAction)save:(id)sender
{
	if ([[WNFileLocations getSaveLocation] isEqualTo:@""])
		{
		[self saveAs: self];
		}else{
		NSWindow *saveWin = [WNCampaign windowForTitle:@"Saving Data"];
		[saveWin center];
		[saveWin makeKeyAndOrderFront: self];
		[saveProgress setUsesThreadedAnimation:YES];
		[saveProgress startAnimation: self];
		[NSKeyedArchiver archiveRootObject:[WNCampaign getMainCampaign] toFile:[WNFileLocations getSaveLocation]];
		[saveWin orderOut: self];
		[saveProgress stopAnimation: self];
		}
}

- (IBAction)saveAs:(id)sender
{
    NSSavePanel *saveDialog = [NSSavePanel savePanel];
    int saveResult = [saveDialog runModalForDirectory:NSHomeDirectory() file:@""];
    if (saveResult == NSOKButton)
		{
		NSWindow *saveWin = [WNCampaign windowForTitle:@"Saving Data"];
		[saveWin center];
		[saveWin makeKeyAndOrderFront: self];
		[saveProgress setUsesThreadedAnimation:YES];
		[saveProgress startAnimation: self];
        [NSKeyedArchiver archiveRootObject:[WNCampaign getMainCampaign] toFile:[saveDialog filename]];
		[WNFileLocations setSaveLocation: [saveDialog filename]];
		[saveWin orderOut: self];
		[saveProgress stopAnimation: self];
		[[WNPreferences getDocControl] noteNewRecentDocumentURL:[[NSURL alloc] initFileURLWithPath:[saveDialog filename]]];
		}
}

-(NSWindow *)startSaveWindow
{
		NSWindow *saveWin = [WNCampaign windowForTitle:@"Saving Data"];
		[saveProgress setIndeterminate: YES];
		[saveWin center];
		[saveWin makeKeyAndOrderFront: self];
		[saveProgress setUsesThreadedAnimation:YES];
		[saveProgress startAnimation: self];
		return saveWin;
}

-(void)stopSaveWindow
{
		[[WNCampaign windowForTitle:@"Saving Data"] orderOut: self];
		[saveProgress stopAnimation: self];
}

@end
