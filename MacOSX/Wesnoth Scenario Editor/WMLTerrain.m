//
//  WMLTerrain.m
//  Wesnoth Scenario Editor
//
//  Created by Marcus Phillips on Thu Mar 25 2004.
//  Copyright (c) 2004 __MyCompanyName__. All rights reserved.
//

#import "Wesnoth_Scenario_Editor_Prefix.h"
#import "WMLTerrain.h"


@implementation WMLTerrain

-(WMLTerrain *) init:(NSString *)inName image:(NSString *)inImage code:(NSString *)inCode userDefined: (BOOL)inUser
{
    [super init];
    
    name = [NSString stringWithString: inName];
    image = [[NSImage alloc] initWithContentsOfFile: inImage];
    scaledImage = [image copy];
    if (image == nil) fprintf(stderr,"Error - File invalid for image\n");
    code = [NSString stringWithString: inCode];
    userDefined = inUser;
    
    [image retain];
    [scaledImage retain];
    [name retain];
    [imageName retain];
    [code retain];
    
    return self;
}

-(void)dealloc
{
    [name autorelease];
    [imageName autorelease];
    [code autorelease];
    [super dealloc];
}

-(NSString *)name
{
    return name;
}

-(NSImage *)image
{
    return image;
}

-(NSImage *)scaledImage
{
    return scaledImage;
}

-(void)resizeTo:(NSSize)newSize
{
	[scaledImage autorelease];
	scaledImage = [image copy];
	[scaledImage setScalesWhenResized:YES];
	[scaledImage setSize: newSize];
	[scaledImage retain];
}

-(void)resizeToID:(id)newID
{
	NSSize mySize = *((NSSize *)newID);
	[self resizeTo: mySize];
}

-(NSString *)terrainCode
{
	return code;
}
@end
