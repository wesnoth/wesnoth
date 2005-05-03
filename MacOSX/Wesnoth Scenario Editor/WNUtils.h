//
//  WNUtils.h
//  Wesnoth Scenario Editor
//
//  Created by Marcus on Sun Apr 25 2004.
//  Copyright (c) 2004 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>


@interface WNUtils : NSObject {

}
+(NSString *)replaceSpaces: (NSString *)source;
+(BOOL)writeImage:(NSImage *)startImage toPNGFile:(NSString *)pngFile;
@end
