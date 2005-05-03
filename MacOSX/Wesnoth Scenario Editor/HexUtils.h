//
//  HexUtils.h
//  Wesnoth Scenario Editor
//
//  Created by Marcus on Fri Mar 26 2004.
//  Copyright (c) 2004 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>


@interface HexUtils : NSObject {

}

+(void)initWithWidth: (int)width;
+(NSPoint)hexFromX:(int)PixelX y:(int)PixelY;
+(NSPoint)pixelFromHexX:(int)ArrayX y:(int)ArrayY;
+(NSPoint)flatHexFromX:(int)PixelX y:(int)PixelY;
+(NSPoint)pixelFromFlatHexX:(int)ArrayX y:(int)ArrayY;
@end
