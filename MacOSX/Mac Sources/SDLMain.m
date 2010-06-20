/*	SDLMain.m - main entry point for our Cocoa-ized SDL app
	Initial Version: Darrell Walisser <dwaliss1@purdue.edu>
	Non-NIB-Code & other changes: Max Horn <max@quendi.de>
	Edited a lot for Wesnoth by Ben Anderman <ben@happyspork.com>
*/

#import "SDL.h"
#import "SDLMain.h"

static int    gArgc;
static char  **gArgv;

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

- (BOOL)_handleKeyEquivalent:(NSEvent *)theEvent
{
	[[super mainMenu] performKeyEquivalent:theEvent];
	return YES;
}

- (void) sendEvent:(NSEvent *)event
{
	if(NSKeyDown == [event type] || NSKeyUp == [event type])
	{
		if([event modifierFlags] & NSCommandKeyMask)
		{
			[super sendEvent: event];
		}
	} else {
		[super sendEvent: event];
	}
}
@end


/* The main class of the application, the application's delegate */
@implementation SDLMain

- (IBAction) openHomepage:(id)sender
{
	[[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:@"http://www.wesnoth.org/"]];
}

/* Called when the internal event loop has just started running */
- (void) applicationDidFinishLaunching: (NSNotification *) note
{
	/* This makes SDL give events to Cocoa, so it can handle things like command+h to hide, etc. */
	setenv ("SDL_ENABLEAPPEVENTS", "1", 1);
	setenv ("SDL_VIDEO_ALLOW_SCREENSAVER", "1", 1);
	int status;

	/* Set the working directory to the .app's Resources directory */
	chdir([[[NSBundle mainBundle] resourcePath] fileSystemRepresentation]);

	//setenv("PYTHONHOME", ".", 1); //not needed because we don't use Python anymore

	/* Hand off to main application code */
	status = SDL_main (gArgc, gArgv);

	/* We're done, thank you for playing */
	exit(status);
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
	if (argc >= 2 && strncmp (argv[1], "-psn", 4) == 0)
	{
		gArgc = 1;
	} else {
		gArgc = argc;
	}
	gArgv = (char**) malloc (sizeof(*gArgv) * (gArgc+1));
	assert (gArgv != NULL);
	for (i = 0; i < gArgc; i++)
		gArgv[i] = argv[i];
	gArgv[i] = NULL;

//	[SDLApplication poseAsClass:[NSApplication class]];
//	NSApplicationMain (argc, argv);
	[SDLApplication sharedApplication];
	[NSBundle loadNibNamed:@"SDLMain" owner:NSApp];
	[NSApp run];
	return 0;
}
