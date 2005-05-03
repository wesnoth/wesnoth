//
//  SelectionUtils.h
//  Wesnoth Scenario Editor
//  This is a collection of utilities to make handling list selections more transparent from 10.2 to 10.3+
//  At a later date the code is easily replacable to ensure compatability
//
//  Created by Marcus on Sat Apr 03 2004.
//  Copyright (c) 2004 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface SelectionUtils : NSObject {

}
+(NSArray *)selectionsFromView:(NSTableView *)myView;
+(void)selectionsToView:(NSTableView *)myView indexArray:(NSArray *)newSelections;
@end
