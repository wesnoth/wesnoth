/* WNEventEditor */

#import <Cocoa/Cocoa.h>
#import "WNCharacterListView.h"
#import "WNIconListView.h"

@interface WNEventEditor : NSObject
{
    IBOutlet NSPopUpButton *actionType;
    IBOutlet WNCharacterListView *eventCharacterList;
    IBOutlet NSTabView *eventInfoTabs;
    IBOutlet NSOutlineView *eventList;
    IBOutlet NSTableView *eventSideList;
    IBOutlet NSPopUpButton *eventTrigger;
    IBOutlet WNIconListView *eventUnitList;
    IBOutlet NSTextField *turnNo;
}
- (void)awakeFromNib;
- (IBAction)actionAdd:(id)sender;
- (IBAction)actionTypeSelect:(id)sender;
- (IBAction)eventAdd:(id)sender;
- (IBAction)eventCharacterSelect:(id)sender;
- (IBAction)eventRemove:(id)sender;
- (IBAction)eventSelect:(id)sender;
- (IBAction)eventSideSelect:(id)sender;
- (IBAction)eventTriggerSelect:(id)sender;
- (IBAction)eventUnitSelect:(id)sender;
- (IBAction)filterAdd:(id)sender;
- (WMLEvent *)findSelectedEvent;
- (IBAction)turnNoSet:(id)sender;
- (void)finishedLaunching:(NSNotification *)notification;
-(void)eventListReset:(NSNotification *)notification;
@end
