#import "Wesnoth_Scenario_Editor_Prefix.h"
#import "WNEventEditor.h"
#import "WNEventLookup.h"

@implementation WNEventEditor

-(void)awakeFromNib
{
	[[NSNotificationCenter defaultCenter] addObserver:self
		selector:@selector(finishedLaunching:)
		name:@"finishedLaunching" object:nil];
	[[NSNotificationCenter defaultCenter] addObserver:self
		selector:@selector(eventListReset:)
		name:@"campaignReset" object:nil];
	[[NSNotificationCenter defaultCenter] addObserver:self
		selector:@selector(eventListReset:)
		name:@"eventListReset" object:nil];
}

- (IBAction)actionAdd:(id)sender
{
	int row = [eventList selectedRow];
	WMLEvent *event, *newChild;
	if (row != -1)
		{
		fprintf(stderr, "Adding action...\n");
		event = [eventList itemAtRow: row];
		newChild = [[WMLEvent alloc] initWithWMLTag:[WNCampaign getEventTagForTag:[actionType titleOfSelectedItem]] withParent:event];
		fprintf(stderr, "Action prepared...\n");
		[event addEventChild:newChild]; 
		fprintf(stderr, "Action added\n");
		}
	[eventList reloadData];
}

- (IBAction)actionTypeSelect:(id)sender
{
}

- (IBAction)eventAdd:(id)sender
{
	[[WNCampaign getActiveScenario] addNewEvent];
	[eventList reloadData];
}

- (IBAction)eventCharacterSelect:(id)sender
{
	int thisChar = [sender selectedRow];
	WNCharacters *characters = [[WNCampaign getMainCampaign] getCharacters];
	[[self findSelectedEvent] setEventValue:thisChar string:[characters getNameAtIndex: thisChar]];
	[eventList reloadData];
}

- (IBAction)eventRemove:(id)sender
{
	WMLEvent *thisEvent = [self findSelectedEvent];
	WMLEvent *parentEvent = [thisEvent getEventParent];
	NSString *tag = [parentEvent getEventTag];
	if (([tag hasPrefix:@"[filter"])||([thisEvent isEventATag]))
		{
		fprintf(stderr, "Attempting to delete event\n");
		[parentEvent removeEventChild: thisEvent];
		[eventList reloadData];
		}else{
		fprintf(stderr, "Can't delete, not part of a filter nor is a main tag\n");
		}
}

- (IBAction)eventSideSelect:(id)sender
{
	[[self findSelectedEvent] setEventIntValue:[sender selectedRow]];
	[eventList reloadData];
}

- (IBAction)eventTriggerSelect:(id)sender
{
}

- (IBAction)eventUnitSelect:(id)sender
{
	[[self findSelectedEvent] setEventIntValue:[sender selectedRow]];
	[eventList reloadData];
}

- (IBAction)filterAdd:(id)sender
{
}

- (WMLEvent *)findSelectedEvent
{
	int row = [eventList selectedRow];
	if (row != 1) return [eventList itemAtRow: row];
	return nil;
}

- (IBAction)turnNoSet:(id)sender
{
}

- (void)finishedLaunching:(NSNotification *)notification
{
	// Setup action menu
	[WNEventLookup init];	
	[actionType removeAllItems];
	WMLTag *tagList = [WNCampaign getEventTags];
	int noTags = [tagList childrenCount];
	fprintf(stderr, "Initialising action menu... found %d actions \n", noTags);
	int i;
	for (i=0; i< noTags ;i++)
		{
		[actionType addItemWithTitle:[[tagList getChildAtIndex:i] getTag]];
		}

}

- (IBAction)eventSelect:(id)sender
{   // An event has been selected :)
	int row = [sender selectedRow];
	int eventType;
	
	if (row != -1)
		{
		WMLEvent *event = [sender itemAtRow: row];
		[WNEventLookup setCurrentEvent:event];
		eventType = [event getEventType];
		switch(eventType)
			{
			case 1: // Character
				[eventCharacterList selectRow:[event getEventIntValue] byExtendingSelection:NO];
				break;
			case 2: // Unit
				[eventUnitList selectRow:[event getEventIntValue] byExtendingSelection:NO];
				break;
			case 3: // Side
				[eventSideList selectRow:[event getEventIntValue] byExtendingSelection:NO];
				break;
			case 4:// MapLoc
				[WNEventLookup setMapLoc:[event getEventMapLoc]];
				break;
			}
		[eventInfoTabs selectTabViewItemWithIdentifier: [WNEventLookup settingForType:eventType]];
		fprintf(stderr, "Setting tab to :%s\n", [[WNEventLookup settingForType:eventType] cString]);
		[[NSNotificationCenter defaultCenter] postNotificationName:@"eventSelected" object:self];
		}
}

-(void)eventListReset:(NSNotification *)notification
{
	[eventList reloadData];
}
@end
