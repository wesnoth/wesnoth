//
//  WNMap.h
//  Wesnoth Scenario Editor
//
//  Created by Marcus Phillips on Fri Mar 26 2004.
//  Copyright (c) 2004 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "WMLMapPoint.h"
#import "WMLTerrain.h"


@interface WMLMap : NSObject {
    int width;
    int height;
    NSMutableArray *mapData;	// mapdata holds rows each row is an array
}
-(WMLMap *)initWithSize: (int)xsize by: (int)ysize;
-(void)dealloc;
-(void)encodeWithCoder: (NSCoder *)coder;
- (id)initWithCoder:(NSCoder *)coder;
-(void)setWidth: (int)newWidth;
-(void)setHeight: (int)newHeight;
-(void)resizeTo: (int)xSize by: (int)ySize;
-(NSMutableArray *)getRow: (int)index;
-(int)getWidth;
-(int)getHeight;
-(void)setPointToTerrain:(int)idNo x:(int)x y:(int)y;
-(void)setPointToUnit:(int)idNo type:(int)newType side:(int)newSide x:(int)x y:(int)y;
-(void)clearUnitAtPointX:(int) xPos y:(int)yPos;
-(void)exportToFile: (NSString *)saveFile;
-(NSPoint)importFromFile: (NSString *)importFile;
-(void)updateUnitsWithLookup:(NSMutableArray *)unitConvert;
-(NSMutableString *)exportToString;
-(NSMutableString *)getUnitListForDifficulty:(NSString *)diff forSide:(int)side;
-(WMLMapPoint *)getMapPointAtX:(int)x Y:(int)y;
-(void)iterateSelector:(SEL)mySelector withObject:(id)myObject;
@end
