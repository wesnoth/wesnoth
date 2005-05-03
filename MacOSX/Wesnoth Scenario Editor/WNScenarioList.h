/* WNScenarioList */

#import <Cocoa/Cocoa.h>
#import "WNCampaign.h"
#import "WMLCampaign.h"

@interface WNScenarioList : NSObject
{
}
- (int)numberOfRowsInTableView:(NSTableView *)tableView;
- (id)tableView:(NSTableView *)tableView objectValueForTableColumn:(NSTableColumn *)tableColumn row:(int)row;
- (void)tableView:(NSTableView *)aTableView setObjectValue:(id)anObject forTableColumn:(NSTableColumn *)aTableColumn row:(int)rowIndex;
@end
