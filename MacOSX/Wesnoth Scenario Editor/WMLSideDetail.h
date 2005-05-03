//
//  WMLSideList.h
//  Wesnoth Scenario Editor
//
//  Created by Marcus Phillips on Wed Mar 31 2004.
//  Copyright (c) 2004 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "WMLMap.h"


@interface WMLSideDetail : NSObject {
    int gold;	// Amount of gold the side starts with
    NSString *leaderName;	// The name of the leader
    NSString *leaderType;	// Unit name of the leader
    int leaderID;	// Unit ID of the leader
    NSMutableArray *recruitTypes;	// Index of type of unit that can be recruited;
    float agression;	// Level of agression for AI units
    NSString *type;	// Whether AI or not
    int team;	// The team this side belongs to
    BOOL canRecruit;	// Whether or not this side can recruit
}
-(WMLSideDetail *)init;
-(void)dealloc;
-(void)encodeWithCoder: (NSCoder *)coder;
- (id)initWithCoder:(NSCoder *)coder;
-(NSString *)getLeaderName;
-(void)setLeaderName:(NSString *)newName;
-(int)getLeaderID;
-(void)setLeaderTypeByID:(int)newID;
-(int)getGold;
-(void)setGold:(int)newGold;
-(NSString *)getType;
-(void)setType:(NSString *)newType;
-(float)getAgression;
-(void)setAgression:(float)newAgression;
-(BOOL)getCanRecruit;
-(void)setCanRecruit:(BOOL)newCanRecruit;
-(NSArray *)getRecruitTypes;
-(void)setSideDetailTeam:(int)newTeam;
-(int)getSideDetailTeam;
-(void)setRecruitTypes:(NSArray *)newSet;
-(void)exportSideDetailToFile:(FILE *)file withTeams:(NSMutableArray *)teams withIndex:(int)index withMap:(WMLMap *)map withDifficulty:(NSString *)diff withSide:(int)mySide withInfo:(NSMutableString *)advancedInfo;
@end
