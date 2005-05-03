/* WNEventMapView */

#import <Cocoa/Cocoa.h>
#import "WNMapView.h"

@interface WNEventMapView : WNMapView
{
	NSImage *eventImage;
	NSImage *scaledEventImage;
}
-(void)awakeFromNib;
-(void)drawRect:(NSRect)rect;
-(void)mouseDown:(NSEvent *)event;
-(void)eventSelected:(NSNotification *)notification;
@end
