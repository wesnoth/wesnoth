//
//  WNEventDataSource.h
//  Wesnoth Scenario Editor
//
//  Created by Marcus Phillips on Tue May 25 2004.
//  Copyright (c) 2004 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>


@interface WNEventDataSource : NSObject {

}
- (int)outlineView:(NSOutlineView *)outlineView numberOfChildrenOfItem:(WMLEvent *)item;
- (id)outlineView:(NSOutlineView *)outlineView child:(int)index ofItem:(WMLEvent *)item;
- (BOOL)outlineView:(NSOutlineView *)outlineView isItemExpandable:(WMLEvent *)item;
- (id)outlineView:(NSOutlineView *)outlineView objectValueForTableColumn:(NSTableColumn *)tableColumn byItem:(WMLEvent *)item;
- (void)outlineView:(NSOutlineView *)outlineView setObjectValue:(id)object forTableColumn:(NSTableColumn *)tableColumn byItem:(id)item;
@end
