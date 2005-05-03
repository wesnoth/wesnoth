//
//  WNUnits.h
//  Wesnoth Scenario Editor
//
//  Created by Marcus Phillips on Fri Mar 26 2004.
//  Copyright (c) 2004 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "WMLTag.h"


@interface WNUnits : NSObject {

}
+(void)init;
+(BOOL)initialised;
+(int)count;
+(WMLTag *)unitAtIndex:(int)index;
+(NSString *)unitNameAtIndex:(int)index;
+(int)unitIndexForName:(NSString *)findName;
+(NSMutableArray *)convertIndexArray: (NSMutableArray *)inArray;
+(NSMutableArray *)convertNameArray: (NSMutableArray *)inArray;
+(NSMutableArray *)getUnitNames;
+(NSImage *)imageAtIndex:(int)index;
+(NSImage *)scaledImageAtIndex:(int)index;
+(void)resizeTo:(NSSize)newSize;
@end
