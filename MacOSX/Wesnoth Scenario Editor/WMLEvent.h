//
//  WMLEvent.h
//  Wesnoth Scenario Editor
//
//  Created by Marcus Phillips on Mon May 24 2004.
//  Copyright (c) 2004 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>


@interface WMLEvent : NSObject {
	NSMutableString *name;
	NSMutableString *tag;
	NSMutableString *stringValue;
	int intValue;
	NSPoint mapLoc;
	BOOL isTag; // Set to yes if this is a tag, no if this is description-value pair
	int type;   // 0=none, 1=character, 2=unit, 3=side, 4=maploc, 5=info, 6=text
	NSMutableArray *children;
	WMLEvent *parent;
}
-(WMLEvent *)initAsTag:(BOOL)setTag withName:(NSString *)newName withStringValue:(NSString *)newValue withIntValue:(int)newInt withType:(int)newType withParent:(WMLEvent *)newParent;
-(WMLEvent *)initAsTag:(BOOL)newTag withType:(int)newType withParent:(WMLEvent *)newParent;
-(WMLEvent *)initWithWMLTag:(WMLTag  *)tag withParent:(WMLEvent *)newParent;
- (void)encodeWithCoder: (NSCoder *)coder;
- (id)initWithCoder:(NSCoder *)coder;
-(void)dealloc;
-(void)setEventName:(NSString *)newName;
-(NSString *)getEventName;
-(void)setEventValue:(int)newValue string:(NSString *)newString;
-(NSString *)getEventStringValue;
-(void)setEventIntValue:(int)newVal;
-(int)getEventIntValue;
-(int)noEventChildren;
-(void)addEventChild:(WMLEvent *)newEventChild;
-(void)removeEventChild:(WMLEvent *)eventChild;
-(WMLEvent *)getEventChildAtIndex:(int)index;
-(BOOL)isEventATag;
-(int)getEventType;
-(WMLEvent *)getEventParent;
-(void)setEventMapLoc:(NSPoint)mapPos;
-(NSPoint)getEventMapLoc;
-(NSMutableString *)getEventTag;
-(void)exportEventToString:(NSMutableString *)outputString;
@end
