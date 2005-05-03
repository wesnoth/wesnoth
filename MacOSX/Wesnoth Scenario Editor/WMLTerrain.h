//
//  WMLTerrain.h
//  Wesnoth Scenario Editor
//
//  Created by Marcus Phillips on Thu Mar 25 2004.
//  Copyright (c) 2004 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>


@interface WMLTerrain : NSObject {
    NSString *name;
    NSString *imageName;
    NSImage *image;
    NSImage *scaledImage;
    NSString *code;
    BOOL userDefined;
}

-(WMLTerrain *) init:(NSString *)inName image:(NSString *)inImage code:(NSString *)inCode userDefined: (BOOL)inUser;
-(void) dealloc;
-(NSString *)name;
-(NSImage *)image;
-(NSImage *)scaledImage;
-(void)resizeTo:(NSSize)newSize;
-(void)resizeToID:(id)newID;
-(NSString *)terrainCode;
@end
