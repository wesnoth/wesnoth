//
//  WMLSide.h
//  Wesnoth Scenario Editor
//
//  Created by Marcus Phillips on Wed Mar 31 2004.
//  Copyright (c) 2004 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "WMLSideDetail.h";


@interface WMLSide : NSObject {
    NSString *name;
	NSMutableString *advancedInfo;
    NSMutableDictionary *SideLevels;
}
-(WMLSide *)init;
-(void)encodeWithCoder: (NSCoder *)coder;
- (id)initWithCoder:(NSCoder *)coder;
-(NSString *)getName;
-(void)setName:(NSString *)newName;
-(WMLSideDetail *)detailForKey:(NSString *)key;
-(void)exportSideToFile:(FILE *)file withTeams:teams withIndex:(int)index withMap:(WMLMap *)map withSide:(int)side;
-(void)setSideAdvancedInfo:(NSString *)newInfo;
-(NSMutableString *)getSideAdvancedInfo;
@end
