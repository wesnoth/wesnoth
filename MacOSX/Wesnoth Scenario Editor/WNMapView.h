/* WNMapView */

#import <Cocoa/Cocoa.h>

@interface WNMapView : NSView
{
	NSTextField *mapInfoTerrain;
	NSTextView *mapInfoCharacterList;
	NSTextView *mapInfoUnitList;
	
    int lastHexX;
    int lastHexY;
    NSImage *cursor;
    NSImage *scaledCursor;
	NSMutableString *infoUnits, *infoCharacters;
    NSTrackingRectTag mapRectTag;
    BOOL mousePressed;
    float lastZoomLevel;
}
- (BOOL)acceptsFirstResponder;
- (BOOL)becomeFirstResponder;
- (BOOL)resignFirstResponder;
- (void)awakeFromNib;
- (void)drawAt:(NSPoint)myPoint mapPoint:(WMLMapPoint *)myPoint zoomed:(BOOL)zoomed;
- (void)displayUnitsAtPoint:(NSPoint)point mapPoint:(WMLMapPoint *)mapPoint forDifficulty:(NSString *)diff;
- (BOOL)isFlipped;
- (void)mapTabSelected:(NSNotification *)notification;
- (void)mapRestoreTracking:(NSNotification *)notification;
- (void)mapChanged:(NSNotification *)notification;
- (void)mouseDown:(NSEvent *)event;
- (void)mouseUp:(NSEvent *)event;
- (void)mouseMoved: (NSEvent *)theEvent;
- (void)mouseDragged:(NSEvent *)theEvent;
- (void)mouseEntered: (NSEvent *)theEvent;
-(void)mapInfo:(NSNotification *)notification;
@end
