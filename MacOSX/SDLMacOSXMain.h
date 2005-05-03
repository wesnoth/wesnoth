/*   SDLMain.m - main entry point for our Cocoa-ized SDL app
       Initial Version: Darrell Walisser <dwaliss1@purdue.edu>
       Non-NIB-Code & other changes: Max Horn <max@quendi.de>

    Feel free to customize this file to suit your needs
*/

#import <Cocoa/Cocoa.h>

@interface SDLMain : NSObject
- (IBAction)prefs: (id)sender;
- (void)findWindows;
- (IBAction)installUserCampaigns: (id)sender;
- (IBAction)installUserCampaignsSkip: (id)sender;
- (IBAction)installUserOverwrite: (id)sender;
- (void)installEachDirFrom: (NSString *)FromDir to:(NSString *)ToDir overwrite:(BOOL)ow;
- (void)ProcessMyOpenFile: (NSString *)FileToProcess mode:(int)mode overwrite:(BOOL)owMode;
- (IBAction)revealMapsFolder: (id)sender;
- (IBAction)revealDataFolder: (id)sender;
- (IBAction)revealImagesFolder: (id)sender;
- (IBAction)revealFontsFolder: (id)sender;
- (IBAction)revealSoundsFolder: (id)sender;
- (IBAction)revealMusicFolder: (id)sender;
- (IBAction)revealPrefsFolder: (id)sender;
- (IBAction)revealCampaignsFolder: (id)sender;
- (IBAction)revealSaveFolder: (id)sender;
- (IBAction)launchConsole: (id)sender;
- (IBAction)launchEditor: (id)sender;
- (IBAction)webForum: (id)sender;
- (IBAction)webDownloads: (id)sender;
- (IBAction)webMainSite: (id)sender;
@end

@interface InstallingDataWindow : NSWindowController
{
}
@end