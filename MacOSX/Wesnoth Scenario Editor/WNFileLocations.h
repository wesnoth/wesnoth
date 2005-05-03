//
//  WNFileLocations.h
//  Wesnoth Scenario Editor
//
//  Created by Marcus Phillips on Thu Mar 25 2004.
//  Copyright (c) 2004 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>


@interface WNFileLocations : NSObject {
}

+(BOOL)init;
+(NSString *)dataLoc;
+(NSString *)unitDataLoc;
+(NSString *)imageLoc;
+(NSString *)terrainImageLoc;
+(void)setSaveLocation:(NSString *)saveLoc;
+(NSString *)getSaveLocation;
+(void)setExportDataLocation:(NSString *)newLoc;
+(NSMutableString *)getExportDataLocation;
+(void)setExportUnitsLocation:(NSString *)newLoc;
+(NSMutableString *)getExportUnitsLocation;
+(void)setExportScenariosLocation:(NSString *)newLoc;
+(NSMutableString *)getExportScenariosLocation;
+(void)setExportImagesLocation:(NSString *)newLoc;
+(NSMutableString *)getExportImagesLocation;
@end
