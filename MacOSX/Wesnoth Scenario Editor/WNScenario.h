//
//  WNScenario.h
//  Wesnoth Scenario Editor
//
//  Created by Marcus Phillips on Fri Mar 26 2004.
//  Copyright (c) 2004 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "WMLMap.h"


@interface WNScenario : NSObject {
    NSString *name;
    WMLMap *map;
    WNScenario *winScenario;
    WNScenario *loseScenario;
    WNScenario *continueScenario;
}

-(void)init;

@end
