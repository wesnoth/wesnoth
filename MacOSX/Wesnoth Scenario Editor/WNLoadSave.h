/* WNLoadSave */

#import <Cocoa/Cocoa.h>

@interface WNLoadSave : NSObject
{
	IBOutlet NSProgressIndicator *loadProgress;
	IBOutlet NSProgressIndicator *saveProgress;
}
- (IBAction)export:(id)sender;
- (IBAction)load:(id)sender;
- (void)doLoadFile:(NSString *)fileToLoad;
- (IBAction)revert:(id)sender;
- (IBAction)save:(id)sender;
- (IBAction)saveAs:(id)sender;
- (NSWindow *)startSaveWindow;
- (void)stopSaveWindow;
@end
