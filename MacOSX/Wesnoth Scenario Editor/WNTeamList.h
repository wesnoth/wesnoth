/* WNTeamList */

#import <Cocoa/Cocoa.h>

@interface WNTeamList : NSObject
{
}
- (int)numberOfRowsInTableView:(NSTableView *)tableView;
- (id)tableView:(NSTableView *)tableView 
        objectValueForTableColumn:(NSTableColumn *)tableColumn row:(int)row;
@end
