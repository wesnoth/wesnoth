#import "Wesnoth_Scenario_Editor_Prefix.h"
#import "WNScenarioDesignDelegate.h"

@implementation WNScenarioDesignDelegate

- (IBAction)windowDidBecomeKey:(id)sender
{
	NSLog(@"Editor now key");
	[WNCampaign switchToEditorMenu];
}

- (IBAction)windowDidResignKey:(id)sender
{
	NSLog(@"Editor lost key");
	[WNCampaign switchToMainMenu];
}

@end
