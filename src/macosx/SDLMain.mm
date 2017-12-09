/*	SDLMain.m - main entry point for our Cocoa-ized SDL app
	Initial Version: Darrell Walisser <dwaliss1@purdue.edu>
	Non-NIB-Code & other changes: Max Horn <max@quendi.de>
	Edited a lot for Wesnoth by Ben Anderman <ben@happyspork.com>
*/

#import "SDL.h"
#import "SDLMain.h"
#include <vector>

extern "C" int wesnoth_main(int argc, char **argv);
static std::vector<char*> gArgs;

@interface SDLApplication : NSApplication
@end

@implementation SDLApplication
/* Invoked from the Quit menu item */
- (void)terminate:(id)sender
{
	(void) sender;
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
	if(NSEventTypeKeyDown == [event type] || NSEventTypeKeyUp == [event type])
	{
		if([event modifierFlags] & NSEventModifierFlagCommand)
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
	(void) sender;
	[[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:@"http://www.wesnoth.org/"]];
}

/* Called when the internal event loop has just started running */
- (void) applicationDidFinishLaunching: (NSNotification *) note
{
	(void) note;
	/* This makes SDL give events to Cocoa, so it can handle things like command+h to hide, etc. */
	setenv ("SDL_ENABLEAPPEVENTS", "1", 1);
	setenv ("SDL_VIDEO_ALLOW_SCREENSAVER", "1", 1);

    /* Set config files for pango and fontconfig, so the data they need can be found */
	setenv ("PANGO_RC_FILE", "./pangorc", 1);
	setenv ("PANGO_SYSCONFDIR", ".", 1);
	setenv ("PANGO_LIBDIR", ".", 1);
	setenv ("FONTCONFIG_PATH", ".", 1);
	setenv ("FONTCONFIG_FILE", "fonts.conf", 1);

	int status;

	/* Set the working directory to the .app's Resources directory */
	chdir([[[NSBundle mainBundle] resourcePath] fileSystemRepresentation]);

	/* Hand off to main application code */
	status = wesnoth_main(gArgs.size() - 1, gArgs.data());

	/* We're done, thank you for playing */
	exit(status);
}
@end

template<int sz>
bool str_eq(const char* str, const char(& cstr)[sz]) {
	return strncmp(str, cstr, sz) == 0;
}

#ifdef main
#  undef main
#endif

/* Main entry point to executable - should *not* be SDL_main! */
int main (int argc, char **argv)
{
	gArgs.push_back(argv[0]); // Program name
	for (int i = 1; i < argc; i++) {
		// Filter out debug arguments that XCode might pass
		if (str_eq(argv[i], "-ApplePersistenceIgnoreState")) {
			i++; // Skip the argument
			continue;
		}
		if (str_eq(argv[i], "-NSDocumentRevisionsDebugMode")) {
			i++; // Skip the argument
			continue;
		}
		// This is passed if launched by double-clicking
		if (strncmp(argv[i], "-psn", 4) == 0) {
			continue;
		}
		gArgs.push_back(argv[i]);
	}
	gArgs.push_back(nullptr);

	[SDLApplication sharedApplication];

	// loadNibNamed:owner:topLevelObjects was introduced in 10.8 (Mountain Lion).
	// In order to support Lion and Mountain Lion +, we need to see which OS we're
	// on. We do this by testing to see if [NSBundle mainBundle] responds to
	// loadNibNamed:owner:topLevelObjects: ... If so, the app is running on at least
	// Mountain Lion... If not, then the app is running on Lion so we fall back to the
	// the older loadNibNamed:owner: method. If your app does not support Lion, then
	// you can go with strictly the newer one and not deal with the if/else conditional.

	if ([[NSBundle mainBundle] respondsToSelector:@selector(loadNibNamed:owner:topLevelObjects:)]) {
		// We're running on Mountain Lion or higher
		[[NSBundle mainBundle] loadNibNamed:@"SDLMain"
		owner:NSApp
		topLevelObjects:nil];
	} else {
		// We're running on Lion
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"

		[NSBundle loadNibNamed:@"SDLMain" owner:NSApp];

#pragma clang diagnostic pop
	}

	[NSApp run];
	return 0;
}
