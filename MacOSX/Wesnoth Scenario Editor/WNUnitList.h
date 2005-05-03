/* WNUnitList */

#import <Cocoa/Cocoa.h>

@interface WNUnitList : NSObject
{
}
- (void)awakeFromNib;
- (int)numberOfRowsInTableView:(NSTableView *)tableView;
- (id)tableView:(NSTableView *)tableView objectValueForTableColumn:(NSTableColumn *)tableColumn row:(int)row;
@end
