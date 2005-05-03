/*   SDLMain.m - main entry point for our Cocoa-ized SDL app
       Initial Version: Darrell Walisser <dwaliss1@purdue.edu>
       Non-NIB-Code & other changes: Max Horn <max@quendi.de>

    Feel free to customize this file to suit your needs
*/


//#include <sys/types.h>	
//#include <dirent.h>	
//#include <sys/stat.h>

// MJP 
#include <carbon/carbon.h>
#include "WNCampaign.h";
//
#import "SDL.h"
#import "SDLMacOSXMain.h"
#import <sys/param.h> /* for MAXPATHLEN */
#import <unistd.h>

//#include "display.hpp"
//#include <fstream>
//#include <iostream>


/* Use this flag to determine whether we use SDLMain.nib or not */
#define		SDL_USE_NIB_FILE	1 //Changed from 0 MJP

int EarlierThan10_3=0;

static int    gArgc;
static char  **gArgv;
static BOOL   gFinderLaunch;

char mactextdomain[1024];	// MJP for gettext
NSMutableArray *EditorList=0;	//	Campaign Array

#if SDL_USE_NIB_FILE
/* A helper category for NSString */
@interface NSString (ReplaceSubString)
- (NSString *)stringByReplacingRange:(NSRange)aRange with:(NSString *)aString;
@end
#else
/* An internal Apple class used to setup Apple menus */
@interface NSAppleMenuController:NSObject {}
- (void)controlMenu:(NSMenu *)aMenu;
@end
#endif

@implementation InstallingDataWindow

@end

@interface SDLApplication : NSApplication
@end

@implementation SDLApplication
/* Invoked from the Quit menu item */
- (void)terminate:(id)sender
{
    /* Post a SDL_QUIT event */
    SDL_Event event;
    event.type = SDL_QUIT;
    SDL_PushEvent(&event);
}

@end


/* The main class of the application, the application's delegate */
@implementation SDLMain

NSWindow *UserWindow=nil, *BackUpWindow=nil, *InstallWindow=nil, *DebugWindow=nil;
NSString *ScenariosFolder=nil, *UserInstallCheck=nil, *userPath=nil, *MyOpenFile=nil;
NSFileManager *manager = nil;
int WindowsInstalled = nil;
BOOL OverwriteBackup = NO, OverwriteUser = NO;



// MJP Added Classes
-(void)findWindows
    {
    // Find the windows
    NSArray *WindowList = [NSApp windows];
    int WindowCount = [WindowList count];
    int WindowLoop, WindowID=-1;
    for (WindowLoop = 0; WindowLoop < WindowCount ;WindowLoop++)
        {	// Locate our info window
        if ([[[WindowList objectAtIndex:WindowLoop] title] isEqualToString: @"Installing Data"])
            {
            WindowID = WindowLoop;
            InstallWindow = [WindowList objectAtIndex:WindowLoop];
            }
        if ([[[WindowList objectAtIndex:WindowLoop] title] isEqualToString: @"Install Backup User Campaigns"]) {BackUpWindow = [WindowList objectAtIndex:WindowLoop];}
        if ([[[WindowList objectAtIndex:WindowLoop] title] isEqualToString: @"Install User Campaigns"]) {UserWindow = [WindowList objectAtIndex:WindowLoop];}
        if ([[[WindowList objectAtIndex:WindowLoop] title] isEqualToString: @"Debug Mode"]) {DebugWindow = [WindowList objectAtIndex:WindowLoop];}
        }
    // end find windows
    WindowsInstalled = 1;
    }

-(BOOL)application:(NSApplication *)openApp openFile:(NSString *)fileToOpen
{
    MyOpenFile = [NSString stringWithString: fileToOpen];
    if (WindowsInstalled == nil) {[self findWindows];};
    fprintf(stderr, "drag'n'dropped:%s, MyOpenFile:%s\n\n",[fileToOpen cString],[MyOpenFile cString]); 
    [UserWindow center];
    NSModalSession session = [NSApp beginModalSessionForWindow:UserWindow];
    while([NSApp runModalSession:session] == NSRunContinuesResponse) {}
    [NSApp endModalSession:session];
    return true;
}

-(IBAction)installUserCampaigns: (id)sender
{
    [NSApp stopModal];
    [UserWindow orderOut:nil];
    [self ProcessMyOpenFile: MyOpenFile mode:1 overwrite:OverwriteUser];
}

-(IBAction)installUserCampaignsSkip: (id)sender
{
    [NSApp stopModal];
    [UserWindow orderOut:nil];
}

-(IBAction)installUserOverwrite: (id)sender
{
    OverwriteUser = !OverwriteUser;
}


-(void)installEachDirFrom:(NSString *)FromDir to:(NSString *)ToDir overwrite:(BOOL)ow
{
    BOOL isDir;
    fprintf(stderr, "Copying %s to %s\n",[FromDir cString], [ToDir cString]);
    NSDirectoryEnumerator *direnum = [[NSFileManager defaultManager] enumeratorAtPath:FromDir];
    NSString *pname;
    while (pname = [direnum nextObject]) 
        {
        if (([[pname pathExtension] isEqualToString:@"rtfd"])||([[pname uppercaseString] isEqualToString:@"MAKEFILE"])||([[pname uppercaseString] isEqualToString:@"README"]))
            {
            [direnum skipDescendents]; /* don’t enumerate this directory */
            }else {
            NSString *myFile = [FromDir stringByAppendingPathComponent: pname];
            NSString *toCopyTo = [ToDir stringByAppendingPathComponent:/*[*/pname /*lastPathComponent]*/];
            fprintf(stderr, "\nTrying to copy %s to %s...", [pname cString], [toCopyTo cString]);
            if ([manager fileExistsAtPath:toCopyTo] && ow)
                {
                if ([manager fileExistsAtPath:myFile isDirectory:&isDir] && isDir)
                        {
                        fprintf(stderr, "Directory already exists and we don't delete those :D\n");
                        }else{
                        fprintf(stderr, "file exists and overwrite is enabled, deleting...");
                        if ([manager removeFileAtPath:toCopyTo handler:nil])
                            {
                            fprintf(stderr, "deleted...");
                            }else{
                            fprintf(stderr, "delete failed...");
                            }
                        }
                }
            if ([manager copyPath:myFile toPath:toCopyTo handler:nil])
                {
                fprintf(stderr, "copy succeeded\n");
                }else{
                fprintf(stderr, "copy failed\n");
                }
            }
        }
    
}

-(void)ProcessMyOpenFile: (NSString *)FileToProcess mode:(int)InstallMode overwrite: (BOOL)owMode
{
    // OK, is there a file manager?
    if (manager == nil) {manager = [NSFileManager defaultManager];}
    
    // Now we split the components of the file to get the filename
    NSArray *DroppedParts = [FileToProcess pathComponents];
    int ArraySize = [DroppedParts count];
    ArraySize--; // Decrease size
    //NSString *CopyToPath = [[[NSBundle mainBundle] resourcePath] stringByAppendingPathComponent: @"data/scenarios"];

    NSArray *libPaths = NSSearchPathForDirectoriesInDomains(NSLibraryDirectory, NSUserDomainMask, YES);
	NSString *MapsPath = [[[[[libPaths objectAtIndex:0] stringByAppendingPathComponent:@"Preferences"] stringByAppendingPathComponent:@"wesnoth"] stringByAppendingPathComponent:@"editor"] stringByAppendingPathComponent:@"maps"];

    NSString *CopyToFile = [MapsPath stringByAppendingPathComponent:[DroppedParts objectAtIndex:ArraySize]];
    fprintf(stderr, "Copying %s to %s\n", [FileToProcess cString], [CopyToFile cString]);
    // OK, Are we looking at a file or directory?
    BOOL isDir;
    if ([manager fileExistsAtPath:FileToProcess isDirectory:&isDir] && isDir)
        { 	// This is a directory
        NSString *ResourcesPath = [[NSBundle mainBundle] resourcePath];
		NSString *CampaignTo = [[ResourcesPath stringByAppendingPathComponent:@"data"] stringByAppendingPathComponent:@"campaigns"];
        fprintf(stderr,"Installing a campaign directory suite %s to %s\n", [FileToProcess cString], [CampaignTo cString]);
		[self installEachDirFrom: FileToProcess to: CampaignTo overwrite:owMode];
/*        [self installEachDirFrom:[FileToProcess stringByAppendingPathComponent:@"images"] to: [ResourcesPath stringByAppendingPathComponent:@"images"] overwrite:owMode];
        [self installEachDirFrom:[FileToProcess stringByAppendingPathComponent:@"misc"] to: [ResourcesPath stringByAppendingPathComponent:@"images/misc"] overwrite:owMode];
        [self installEachDirFrom:[FileToProcess stringByAppendingPathComponent:@"data"] to: [ResourcesPath stringByAppendingPathComponent:@"data"] overwrite:owMode];
        [self installEachDirFrom:[FileToProcess stringByAppendingPathComponent:@"scenarios"] to: [ResourcesPath stringByAppendingPathComponent:@"data/scenarios"] overwrite:owMode];
        [self installEachDirFrom:[FileToProcess stringByAppendingPathComponent:@"maps"] to: [ResourcesPath stringByAppendingPathComponent:@"data/maps"] overwrite:owMode];
        [self installEachDirFrom:[FileToProcess stringByAppendingPathComponent:@"units"] to: [ResourcesPath stringByAppendingPathComponent:@"data/units"] overwrite:owMode];*/
        }else{	// Not a directory, so let us just copy :D
        [manager copyPath:FileToProcess toPath:CopyToFile handler:nil];
        }
	// Delete cache directory
	NSArray *paths = NSSearchPathForDirectoriesInDomains(NSLibraryDirectory, NSUserDomainMask, YES);
	[manager removeFileAtPath:[[paths objectAtIndex:0] stringByAppendingPathComponent:@"Preferences/Wesnoth/cache"] handler:nil];
    // Now sort out the backup
   if (InstallMode == 1)	// Do we actually want to make a backup?
        {
        if ([paths count] > 0)  
            {
            NSString *BackUpPath = [[paths objectAtIndex:0] stringByAppendingPathComponent:@"Preferences/Wesnoth/user-scenario-backup"];
            if (!([manager fileExistsAtPath:BackUpPath]))
                {
                // OK no backup dir, so make one then set the flag that it is created
                [manager createDirectoryAtPath:BackUpPath attributes:nil];
                NSString *FlagPath = [[[NSBundle mainBundle] resourcePath] stringByAppendingPathComponent:@"user-scenarios-installed"];
                [manager createFileAtPath: FlagPath contents:nil attributes:nil];
                }
            // Now copy the file to the backup dir
            NSString *BackUpFile = [BackUpPath stringByAppendingPathComponent:[DroppedParts objectAtIndex:ArraySize]];
            if ([manager fileExistsAtPath: BackUpFile] && OverwriteUser)
                {
                fprintf(stderr, "Backup already exists, overwrite enabled so deleting...");
                [manager removeFileAtPath: BackUpFile handler:nil];
                }
            fprintf(stderr, "Copying to Backup Directory");
            [manager copyPath:FileToProcess toPath:BackUpFile handler:nil];
            }
        }

}

// MJP Folder Menu Resposes start

-(IBAction)prefs: (id)sender
{
    //display *myPrefsDisp;
    SDL_Event event;

    event.type=SDL_KEYDOWN;
    event.key.type=SDL_KEYDOWN;
    event.key.state=SDL_PRESSED;
    event.key.keysym.mod = KMOD_CTRL;
    event.key.keysym.sym =SDLK_p;
    if (SDL_PushEvent(&event)==-1)
        {
        fprintf(stderr,"Cannot send keypress");
        }else{
        fprintf(stderr,"Keypress sent");
        }
}

- (IBAction)revealMapsFolder: (id)sender
{
    NSArray *paths;
    paths = NSSearchPathForDirectoriesInDomains(NSLibraryDirectory, NSUserDomainMask, YES);
 if ([paths count] > 0)  {
  // only copying one file
        NSString *destPath = [[[[[paths objectAtIndex:0] stringByAppendingPathComponent:@"Preferences"] stringByAppendingPathComponent:@"wesnoth"] stringByAppendingPathComponent:@"editor"] stringByAppendingPathComponent:@"maps"];
        [[NSWorkspace sharedWorkspace] selectFile:nil inFileViewerRootedAtPath:destPath];     
        }  
}

- (IBAction)revealDataFolder: (id)sender
{
NSString *DataFolderPath = [[[NSBundle mainBundle]pathForResource:@"data" ofType:@""]stringByAppendingPathComponent:@""];
[[NSWorkspace sharedWorkspace] selectFile:nil inFileViewerRootedAtPath:DataFolderPath];     
}

- (IBAction)revealImagesFolder: (id)sender
{
NSString *DataFolderPath = [[[NSBundle mainBundle]pathForResource:@"images" ofType:@""]stringByAppendingPathComponent:@""];
[[NSWorkspace sharedWorkspace] selectFile:nil inFileViewerRootedAtPath:DataFolderPath];     
}

- (IBAction)revealFontsFolder: (id)sender
{
NSString *DataFolderPath = [[[NSBundle mainBundle]pathForResource:@"fonts" ofType:@""]stringByAppendingPathComponent:@""];
[[NSWorkspace sharedWorkspace] selectFile:nil inFileViewerRootedAtPath:DataFolderPath];     
}

- (IBAction)revealSoundsFolder: (id)sender
{
NSString *DataFolderPath = [[[NSBundle mainBundle]pathForResource:@"sounds" ofType:@""]stringByAppendingPathComponent:@""];
[[NSWorkspace sharedWorkspace] selectFile:nil inFileViewerRootedAtPath:DataFolderPath];     
}

- (IBAction)revealMusicFolder: (id)sender
{
NSString *DataFolderPath = [[[NSBundle mainBundle]pathForResource:@"music" ofType:@""]stringByAppendingPathComponent:@""];
[[NSWorkspace sharedWorkspace] selectFile:nil inFileViewerRootedAtPath:DataFolderPath];     
}

- (IBAction)revealPrefsFolder: (id)sender
{
    NSArray *paths;
    paths = NSSearchPathForDirectoriesInDomains(NSLibraryDirectory, NSUserDomainMask, YES);
 if ([paths count] > 0)  {
  // only copying one file
        NSString *destPath = [[paths objectAtIndex:0] stringByAppendingPathComponent:@"Preferences/Wesnoth"];
        [[NSWorkspace sharedWorkspace] selectFile:nil inFileViewerRootedAtPath:destPath];     
        }  
}

- (IBAction)revealCampaignsFolder: (id)sender
{
    NSArray *paths;
    paths = NSSearchPathForDirectoriesInDomains(NSLibraryDirectory, NSUserDomainMask, YES);
 if ([paths count] > 0)  {
  // only copying one file
        NSString *destPath = [[paths objectAtIndex:0] stringByAppendingPathComponent:@"Preferences/Wesnoth/data/campaigns"];
        [[NSWorkspace sharedWorkspace] selectFile:nil inFileViewerRootedAtPath:destPath];     
        }  
}

- (IBAction)revealSaveFolder: (id)sender
{
    NSArray *paths;
    paths = NSSearchPathForDirectoriesInDomains(NSLibraryDirectory, NSUserDomainMask, YES);
 if ([paths count] > 0)  {
  // only copying one file
        NSString *destPath = [[paths objectAtIndex:0] stringByAppendingPathComponent:@"Preferences/Wesnoth/saves"];
        [[NSWorkspace sharedWorkspace] selectFile:nil inFileViewerRootedAtPath:destPath];     
        }  
}

// MJP Folder Menu Responses End

- (IBAction)launchConsole: (id)sender
{
    fprintf(stderr, "Launching Console...\n");
    [[NSWorkspace sharedWorkspace] launchApplication:@"Console.app"];
}

- (IBAction)launchEditor: (id)sender
{
    fprintf(stderr, "Launching Editor...\n");
	if (EditorList == 0) EditorList = [[NSMutableArray alloc] init];
	WNCampaign *newEditor = [[WNCampaign alloc] init];
	[EditorList addObject:newEditor];
	[WNCampaign setMainMenu:[[NSApplication sharedApplication] mainMenu]];

    if ([NSBundle loadNibNamed:@"MainMenu" owner:newEditor])
		{
		NSLog(@"Editor NIB loaded");
		}else{
		NSLog(@"Error loading NIB");
		}
	[WNCampaign setEditorMenu:[[NSApplication sharedApplication] mainMenu]];
	NSLog(@"About to change focus");
	[[newEditor getEditorWindow] makeKeyAndOrderFront:nil];
	
	//[newEditor showEditorWindow];
	
}

// MJP Scenario Responses
-(IBAction)installCampaignsOverwrite: (id)sender
{
    OverwriteBackup = !OverwriteBackup;
}

-(IBAction)installCampaignsNow: (id)sender
{
    [NSApp stopModal];
    [BackUpWindow orderOut:nil];
                
    NSDirectoryEnumerator *direnum = [[NSFileManager defaultManager] enumeratorAtPath:userPath];
    NSString *backupName;
    while (backupName = [direnum nextObject]) 
        {
        if ([[backupName pathExtension] isEqualToString:@"rtfd"]) 
            {
            [direnum skipDescendents]; /* don’t enumerate this directory */
            }else {
            [direnum skipDescendents];	// In fact, we don't want to pursue *any* subdirs
            NSString *myBackupFile = [userPath stringByAppendingPathComponent: backupName];
            fprintf(stderr, "\nTrying to restore %s\n", [backupName cString]);
            [self ProcessMyOpenFile:myBackupFile mode:0 overwrite:OverwriteBackup];
            }
        }
    [manager createFileAtPath: UserInstallCheck contents:nil attributes:nil];
}


-(IBAction)installCampaignsNever: (id)sender
{
    [NSApp stopModal];
    [BackUpWindow orderOut:nil];
    [manager createFileAtPath: UserInstallCheck contents:nil attributes:nil];
}

-(IBAction)installCampaignsAskAgain: (id)sender
{
    [NSApp stopModal];
    [BackUpWindow orderOut:nil];
}

/* MJP Web links */
- (IBAction)webForum: (id)sender
{
	[[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:@"http://www.wesnoth.org/forum"]];
}

- (IBAction)webDownloads: (id)sender
{
	[[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:@"http://www.wesnoth.org/downloads.htm"]];
}

- (IBAction)webMainSite: (id)sender
{
	[[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:@"http://www.wesnoth.org"]];
}


/* Set the working directory to the .app's parent directory */
- (void) setupWorkingDirectory:(BOOL)shouldChdir
{
    char parentdir[MAXPATHLEN];
    char *c;
    
    strncpy ( parentdir, gArgv[0], sizeof(parentdir) );
    c = (char*) parentdir;

    while (*c != '\0')     /* go to end */
        c++;
    
    while (*c != '/')      /* back up to parent */
        c--;
    
    *c++ = '\0';             /* cut off last part (binary name) */
  
	#ifndef Mac_Editor
    if (shouldChdir)
    {
		assert ( chdir (parentdir) == 0 );   /* chdir to the binary app's parent */
		assert ( chdir ("../Resources/") == 0 ); /* chdir to the .app's parent */ // MJP removed ../../
	}
	#endif
    
    // MJP Check for prefs dir and if it doesn't exist make it
/*    DIR* MyDir = opendir("/Library/Preferences/Wesnoth");
    if (MyDir == NULL)
        {
        mkdir("/Library/Preferences/Wesnoth", 00770);
        std::ofstream fOut("/Library/Preferences/Wesnoth/preferences");
        fOut<<"fullscreen=false\nturbo=true\n";
        fOut.close();
        }
 */   
}

#if SDL_USE_NIB_FILE

/* Fix menu to contain the real app name instead of "SDL App" */
- (void)fixMenu:(NSMenu *)aMenu withAppName:(NSString *)appName
{
    NSRange aRange;
    NSEnumerator *enumerator;
    NSMenuItem *menuItem;

    aRange = [[aMenu title] rangeOfString:@"SDL App"];
    if (aRange.length != 0)
        [aMenu setTitle: [[aMenu title] stringByReplacingRange:aRange with:appName]];

    enumerator = [[aMenu itemArray] objectEnumerator];
    while ((menuItem = [enumerator nextObject]))
    {
        aRange = [[menuItem title] rangeOfString:@"SDL App"];
        if (aRange.length != 0)
            [menuItem setTitle: [[menuItem title] stringByReplacingRange:aRange with:appName]];
        if ([menuItem hasSubmenu])
            [self fixMenu:[menuItem submenu] withAppName:appName];
    }
    [ aMenu sizeToFit ];
}

#else

void setupAppleMenu(void)
{
    /* warning: this code is very odd */
    NSAppleMenuController *appleMenuController;
    NSMenu *appleMenu;
    NSMenuItem *appleMenuItem;

    appleMenuController = [[NSAppleMenuController alloc] init];
    appleMenu = [[NSMenu alloc] initWithTitle:@""];
    appleMenuItem = [[NSMenuItem alloc] initWithTitle:@"" action:nil keyEquivalent:@""];
    
    [appleMenuItem setSubmenu:appleMenu];

    /* yes, we do need to add it and then remove it --
       if you don't add it, it doesn't get displayed
       if you don't remove it, you have an extra, titleless item in the menubar
       when you remove it, it appears to stick around
       very, very odd */
    [[NSApp mainMenu] addItem:appleMenuItem];
    [appleMenuController controlMenu:appleMenu];
    [[NSApp mainMenu] removeItem:appleMenuItem];
    [appleMenu release];
    [appleMenuItem release];
}

/* Create a window menu */
void setupWindowMenu(void)
{
    NSMenu		*windowMenu;
    NSMenuItem	*windowMenuItem;
    NSMenuItem	*menuItem;

// MJP Add show folders menus
    NSMenu *folderMenu;
    NSMenuItem *DataFolder, *ImagesFolder, *SoundsFolder, *MusicFolder, *PrefsFolder, *SaveFolder, *folderMenuItemTop;
    folderMenu = [[NSMenu alloc] initWithTitle:@"Folders"];
    DataFolder = [[NSMenuItem alloc] initWithTitle:@"Open Data Folder in Finder" action:@selector(revealDataFolder:) keyEquivalent:@""];
    ImagesFolder = [[NSMenuItem alloc] initWithTitle:@"Open Images Folder in Finder" action:@selector(revealImagesFolder:) keyEquivalent:@""];
    SoundsFolder = [[NSMenuItem alloc] initWithTitle:@"Open Sounds Folder in Finder" action:@selector(revealSoundsFolder:) keyEquivalent:@""];
    MusicFolder = [[NSMenuItem alloc] initWithTitle:@"Open Music Folder in Finder" action:@selector(revealMusicFolder:) keyEquivalent:@""];
    PrefsFolder = [[NSMenuItem alloc] initWithTitle:@"Open Preferences Folder in Finder" action:@selector(revealPrefsFolder:) keyEquivalent:@""];
    SaveFolder = [[NSMenuItem alloc] initWithTitle:@"Open Saved Games Folder in Finder" action:@selector(revealSaveFolder:) keyEquivalent:@""];
    [folderMenu addItem:DataFolder];
    [folderMenu addItem:ImagesFolder];
    [folderMenu addItem:SoundsFolder];
    [folderMenu addItem:MusicFolder];
    [folderMenu addItem:PrefsFolder];
    [folderMenu addItem:SaveFolder];
    
    folderMenuItemTop = [[NSMenuItem alloc] initWithTitle:@"Folder" action:nil keyEquivalent:@""];
    [folderMenuItemTop setSubmenu:folderMenu];
    [[NSApp mainMenu] addItem:folderMenuItemTop];
    
    
// MJP End

    windowMenu = [[NSMenu alloc] initWithTitle:@"Window"];
    
    /* "Minimize" item */
    menuItem = [[NSMenuItem alloc] initWithTitle:@"Minimize" action:@selector(performMiniaturize:) keyEquivalent:@"m"];
    [windowMenu addItem:menuItem];
    [menuItem release];
    
    /* Put menu into the menubar */
    windowMenuItem = [[NSMenuItem alloc] initWithTitle:@"Window" action:nil keyEquivalent:@""];
    [windowMenuItem setSubmenu:windowMenu];
    [[NSApp mainMenu] addItem:windowMenuItem];
    
    /* Tell the application object that this is now the window menu */
    [NSApp setWindowsMenu:windowMenu];

    /* Finally give up our references to the objects */
    [windowMenu release];
    [windowMenuItem release];
}


/* Replacement for NSApplicationMain */
void CustomApplicationMain (argc, argv)
{
    NSAutoreleasePool	*pool = [[NSAutoreleasePool alloc] init];
    SDLMain				*sdlMain;

    /* Ensure the application object is initialised */
    [SDLApplication sharedApplication];
    
    /* Set up the menubar */
    [NSApp setMainMenu:[[NSMenu alloc] init]];
    setupAppleMenu();
    setupWindowMenu();
    
    /* Create SDLMain and make it the app delegate */
    sdlMain = [[SDLMain alloc] init];
    [NSApp setDelegate:sdlMain];
    
    /* Start the main event loop */
    [NSApp run];
    
    [sdlMain release];
    [pool release];
}

#endif
/* Called when the internal event loop has just started running */
- (void) applicationDidFinishLaunching: (NSNotification *) note
{
    int status;
    int myArgc = 0;
    char *myArgv[32];
    
	// Screen Depth
	NSScreen *myScreen = [NSScreen mainScreen];
	NSRect resolution = [myScreen frame];
	fprintf(stderr, "Resolution: %g,%g,%g,%g\n", resolution.origin.x, resolution.origin.y,resolution.size.width, resolution.size.height);
	resolution = [myScreen visibleFrame];
	fprintf(stderr, "Resolution: %g,%g,%g,%g\n", resolution.origin.x, resolution.origin.y,resolution.size.width, resolution.size.height);
	
    // Copy args
    int tmp;
    for (tmp = 0; tmp<gArgc ;tmp++) myArgv[tmp] = gArgv[tmp];
    myArgc = gArgc;
    
    if (WindowsInstalled == nil) {[self findWindows];}
    // MJP Not happy as this red's carbon, but still...
    UInt32 KeyMode;
    KeyMode = GetCurrentEventKeyModifiers();
    if (KeyMode == 2048)
        {
        myArgv[myArgc] = "--debug\0";
        myArgc++;
        [DebugWindow center];
        [DebugWindow makeKeyAndOrderFront:nil];
        }
  /* KeyMap theKeys;
    ::GetKeys(theKeys);
    const bool optionDown = theKeys[1] & kOptionKey;*/

    /* Set the working directory to the .app's parent directory */
    [self setupWorkingDirectory:gFinderLaunch];


// MJP Check for Data Dir, etc and copy if red'd
if (manager == nil) {manager = [NSFileManager defaultManager];}

#ifdef Mac_Editor
	assert ( chdir ([[[[[[[NSBundle mainBundle] bundlePath] stringByDeletingLastPathComponent] stringByAppendingPathComponent:@"Battle For Wesnoth.app"] stringByAppendingPathComponent:@"Contents"] stringByAppendingPathComponent:@"Resources"] cString]) ==0);
#endif

#ifndef Mac_Editor
NSString *ApplicationPath = [[[NSBundle mainBundle]bundlePath]stringByDeletingLastPathComponent];
NSArray *PathArray = [ApplicationPath pathComponents];	// Get all the path components
NSString *ResourcePath = [[NSBundle mainBundle] resourcePath];
NSMutableString *UseFolderPath = [NSMutableString string];

// MJP Set Text Domain Variable
NSString *messagesPath = [ResourcePath stringByAppendingPathComponent:@"messages"];
strcpy(mactextdomain, [messagesPath cString]);

int PathSize = [PathArray count];
if ([[PathArray objectAtIndex:(PathSize-1)] isEqualToString:@"build"])	// Are we in the build directory
    {	// If we are, move up a level
    [UseFolderPath setString:[ApplicationPath stringByDeletingLastPathComponent]];
    }else{	// If not, stay where we are
    [UseFolderPath setString:ApplicationPath];
    }
if (!([manager fileExistsAtPath:[ResourcePath stringByAppendingPathComponent:@"data"]]))
    {
    // OK the data files don't exist... so let us install
  if ([manager fileExistsAtPath:[UseFolderPath stringByAppendingPathComponent:@"data"]])
        {
        if (InstallWindow != nil)
            {
            [InstallWindow center];
            [InstallWindow makeKeyAndOrderFront:nil];
            }
        [manager copyPath:[UseFolderPath stringByAppendingPathComponent:@"data"] toPath:[ResourcePath stringByAppendingPathComponent:@"data"] handler:nil];
        [manager copyPath:[UseFolderPath stringByAppendingPathComponent:@"images"] toPath:[ResourcePath stringByAppendingPathComponent:@"images"] handler:nil];
        [manager copyPath:[UseFolderPath stringByAppendingPathComponent:@"sounds"] toPath:[ResourcePath stringByAppendingPathComponent:@"sounds"] handler:nil];
        [manager copyPath:[UseFolderPath stringByAppendingPathComponent:@"music"] toPath:[ResourcePath stringByAppendingPathComponent:@"music"] handler:nil];
        [manager copyPath:[UseFolderPath stringByAppendingPathComponent:@"icons"] toPath:[ResourcePath stringByAppendingPathComponent:@"icons"] handler:nil];
        [manager copyPath:[UseFolderPath stringByAppendingPathComponent:@"fonts"] toPath:[ResourcePath stringByAppendingPathComponent:@"fonts"] handler:nil];
        [manager copyPath:[UseFolderPath stringByAppendingPathComponent:@"messages"] toPath:[ResourcePath stringByAppendingPathComponent:@"messages"] handler:nil];
        if (InstallWindow != nil) [InstallWindow orderOut: nil];
        }
    }
// MJP Copy Stuff Ends... So all the data files have been created
fprintf(stderr, "\nChecking for user scenarios\n");
ScenariosFolder = [ResourcePath stringByAppendingPathComponent: @"data/scenarios"];
UserInstallCheck = [ResourcePath stringByAppendingPathComponent:@"user-scenarios-installed"];

if (![manager fileExistsAtPath:UserInstallCheck])
    {	// ok, there is no prior install
    fprintf(stderr, "\nNo installation performed yet\n");
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSLibraryDirectory, NSUserDomainMask, YES);
    if ([paths count] > 0)  
        {
        userPath = [[paths objectAtIndex:0] stringByAppendingPathComponent:@"Preferences/Wesnoth/user-scenario-backup"];
        fprintf(stderr,"\nChecking existance of %s\n", [userPath cString]);
        if ([manager fileExistsAtPath:userPath])
            {
            fprintf(stderr, "\nUser scenario dir exists\n");
            
            // Show dialog
            [BackUpWindow center];
            NSModalSession session = [NSApp beginModalSessionForWindow:BackUpWindow];
            while([NSApp runModalSession:session] == NSRunContinuesResponse) {}
            [NSApp endModalSession:session];
            // end dialog
            }
        }  
    }
#endif
// MJP Now we need to check if the symlink exists within data -> user-campaings


#if SDL_USE_NIB_FILE
    /* Set the main menu to contain the real app name instead of "SDL App" */
// MJP Commented out next
//    [self fixMenu:[NSApp mainMenu] withAppName:[[NSProcessInfo processInfo] processName]];

#endif

    /* Hand off to main application code */
    status = SDL_main (myArgc, myArgv);

    /* We're done, thank you for playing */
    exit(status);
}
@end


@implementation NSString (ReplaceSubString)

- (NSString *)stringByReplacingRange:(NSRange)aRange with:(NSString *)aString
{
    unsigned int bufferSize;
    unsigned int selfLen = [self length];
    unsigned int aStringLen = [aString length];
    unichar *buffer;
    NSRange localRange;
    NSString *result;

    bufferSize = selfLen + aStringLen - aRange.length;
    buffer = NSAllocateMemoryPages(bufferSize*sizeof(unichar));
    
    /* Get first part into buffer */
    localRange.location = 0;
    localRange.length = aRange.location;
    [self getCharacters:buffer range:localRange];
    
    /* Get middle part into buffer */
    localRange.location = 0;
    localRange.length = aStringLen;
    [aString getCharacters:(buffer+aRange.location) range:localRange];
     
    /* Get last part into buffer */
    localRange.location = aRange.location + aRange.length;
    localRange.length = selfLen - localRange.location;
    [self getCharacters:(buffer+aRange.location+aStringLen) range:localRange];
    
    /* Build output string */
    result = [NSString stringWithCharacters:buffer length:bufferSize];
    
    NSDeallocateMemoryPages(buffer, bufferSize);
    
    return result;
}

@end



#ifdef main
#  undef main
#endif


/* Main entry point to executable - should *not* be SDL_main! */
int main (int argc, char **argv)
{
    /* Copy the arguments into a global variable */
    int i;
    
    /* This is passed if we are launched by double-clicking */
    if ( argc >= 2 && strncmp (argv[1], "-psn", 4) == 0 ) {
    #ifdef Mac_Editor_OLD
        char MyFileString[256];
        NSOpenPanel *oPanel = [NSOpenPanel openPanel];
        int result = [oPanel runModalForDirectory:NSHomeDirectory() file:nil];
        if (result == NSOKButton) {
            NSArray *filesToOpen = [oPanel filenames];
            int count = [filesToOpen count];
            NSString *aFile = [filesToOpen objectAtIndex:0];
            [aFile getCString:MyFileString];
            argv[1] = MyFileString;
    
        }

        gArgc = 2;
    #else
        gArgc = 1;	// Was 1 MJP
        //argv[1]="--windowed";	// MJP
    #endif
	gFinderLaunch = YES;
    } else {
        gArgc = argc;
	gFinderLaunch = NO;
    }
    gArgv = (char**) malloc (sizeof(*gArgv) * (gArgc+1));
    assert (gArgv != NULL);
    for (i = 0; i < gArgc; i++)
        gArgv[i] = argv[i];
    gArgv[i] = NULL;


	// MJP Set Version Flag
//	APPKIT_EXTERN double NSAppKitVersionNumber;
	if (floor(NSAppKitVersionNumber) <= NSAppKitVersionNumber10_2)
		{
		EarlierThan10_3 = 1;
		}else{
		EarlierThan10_3 = 2;
		}
	fprintf(stderr, "EarlierThan10_3 set to:%d\n", EarlierThan10_3);



#if SDL_USE_NIB_FILE
    [SDLApplication poseAsClass:[NSApplication class]];
    NSApplicationMain (argc, argv);
#else
    CustomApplicationMain (argc, argv);
#endif
    return 0;
}
