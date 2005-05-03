//
//  WNCharacters.m
//  Wesnoth Scenario Editor
//
//  Created by Marcus Phillips on Mon Apr 19 2004.
//  Copyright (c) 2004 __MyCompanyName__. All rights reserved.
//

#import "Wesnoth_Scenario_Editor_Prefix.h"
#import "WNCharacters.h"
#import "WNUnits.h"


@implementation WNCharacters
-(WNCharacters *)init
{
    [super init];
    names = [[NSMutableArray alloc] init];
    unitType = [[NSMutableArray alloc] init];
    hitPercentage = [[NSMutableArray alloc] init];
    useCustomDialog = [[NSMutableArray alloc] init];
    dialogImage = [[NSMutableArray alloc] init];
    trait1 = [[NSMutableArray alloc] init];
    trait2 = [[NSMutableArray alloc] init];
    onDeathAction = [[NSMutableArray alloc] init];
    onDeathMessage = [[NSMutableArray alloc] init];
	ai = [[[NSMutableArray alloc] init] retain];
    
    [names retain];
    [unitType retain];
    [hitPercentage retain];
    [useCustomDialog retain];
    [dialogImage retain];
    [trait1 retain];
    [trait2 retain];
    [onDeathAction retain];
    [onDeathMessage retain];
    
    [self newCharacter];
    
    return self;
}

-(void) dealloc
{
    [names autorelease];
    [unitType autorelease];
    [hitPercentage autorelease];
    [useCustomDialog autorelease];
    [dialogImage autorelease];
    [trait1 autorelease];
    [trait2 autorelease];
    [onDeathAction autorelease];
    [onDeathMessage autorelease];
	[ai autorelease];
    
    [super dealloc];
}

-(void)encodeWithCoder: (NSCoder *)coder
{
    NSMutableArray *tmpUnitArray = [WNUnits convertIndexArray: unitType];
    [coder encodeObject: names forKey:@"names"];
    [coder encodeObject: tmpUnitArray forKey:@"unitType"];
    [coder encodeObject: hitPercentage forKey:@"hitPercentage"];
    [coder encodeObject: useCustomDialog forKey:@"useCustomDialog"];
    [coder encodeObject: dialogImage forKey:@"dialogImage"];
    [coder encodeObject: trait1 forKey:@"trait1"];
    [coder encodeObject: trait2 forKey:@"trait2"];
    [coder encodeObject: onDeathAction forKey:@"onDeathAction"];
    [coder encodeObject: onDeathMessage forKey:@"onDeathMessage"];
	[coder encodeObject: ai forKey:@"ai"];
    [tmpUnitArray autorelease];
}

- (id)initWithCoder:(NSCoder *)coder
{
    [super init];
    names = [[coder decodeObjectForKey:@"names"] retain];
    unitType = [WNUnits convertNameArray:[coder decodeObjectForKey:@"unitType"]];
    hitPercentage = [[coder decodeObjectForKey:@"hitPercentage"] retain];
    useCustomDialog = [[coder decodeObjectForKey:@"useCustomDialog"] retain];
    dialogImage = [[coder decodeObjectForKey:@"dialogImage"] retain];
    trait1 = [[coder decodeObjectForKey:@"trait1"] retain];
    trait2 = [[coder decodeObjectForKey:@"trait2"] retain];
    onDeathAction = [[coder decodeObjectForKey:@"onDeathAction"] retain];
    onDeathMessage = [[coder decodeObjectForKey:@"onDeathMessage"] retain];
	ai = [coder decodeObjectForKey:@"ai"];
	if (ai==nil)
		{
		ai = [[[NSMutableArray alloc] init] retain];
		int loop;
		for (loop=0; loop<[names count] ;loop++) [ai addObject:@"default"];
		}
	[ai retain];
    
    return self;
}



-(int) count
{
    return [names count];
}

-(int) newCharacter
{
    [names addObject:@"Unknown Warrior"];
    [unitType addObject:[[NSNumber alloc] initWithInt:0]];
    [hitPercentage addObject:[[NSNumber alloc] initWithFloat:100.0]];
    [useCustomDialog addObject:@"No"];
    [dialogImage addObject:[[NSImage alloc] init]];
    [trait1 addObject:@"[None Specified]"];
    [trait2 addObject:@"[None Specified]"];
    [onDeathAction addObject:@"Nothing"];
    [onDeathMessage addObject:@"I am defeated... aaaaarrrrfgggghhh"];
	[ai addObject:@"default"];
    return ([names count]-1);
}

-(void)setNameAtIndex:(int)index to:(NSString *)newName
{
	[names replaceObjectAtIndex:index withObject: newName];
}

-(NSString *)getNameAtIndex: (int)index
{
    return [names objectAtIndex: index];
}

-(int)getUnitTypeAtIndex: (int)index
{
    return [[unitType objectAtIndex:index] intValue];
}

-(void)setUnitTypeAtIndex: (int)index to:(int)newType
{
    [unitType replaceObjectAtIndex:index withObject: [[NSNumber alloc] initWithInt:newType]];
}

-(void)setDialogImageAtIndex:(int)index to:(NSImage *)newImage
{
    [dialogImage replaceObjectAtIndex: index withObject: newImage];
}

-(NSImage *)getDialogImageAtIndex:(int)index
{
    return [dialogImage objectAtIndex:index];
}

-(void)setHPAtIndex:(int)index to:(float)newVal
{
    [hitPercentage replaceObjectAtIndex:index withObject:[[NSNumber alloc] initWithFloat:newVal]];
}

-(float)getHPAtIndex:(int)index
{
    return [[hitPercentage objectAtIndex:index] floatValue];
}

-(void)setDeathMessageAtIndex:(int)index to:(NSString *)newMessage
{
    [onDeathMessage replaceObjectAtIndex:index withObject:newMessage];
}

-(NSString *)getDeathMessageAtIndex:(int)index
{
    return [onDeathMessage objectAtIndex:index];
}

-(void)setTrait:(int)traitNo atIndex:(int)index to:(NSString *)newTrait
{
	switch(traitNo)
		{
		case 1:
			[trait1 replaceObjectAtIndex:index withObject:newTrait];
			break;
		case 2:
			[trait2 replaceObjectAtIndex:index withObject:newTrait];
			break;
		}
}

-(NSString *)getTrait:(int)trainNo atIndex:(int)index
{
	switch(trainNo)
		{
		case 1:
			return [trait1 objectAtIndex:index];
			break;
		case 2:
			return [trait2 objectAtIndex:index];
			break;
		}
	return nil;
}

-(void)setAIAtIndex:(int)index to:(NSString *)newAI
{
	[ai replaceObjectAtIndex:index withObject:newAI];
}

-(NSString *)getAIAtIndex:(int)index
{
	return [ai objectAtIndex:index];
}

-(void)setDeathActionAtIndex:(int)index to:(NSString *)newAction
{
	[onDeathAction replaceObjectAtIndex:index withObject:newAction];
}

-(NSString *)getDeathActionAtIndex:(int)index
{
	return [onDeathAction objectAtIndex:index];
}

-(void)initInPlay
{
	inPlay = [[[NSMutableDictionary alloc] init] retain];
}

-(void)clearInPlay
{
	[inPlay autorelease];
}
-(void)setInPlay:(int)index
{
	fprintf(stderr, "Marking unit %d in play\n", index);
	if (inPlay == nil) [self initInPlay];
	[inPlay setObject:@"Found" forKey:[NSNumber numberWithInt: index]];
}

-(NSMutableString *)exportCharacterEventsFromScenario: (WMLScenario *)sender
{   // Export events for those characters marked as in play
	NSArray *keys = [inPlay allKeys];
	NSMutableString *text = [[[NSMutableString alloc] init] retain];
	
	fprintf(stderr, "Getting next scenario info for Character events...");
	NSMutableDictionary *nextScenarioText = [sender getNextScenarioText];
	[nextScenarioText retain];
	fprintf(stderr, "Done\n");
	
	int noInPlay = [keys count];
	int i, index;
	NSString *doOnDeath;
	
	for (i=0; i<noInPlay ;i++)
		{
		index = [[keys objectAtIndex:i] intValue];
		// first death message
		[text appendString:@"[event]\nname=die\n[filter]\ndescription="];
		[text appendString:[names objectAtIndex:index]];
		[text appendString:@"\n[/filter]\n[message]\nspeaker=unit\nmessage=\""];
		[text appendString:[onDeathMessage objectAtIndex:index]];
		// Need to add custom icon stuff
		[text appendString:@"\"\n[/message]\n"];
		
		doOnDeath = [onDeathAction objectAtIndex:index];
		if ([doOnDeath isEqualTo:@"Victory"])
			{
			[text appendString:@"[endlevel]\nresult=victory\n"];
			[text appendString:[nextScenarioText objectForKey:@"Victory"]];
			[text appendString:@"[/endlevel]\n"];
			}
		if ([doOnDeath isEqualTo:@"Continue"])
			{
			[text appendString:@"[endlevel]\nresult=continue\n"];
			[text appendString:[nextScenarioText objectForKey:@"Continue"]];
			[text appendString:@"[/endlevel]\n"];
			}
			
		[text appendString:@"[/event]\n\n"];
		}
	[nextScenarioText autorelease];
	fprintf(stderr, "Character events export prepared\n");
	return text;	
}

@end
