//
//  WNUtils.m
//  Wesnoth Scenario Editor
//
//  Created by Marcus on Sun Apr 25 2004.
//  Copyright (c) 2004 __MyCompanyName__. All rights reserved.
//

#import "Wesnoth_Scenario_Editor_Prefix.h"
#import "WNUtils.h"


@implementation WNUtils
+(NSString *)replaceSpaces: (NSString *)source
{
	NSMutableString *tmpString = [[NSMutableString alloc]initWithString: source];
	[tmpString replaceOccurrencesOfString:@" " withString:@"_" options:NSLiteralSearch range:NSMakeRange(0, [tmpString length])];
	return tmpString;
}

+(BOOL)writeImage:(NSImage *)startImage toPNGFile:(NSString *)pngFile
{
    NSData *tiff = [startImage TIFFRepresentation]; 
    NSBitmapImageRep *bitmap = [NSBitmapImageRep imageRepWithData:tiff]; 
    return [[bitmap representationUsingType:NSPNGFileType properties:nil] writeToFile:pngFile atomically:YES]; 
}

@end
