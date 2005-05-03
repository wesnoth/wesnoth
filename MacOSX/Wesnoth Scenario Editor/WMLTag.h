//
//  WNWMLReader.h
//  Wesnoth Scenario Editor
//
//  Created by Marcus Phillips on Wed Mar 24 2004.
//  Copyright (c) 2004 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>


@interface WMLTag : NSObject {
    NSMutableArray *Children;
    NSMutableDictionary *Settings;
    NSString *wmlText;
    NSString *Tag;
    int Curpos;
    int Maxpos;
    int Lastpos;

}
-(BOOL)initWithFile: (NSString *)wmlFile setTag: (NSString *)thisTag;
-(BOOL)initWithString: (NSString *)wmlString setTag: (NSString *)thisTag;
-(NSString *)getLine;
-(int) childrenCount;
-(WMLTag *) getChildAtIndex:(int)cIndex;
-(int) settingsCount;
-(NSString *)getSettingFor:(NSString *)sKey;
-(NSString *)getTag;
-(NSArray *)getTagSettingKeys;
@end
