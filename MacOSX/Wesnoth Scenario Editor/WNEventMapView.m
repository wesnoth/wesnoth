#import "Wesnoth_Scenario_Editor_Prefix.h"
#import "WNEventMapView.h"
#import "WNEventLookup.h"

@implementation WNEventMapView
-(void)awakeFromNib
{
    [[NSNotificationCenter defaultCenter] addObserver:self
            selector:@selector(eventSelected:)
            name:@"eventSelected" object:nil];
	eventImage = [[NSImage alloc] initWithContentsOfFile:[[[NSBundle mainBundle] resourcePath] stringByAppendingPathComponent: @"eventmarker.png"]];
    scaledEventImage = [eventImage copy];
	[super awakeFromNib];
}

- (void)drawRect:(NSRect)rect
{    
	int scaledX=70, scaledY=70;
	NSSize scaledSize = {70,70};
	
	float zoomLevel = [WNCampaign getMapZoom];
	if (zoomLevel !=1.0)
		{
		scaledX = 70*zoomLevel;
		scaledY = 70*zoomLevel;
		scaledSize.width = scaledX;
		scaledSize.height = scaledY;
		}
	if (zoomLevel != lastZoomLevel)
		{
		[scaledEventImage autorelease];
		scaledCursor = [eventImage copy];
		[scaledEventImage setScalesWhenResized:YES];
		[scaledEventImage setSize:scaledSize];
		[scaledEventImage retain];
		}
	[super drawRect:rect];
}

-(void)drawAt:(NSPoint)myPoint mapPoint:(WMLMapPoint *)myMapPoint zoomed:(BOOL)zoomed
{
	[super drawAt:myPoint mapPoint:myMapPoint zoomed:zoomed];   // draw normally
	NSPoint eventPoint = [WNEventLookup getMapLoc]; // find out the event position
	NSPoint hexPoint = [HexUtils pixelFromFlatHexX: eventPoint.x y:eventPoint.y];
	//fprintf(stderr, "x1:%g, x2:%g, y1:%g, y2:%g\n", myPoint.x, hexPoint.x, myPoint.y, hexPoint.y);
	if ((hexPoint.x == myPoint.x)&&(hexPoint.y == myPoint.y))
		{
		fprintf(stderr, "Drawing event icon\n");
		[scaledEventImage compositeToPoint: myPoint operation:NSCompositeSourceOver];
		}
}

-(void)mouseDown:(NSEvent *)event
{
	NSPoint eventLocation = [event locationInWindow];
	NSPoint localPos = [self convertPoint:eventLocation fromView:nil];
	fprintf(stderr,"*** Click in event map: %f,%f\n",localPos.x,localPos.y);
	NSPoint hexPos = [HexUtils flatHexFromX: localPos.x y: localPos.y];
	hexPos.y += 1;
	hexPos.x += 1;
//	int x = hexPos.x;
//	int y = hexPos.y;
	NSEventType type = [event type];
	int modifiers = [event modifierFlags];
	if ((modifiers & NSControlKeyMask)==NSControlKeyMask) type = NSRightMouseDown;
	switch(type)
		{
		case NSLeftMouseDown:
//			fprintf(stderr, "Placing event at x:%d, y:%d, hexX:%g, hexY:%g\n", x,y,hexPos.x, hexPos.y);
			[WNEventLookup setMapLoc:hexPos];
			[[WNEventLookup getCurrentEvent] setEventMapLoc:hexPos];
			break;
		case NSRightMouseDown:
			break;
		default:
			break;
		}
	[self display];
}

-(void)eventSelected:(NSNotification *)notification
{
	fprintf(stderr, "New event selected so redraw\n");
	[self display];
}

@end
