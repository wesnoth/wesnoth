//
//  WNEventLookup.h
//  Wesnoth Scenario Editor
//
//  Created by Marcus Phillips on Wed May 26 2004.
//  Copyright (c) 2004 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>


@interface WNEventLookup : NSObject {
}
+(void)init;
+(int)typeForSetting:(NSString *)setting;
+(NSString *)settingForType:(int)type;
+(void)setMapLoc:(NSPoint)mapPos;
+(NSPoint)getMapLoc;
+(void)setCurrentEvent:(WMLEvent *)thisEvent;
+(WMLEvent *)getCurrentEvent;
@end
