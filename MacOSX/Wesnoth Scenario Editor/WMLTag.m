//
//  WNWMLReader.m
//  Wesnoth Scenario Editor
//  Class to read a textfile and return Wesnoth code tags
//
//  Created by Marcus Phillips on Wed Mar 24 2004.
//  Copyright (c) 2004 __MyCompanyName__. All rights reserved.
//

#import "Wesnoth_Scenario_Editor_Prefix.h"
#import "WMLTag.h"


@implementation WMLTag
    
- (BOOL)initWithFile: (NSString *)wmlFile setTag:(NSString *)thisTag	
        // Just load a file into a string 1st
    {
    fprintf(stderr, "initialising WMLTag with file:%s\n", [wmlFile UTF8String]);
    return [self initWithString: [NSString stringWithContentsOfFile: wmlFile] setTag:thisTag];
    }
    
-(BOOL)initWithString: (NSString *)wmlString setTag:(NSString *)thisTag
	// Main init method;
    {
    NSString *tmpLine=nil;
    NSString *closeTag=nil;
    NSString *Token, *Value;
    NSRange foundPos, searchRange, equalPos, subRange;
    WMLTag *child;

    // First Allocate data structures
    wmlText = [NSString stringWithString:wmlString];	// Make our copy;
    Children = [[NSMutableArray alloc] init];
    Settings = [[NSMutableDictionary alloc] init];
    Tag = [NSString stringWithString:thisTag];
    
    // Ensure we don't lose these objects
    [wmlText retain];
    [Children retain];
    [Settings retain];
    [Tag retain];
    
    // Set position markers, just to be sure
    Maxpos = [wmlText length];
    Lastpos = 0;
    Curpos = 0;
    
    fprintf(stderr, "Initialising new WMLTag:%s\n", [thisTag UTF8String]);
    do{
        tmpLine = [self getLine];
        if ((tmpLine == nil) || ([tmpLine length]==0))
            {
            //fprintf(stderr,"tmpLine is empty\n");
            }else{
            //fprintf(stderr, "Loaded line:%s\n", [tmpLine UTF8String]);
            switch([tmpLine characterAtIndex:0])	// What is the first char
                {
                case '[':	// Open Tag
                    if ([tmpLine characterAtIndex:1]=='/')
                        {
                        fprintf(stderr,"ERROR in wmlTag init... unpaired closetag found\n");
                        }else{
                        closeTag = [@"[/" stringByAppendingString: [tmpLine substringFromIndex:1]];
                        //fprintf(stderr,"Searching for %s\n", [closeTag UTF8String]);
                        searchRange.location = Curpos;
                        searchRange.length = [wmlText length]-Curpos;
                        foundPos = [wmlText rangeOfString:closeTag options:NSCaseInsensitiveSearch range:searchRange];
                        if (foundPos.length>0)
                            {
                            searchRange.location = Curpos;
                            searchRange.length = (foundPos.location - Curpos);
                            //fprintf(stderr, "Tag is:***%s***\n", [[wmlText substringWithRange: searchRange] UTF8String]);
                            Curpos = foundPos.location+foundPos.length;
                            child = [WMLTag alloc];
                            [child initWithString:[wmlText substringWithRange: searchRange] setTag: tmpLine];
                            [Children addObject:child];
                            }else{
                            fprintf(stderr,"failed to find tag\n");
                            }
                        }
                    break;
                case '#':	// Comment, do nothing
                    fprintf(stderr, "Comment Line... skipping\n");
                    break;
                default:	// ok, check for an assignment now
                    equalPos = [tmpLine rangeOfString:@"="];
                    if (equalPos.length == 0)	// Not found
                        {
                        fprintf(stderr,"Unknown syntax, skipping\n");
                        }else{
                        subRange.location = 0;
                        subRange.length = equalPos.location;
                        Token = [tmpLine substringWithRange: subRange];
                        subRange.location = equalPos.location+1;
                        subRange.length = ([tmpLine length] - equalPos.location)-1;
                        Value = [tmpLine substringWithRange: subRange];
                        [Settings setObject: Value forKey: Token];
                        }
                    break;
                }
            }
    }while(tmpLine != nil);
    
    // OK, basic structures there, now we need to start looking
    [super init];
    return YES;
    }
    
-(NSString *)getLine
{
    int glDone=0;
    char ThisChar;
    
    if (Curpos == Maxpos) return nil;
    Lastpos = Curpos;
    while ((Curpos < Maxpos)&&(glDone == 0))
        {
        ThisChar = [wmlText characterAtIndex:Curpos];
        if (ThisChar == '\n') glDone = 1;
        Curpos++;
        }
    NSRange glRange;
    
    glRange.location = Lastpos;
    glRange.length = Curpos-Lastpos;
    return [[wmlText substringWithRange:glRange] stringByTrimmingCharactersInSet: [NSCharacterSet whitespaceAndNewlineCharacterSet]];
}
    
-(int) childrenCount	// Returns the number of children
{
    return [Children count];
}

-(WMLTag *) getChildAtIndex:(int)cIndex
{
    return [Children objectAtIndex:cIndex];
}
-(int) settingsCount	// Returns the number of settings
{
    return [Settings count];
}

-(NSString *)getSettingFor:(NSString *)sKey
{
    return [Settings objectForKey:sKey];
}

-(NSString *)getTag
{
    return Tag;
}

-(NSArray *)getTagSettingKeys
{
	return [Settings allKeys];
}
@end

