//
//  WNCharacters.h
//  Wesnoth Scenario Editor
//
//  Created by Marcus Phillips on Mon Apr 19 2004.
//  Copyright (c) 2004 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface WNCharacters : NSObject {
    NSMutableArray *names;	// Names of character
    NSMutableArray *unitType;	// Parent unit type of characters
    NSMutableArray *hitPercentage;	// Percentage of parent HP
    NSMutableArray *useCustomDialog;	// Dictates whether to use a custom icon
    NSMutableArray *dialogImage;	// This is the custom image to use for dialogs
    NSMutableArray *trait1;	// This is the first of the custom traits
    NSMutableArray *trait2;	// This is the second of the custom traits
    NSMutableArray *onDeathAction;	// This is the custom action on death
    NSMutableArray *onDeathMessage;	// This is what the character will say on death
	NSMutableArray *ai; // This is the AI type
	NSMutableDictionary *inPlay;	// This provides a means of noting which characters are in play
}
-(WNCharacters *)init;
-(void) dealloc;
-(void)encodeWithCoder: (NSCoder *)coder;
- (id)initWithCoder:(NSCoder *)coder;
-(int) count;
-(int) newCharacter;	// Adds a new blank character and then returns its index number
-(void)setNameAtIndex:(int)index to:(NSString *)newName;
-(NSString *)getNameAtIndex: (int)index;
-(int)getUnitTypeAtIndex: (int)index;
-(void)setUnitTypeAtIndex: (int)index to:(int)newType;
-(void)setDialogImageAtIndex:(int)index to:(NSImage *)newImage;
-(NSImage *)getDialogImageAtIndex:(int)index;
-(void)setHPAtIndex:(int)index to:(float)newVal;
-(float)getHPAtIndex:(int)index;
-(void)setDeathMessageAtIndex:(int)index to:(NSString *)newMessage;
-(NSString *)getDeathMessageAtIndex:(int)index;
-(void)setTrait:(int)traitNo atIndex:(int)index to:(NSString *)newTrait;
-(NSString *)getTrait:(int)trainNo atIndex:(int)index;
-(void)setAIAtIndex:(int)index to:(NSString *)newAI;
-(NSString *)getAIAtIndex:(int)index;
-(void)setDeathActionAtIndex:(int)index to:(NSString *)newAction;
-(NSString *)getDeathActionAtIndex:(int)index;
-(void)initInPlay;
-(void)clearInPlay;
-(void)setInPlay:(int)index;
-(NSMutableString *)exportCharacterEventsFromScenario: (id)sender;
@end
