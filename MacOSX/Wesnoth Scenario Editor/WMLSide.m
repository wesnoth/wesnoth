//
//  WMLSide.m
//  Wesnoth Scenario Editor
//
//  Created by Marcus Phillips on Wed Mar 31 2004.
//  Copyright (c) 2004 __MyCompanyName__. All rights reserved.
//

#import "Wesnoth_Scenario_Editor_Prefix.h"
#import "WMLSide.h"


@implementation WMLSide
-(WMLSide *)init
{
    [super init];
    fprintf(stderr, "WMLSide initialising\n");
    SideLevels = [[NSMutableDictionary alloc] init];
    [SideLevels retain];
    [SideLevels setObject:[[WMLSideDetail alloc] init] forKey:@"Easy"];
    [SideLevels setObject:[[WMLSideDetail alloc] init] forKey:@"Normal"];
    [SideLevels setObject:[[WMLSideDetail alloc] init] forKey:@"Hard"];
    name = [[NSString alloc] initWithString:@"Unknown side"];
    [name retain];
	advancedInfo = [[[NSMutableString alloc] init] retain];
    fprintf(stderr, "WMLSide initialised\n");
    return self;
}

-(void)dealloc
{
    [name autorelease];
    [SideLevels autorelease];
    [super dealloc];
}

-(void)encodeWithCoder: (NSCoder *)coder
{
    [coder encodeObject: name forKey:@"name"];
	[coder encodeObject: advancedInfo forKey:@"advancedInfo"];
    [coder encodeObject: SideLevels forKey:@"SideLevels"];
}

- (id)initWithCoder:(NSCoder *)coder
{
    [super init];
    
    name = [[coder decodeObjectForKey:@"name"] retain];
    SideLevels = [[coder decodeObjectForKey:@"SideLevels"] retain];
	advancedInfo = [coder decodeObjectForKey:@"advancedInfo"];
	if (advancedInfo == nil) advancedInfo = [[NSMutableString alloc] init];
	[advancedInfo retain];
    
    return self;
}


-(NSString *)getName
{
    return name;
}

-(void)setName:(NSString *)newName
{
    [name autorelease];
    name = [NSString stringWithString: newName];
    [name retain];
}

-(WMLSideDetail *)detailForKey:(NSString *)key
{
    return [SideLevels objectForKey: key];
}

-(void)exportSideToFile:(FILE *)file withTeams:teams withIndex:(int)index withMap:(WMLMap *)map withSide:(int)side
{
    fprintf(file,"#ifdef EASY\n");
    [[SideLevels objectForKey:@"Easy"] exportSideDetailToFile:file withTeams:teams withIndex:index withMap:(WMLMap *)map withDifficulty:@"Easy" withSide:side withInfo:advancedInfo];
    fprintf(file,"#endif\n\n");
    fprintf(file,"#ifdef NORMAL\n");
    [[SideLevels objectForKey:@"Normal"] exportSideDetailToFile:file withTeams:teams withIndex:index withMap:(WMLMap *)map withDifficulty:@"Normal" withSide:side withInfo:advancedInfo];
    fprintf(file,"#endif\n\n");
    fprintf(file,"#ifdef HARD\n");
    [[SideLevels objectForKey:@"Hard"] exportSideDetailToFile:file withTeams:teams withIndex:index withMap:(WMLMap *)map withDifficulty:@"Hard" withSide:side withInfo:advancedInfo];
    fprintf(file,"#endif\n\n");
}

-(void)setSideAdvancedInfo:(NSString *)newInfo
{
	[advancedInfo setString: newInfo];
}

-(NSMutableString *)getSideAdvancedInfo
{
	return advancedInfo;
}

@end
