//
//  WNTerrains.h
//  Wesnoth Scenario Editor
//
//  Created by Marcus Phillips on Fri Mar 26 2004.
//  Copyright (c) 2004 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "WMLTerrain.h"


@interface WNTerrains : NSObject {

}
+(void)init;
+(WMLTerrain *)terrainAtIndex:(int)terrainNo;
+(BOOL)initialised;
+(int)count;
+(int)terrainForCode: (NSString *)codeString;
+(int)terrainForChar: (char)tmpCode;
+(void)resizeTo:(NSSize)newSize;
+(NSString *)codeAtIndex:(int)index;
@end
