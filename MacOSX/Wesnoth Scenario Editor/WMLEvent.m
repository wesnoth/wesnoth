//
//  WMLEvent.m
//  Wesnoth Scenario Editor
//
//  Created by Marcus Phillips on Mon May 24 2004.
//  Copyright (c) 2004 __MyCompanyName__. All rights reserved.
//

#import "Wesnoth_Scenario_Editor_Prefix.h"
#import "WMLEvent.h"
#import "WNUnits.h"
#import "WNEventLookup.h"


@implementation WMLEvent
-(WMLEvent *)initAsTag:(BOOL)setTag withName:(NSString *)newName withStringValue:(NSString *)newValue withIntValue:(int)newInt withType:(int)newType withParent:(WMLEvent *)newParent
{
	name = [[[NSMutableString alloc] initWithString:newName] retain];
	tag = [[[NSMutableString alloc] initWithString:newName] retain];
	stringValue = [[[NSMutableString alloc] initWithString:newValue] retain];
	intValue = 0;
	mapLoc.x = 0;
	mapLoc.y = 0;
	isTag = setTag;
	type = newType;
	parent = newParent;
	children = [[[NSMutableArray alloc] init] retain];
	return self;
}

-(WMLEvent *)initAsTag:(BOOL)newTag withType:(int)newType withParent:(id)newParent
{
	return [self initAsTag:newTag withName:@"" withStringValue:@"" withIntValue:0 withType:newType withParent:newParent];
}

-(WMLEvent *)initWithWMLTag:(WMLTag  *)wmlTag withParent:(WMLEvent *)newParent
{   // Takes a WMLTag heirarchy and creates an event heirarchy to match
	// First create parent
	WMLEvent *child;
	NSString *value, *setting, *strSetting;
	fprintf(stderr, "initWithWMLTag...%s\n", [[wmlTag getTag] cString]);
/*	WMLEvent *thisEvent = [[[WMLEvent alloc] initAsTag:YES withName:[tag getTag] withStringValue:@"" withIntValue:0 withType:0 withParent:newParent] retain];*/
	[self initAsTag:YES withName:[wmlTag getTag] withStringValue:@"" withIntValue:0 withType:0 withParent:newParent];
	NSArray *settings = [wmlTag getTagSettingKeys];
	int i, thisType, intSetting;
	fprintf(stderr,"initWithWMLTag found %d settings...\n", [settings count]);
	for (i=0; i<[settings count] ; i++)
		{
		setting = [settings objectAtIndex:i];
		value = [wmlTag getSettingFor:setting];
		thisType = [WNEventLookup typeForSetting:value];
		fprintf(stderr, "Type:%d... for *%s*", thisType, [value cString]);
		switch(thisType)
			{
			case 0: // No Value
				intSetting = 0;
				strSetting =@"";
				break;
			case 1: // Character
				intSetting = 0;
				strSetting = [[[WNCampaign getMainCampaign] getCharacters] getNameAtIndex:0];
				break;
			case 2: // Unit
				intSetting = 0;
				strSetting = [WNUnits unitNameAtIndex:0];
				break;
			case 3: // Side;
				intSetting = 0;
				strSetting = [[[WNCampaign getActiveScenario] getSideAtIndex:0] getName];
				break;
			case 4: // maploc
				intSetting = 0;
				strSetting = @"x=0, y=0";
				break;
			case 5: // text
				intSetting = 0;
				strSetting = @"Set me!";
				break;
			default:
				intSetting = 0;
				strSetting = @"unknown";
				break;
			}
		fprintf(stderr, "Setting:%d str:%s\n", i, [strSetting cString]);
		child = [[WMLEvent alloc] initAsTag:NO withName:setting withStringValue:strSetting withIntValue:intSetting withType:thisType withParent:self];
		[children addObject: child];
		}
	return self;
}

- (void)encodeWithCoder: (NSCoder *)coder
{
	[coder encodeObject:name forKey:@"name"];
	[coder encodeObject:tag forKey:@"tag"];
	[coder encodeObject:stringValue forKey:@"stringValue"];
	[coder encodeInt:intValue forKey:@"intValue"];
	[coder encodePoint:mapLoc forKey:@"mapLoc"];
	[coder encodeInt:isTag forKey:@"isTag"];
	[coder encodeInt:type forKey:@"type"];
	[coder encodeObject:children forKey:@"children"];
	[coder encodeObject:parent forKey:@"parent"];	
}

- (id)initWithCoder:(NSCoder *)coder
{
	fprintf(stderr, "[WMLEvent] initWithCoder\n");
	[super init];
	name = [coder decodeObjectForKey:@"name"];
	[name retain];
	tag = [coder decodeObjectForKey:@"tag"];
	[tag retain];
	stringValue = [coder decodeObjectForKey:@"stringValue"];
	[stringValue retain];
	intValue = [coder decodeIntForKey:@"intValue"];
	mapLoc = [coder decodePointForKey:@"mapLoc"];
	isTag = [coder decodeIntForKey:@"isTag"];
	type = [coder decodeIntForKey:@"type"];
	children = [coder decodeObjectForKey:@"children"];
	[children retain];
	parent = [coder decodeObjectForKey:@"parent"];
	return self;
}

-(void)dealloc
{
	[name autorelease];
	[tag autorelease];
	[stringValue autorelease];
	[children autorelease];
}

-(void)setEventName:(NSString *)newName
{
	[name setString:newName];
}

-(NSString *)getEventName
{
	return name;
}

-(void)setEventValue:(int)newValue string:(NSString *)newString
{
	[stringValue setString:newString];
	intValue = newValue;
}

-(NSString *)getEventStringValue
{
	char tmpBuf[20];
	
	switch(type)
		{
		case 1: // Character
			[stringValue setString: [[[WNCampaign getMainCampaign] getCharacters] getNameAtIndex:intValue]];
			break;
		case 2: // Unit
			[stringValue setString: [WNUnits unitNameAtIndex: intValue]];
			break;
		case 3: // Side
			[stringValue setString: [[[WNCampaign getActiveScenario] getSideAtIndex:intValue] getName]];
			break;
		case 4: // Maploc
			sprintf(tmpBuf,"x=%g, y=%g", mapLoc.x, mapLoc.y);
			[stringValue setString:[NSString stringWithCString: tmpBuf]];
			break;
		}
		
	return stringValue;
}

-(void)setEventIntValue:(int)newVal
{
	intValue = newVal;
}

-(int)getEventIntValue
{
	return intValue;
}

-(int)noEventChildren
{
	return [children count];
}

-(void)addEventChild:(WMLEvent *)newEventChild
{
	if (newEventChild == nil) fprintf(stderr, "ERROR - Added nil child\n");
	[children addObject:newEventChild];
}

-(void)removeEventChild:(WMLEvent *)eventChild
{
	[children removeObjectIdenticalTo:eventChild];
}

-(WMLEvent *)getEventChildAtIndex:(int)index
{
	return [children objectAtIndex:index];
}

-(BOOL)isEventATag
{
	return isTag;
}

-(int)getEventType
{
	return type;
}

-(WMLEvent *)getEventParent
{
	return parent;
} 

-(void)setEventMapLoc:(NSPoint)mapPos
{
	mapLoc = mapPos;
}

-(NSPoint)getEventMapLoc
{
	return mapLoc;
}

-(NSMutableString *)getEventTag
{
	return tag;
}

-(void)exportEventToString:(NSMutableString *)outputString
{	// parses the event as text and appends to the outputstring. Recurses through children
	[outputString appendString:@"\n"];

	int childLoop;
	int childCount;
	NSMutableString *closeTag = [[NSMutableString alloc] init];
	char buf[64];
	
	if ([self isEventATag])
		{
		[outputString appendString:tag];
		childCount = [children count];
		for (childLoop = 0; childLoop<childCount ;childLoop++) [[children objectAtIndex:childLoop] exportEventToString:outputString];
		[outputString appendString:@"\n"];
		[closeTag setString:tag];
		[closeTag insertString:@"/" atIndex:1];
		[outputString appendString:closeTag];
		}else{
		[outputString appendString:tag];
		[outputString appendString:@"="];
		switch(type)
			{
			case 1:	// Character
				[outputString appendString: [[[WNCampaign getMainCampaign] getCharacters] getNameAtIndex:intValue]];
				break;
			case 2:	// Unit
				[outputString appendString: [WNUnits unitNameAtIndex: intValue]];
				break;
			case 3:	// Side
				sprintf(buf,"%d",intValue);
				[outputString appendString:[NSString stringWithCString:buf]];
				break;
			case 4:	// Maploc
				sprintf(buf,"x=%g,y=%g",mapLoc.x,mapLoc.y);
				[outputString appendString: [NSString stringWithCString:buf]];
				break;
			case 5:	// Info
				break;
			case 6:	// Text
				[outputString appendString:stringValue];
				break;
			}
		[outputString appendString:@"\n"];
		}
}

@end
