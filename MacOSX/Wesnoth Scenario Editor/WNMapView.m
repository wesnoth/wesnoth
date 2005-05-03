#import "Wesnoth_Scenario_Editor_Prefix.h"
#import "WNCampaign.h"
#import "WNMapView.h"
#import "WNUnits.h"
#import "WNTerrains.h"
#import "WMLMap.h"
#import "WMLMapPoint.h"
#import "HexUtils.h"
#import "WNPreferences.h"
#import "WNMapTabViewDelegate.h"

@implementation WNMapView
float unitOpacity;

- (BOOL) acceptsFirstResponder
{
    // We want this view to be able to receive key events.
    return YES;
}

- (BOOL)becomeFirstResponder 
 {
    [[self window] setAcceptsMouseMovedEvents: YES]; 
    return YES; 
 } 
  
 - (BOOL)resignFirstResponder 
 { 
    [[self window] setAcceptsMouseMovedEvents: NO]; 
    return YES; 
 } 
  

-(void)awakeFromNib
{   // Add notification for Map Changes
    [[NSNotificationCenter defaultCenter] addObserver:self
            selector:@selector(mapChanged:)
            name:@"mapChanged" object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self
            selector:@selector(mapRestoreTracking:)
            name:@"mapRestoreTracking" object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self
            selector:@selector(mapTabSelected:)
            name:@"mapTabSelected" object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self
            selector:@selector(mapInfo:)
            name:@"mapInfo" object:nil];
        
    [[self window] setAcceptsMouseMovedEvents: YES]; 
    if ([[self window] acceptsMouseMovedEvents]) {fprintf(stderr,"window now acceptsMouseMovedEvents\n");} 
	cursor = [[NSImage alloc] initWithContentsOfFile:[[[NSBundle mainBundle] resourcePath] stringByAppendingPathComponent: @"mapcursor.png"]];
    scaledCursor = [cursor copy];
	[cursor retain];
	[scaledCursor retain];
	mapRectTag = [self addTrackingRect: [self bounds] owner:self userData:nil assumeInside:NO];
//    NSWindow *  window = [self window];
//    [window setAcceptsMouseMovedEvents: YES];
//    [window acceptsFirstResponder: YES];
//    [window makeFirstResponder: self];
	infoUnits = [[[NSMutableString alloc] init] retain];
	infoCharacters = [[[NSMutableString alloc] init] retain];
	mousePressed = 0;
}


- (id)initWithFrame:(NSRect)frameRect
{
	[super initWithFrame:frameRect];
	return self;
}

- (void)drawRect:(NSRect)rect
{    
	WMLMapPoint *tmpPoint = nil;
	WMLMap *tmpMap = nil;
	NSMutableArray *tmpRow = nil;
	NSPoint myPoint;
	int scaledX=70, scaledY=70;
	NSSize scaledSize = {70,70};
	unitOpacity = [WNPreferences getUnitOpacity];
	
    NSRect mapBounds = [self bounds];
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
		[scaledCursor autorelease];
		scaledCursor = [cursor copy];
		[scaledCursor setScalesWhenResized:YES];
		[scaledCursor setSize:scaledSize];
		[scaledCursor retain];
		lastZoomLevel = zoomLevel;
		}
	
	// BG = Black
        [[NSColor blackColor] set];
        [NSBezierPath fillRect:mapBounds];
	NSPoint hexStart = [HexUtils flatHexFromX:rect.origin.x y:rect.origin.y];
	NSPoint hexEnd = [HexUtils flatHexFromX:(rect.origin.x+rect.size.width) y:(rect.origin.y+rect.size.height)];
	int hexStartX = hexStart.x - 1;
	int hexStartY = hexStart.y - 1;
	int hexEndX = hexEnd.x + 1;
	int hexEndY = hexEnd.y + 1;
	if (hexStartX < 0) hexStartX = 0;
	if (hexStartY < 0) hexStartY = 0;
	
	int loopX=0, loopY=0, maxTile;
	
	tmpMap = [WNCampaign getActiveMap];
	for (loopY = hexStartY ;loopY<=hexEndY; loopY++)
		{
		tmpRow = [tmpMap getRow:loopY];
		if (tmpRow != nil)
			{
			maxTile = [tmpRow count];
			for (loopX = hexStartX ;((loopX<=hexEndX)&&(loopX<maxTile)); loopX++)
				{
				tmpPoint = [tmpRow objectAtIndex:loopX];
				myPoint = [HexUtils pixelFromFlatHexX:loopX y:loopY];
				myPoint.y+=scaledY;
				if (zoomLevel == 1.0)
					[self drawAt: myPoint mapPoint:tmpPoint zoomed:NO];
				else
					[self drawAt: myPoint mapPoint:tmpPoint zoomed:YES];
				}
			}
		}
        myPoint = [HexUtils pixelFromFlatHexX: lastHexX y:lastHexY];
        myPoint.y+=scaledY;
		if (zoomLevel == 1.0)
			[cursor compositeToPoint: myPoint operation:NSCompositeSourceOver];
		else
			[scaledCursor compositeToPoint: myPoint operation:NSCompositeSourceOver];
    }

-(void)drawAt:(NSPoint)myPoint mapPoint:(WMLMapPoint *)myMapPoint zoomed:(BOOL)zoomed
{
    if (zoomed == NO)
        {
        [[[WNTerrains terrainAtIndex:[myMapPoint terrainID]] image]
            compositeToPoint: myPoint operation:NSCompositeSourceOver];
        }else{
        [[[WNTerrains terrainAtIndex:[myMapPoint terrainID]] scaledImage]
            compositeToPoint: myPoint operation:NSCompositeSourceOver];
        }

    if ([WNPreferences getPlaceEasy]==YES) [self displayUnitsAtPoint:myPoint mapPoint:myMapPoint forDifficulty:@"Easy"];
    if ([WNPreferences getPlaceNormal]==YES) [self displayUnitsAtPoint:myPoint mapPoint:myMapPoint forDifficulty:@"Normal"];
    if ([WNPreferences getPlaceHard]==YES) [self displayUnitsAtPoint:myPoint mapPoint:myMapPoint forDifficulty:@"Hard"];
}

- (void)displayUnitsAtPoint:(NSPoint)point mapPoint:(WMLMapPoint *)mapPoint forDifficulty:(NSString *)diff
{
	NSNumber *tmpNum = [mapPoint getUnitIDForDifficulty:diff];
	NSNumber *tmpType = [mapPoint getUnitTypeForDifficulty:diff];
	if (tmpNum != nil)
		{
		switch([tmpType intValue])
			{
			case 1:
				[[WNUnits scaledImageAtIndex:[tmpNum intValue]] 
					compositeToPoint: point operation:NSCompositeSourceOver fraction:unitOpacity];
				break;
			case 2:
				[[WNUnits scaledImageAtIndex: 
					[[[WNCampaign getMainCampaign] getCharacters] getUnitTypeAtIndex:[tmpNum intValue]]]
						compositeToPoint: point operation:NSCompositeSourceOver fraction:unitOpacity];
				break;
			default:
				fprintf(stderr, "ERROR - Reached Default in displayUnitsAtPoint\n");
				break;
			}
		}

}

- (BOOL)isFlipped
{
    return YES;
}

-(void)mapTabSelected:(NSNotification *)notification
{
    [self removeTrackingRect: mapRectTag];
    mapRectTag = [self addTrackingRect: [self bounds] owner:self userData:nil assumeInside:NO];
    [[self window] makeFirstResponder: self];
    [[self window] setAcceptsMouseMovedEvents: NO]; 
    [[self window] setAcceptsMouseMovedEvents: YES]; 
}

-(void)mapRestoreTracking:(NSNotification *)notification
{
    [[self window] makeFirstResponder: self];
    [[self window] setAcceptsMouseMovedEvents: NO]; 
    [[self window] setAcceptsMouseMovedEvents: YES]; 
}


-(void)mapChanged:(NSNotification *)notification
{
	WMLMap *tmpMap = [WNCampaign getActiveMap];
	NSSize newSize;
	
	NSPoint tmpPoint = [HexUtils pixelFromFlatHexX: ([tmpMap getWidth] + 1) y:([tmpMap getHeight] + 1)];
	newSize.width = tmpPoint.x;
	newSize.height = tmpPoint.y;
	fprintf(stderr, "Setting Map Size To :%f,%f\n", newSize.width, newSize.height);
	[self setFrameSize: newSize];
	[self display];
	[[self superview] display];
}

-(void)mouseDown:(NSEvent *)event
{
	NSPoint eventLocation = [event locationInWindow];
	NSPoint localPos = [self convertPoint:eventLocation fromView:nil];
	fprintf(stderr,"*** Click PointFrame: %f,%f\n",localPos.x,localPos.y);
	NSPoint hexPos = [HexUtils flatHexFromX: localPos.x y: localPos.y];
	WMLMap *tmpMap = [WNCampaign getActiveMap];
	int xPos = hexPos.x;
	int yPos = hexPos.y;
        NSMutableString *clickType = [WNMapTabViewDelegate getTab];
        NSEventType type = [event type];
        int modifiers = [event modifierFlags];
        if ((modifiers & NSControlKeyMask)==NSControlKeyMask) type = NSRightMouseDown;
        switch(type)
            {
            case NSLeftMouseDown:
                mousePressed = YES;
                if ([clickType isEqualTo:@"Terrain"])
                    [tmpMap setPointToTerrain:[WNPreferences getTerrainID] x:xPos y:yPos];
                if ([clickType isEqualTo:@"Units"])
                    [tmpMap setPointToUnit:[WNPreferences getUnitID] 
                        type:1 side:[WNPreferences getUnitSide] x:xPos y:yPos];
                if ([clickType isEqualTo:@"Characters"])
                    [tmpMap setPointToUnit:[WNPreferences getCharacterID]
                        type:2 side:[WNPreferences getUnitSide] x:xPos y:yPos];
                break;
            case NSRightMouseDown:
                [tmpMap clearUnitAtPointX: xPos y:yPos];
                break;
            default:
                break;
            }
	[self display];
}

-(void)rightMouseDown:(NSEvent *)event
{
    [self mouseDown: event];
}

-(void)rightMouseUp:(NSEvent *)event
{
}

-(void)mouseUp:(NSEvent *)event
{
	mousePressed = NO;
}

- (void)mouseMoved: (NSEvent *)theEvent
{
//	fprintf(stderr, "tracking mouse\n");
	NSPoint location = [self convertPoint: [theEvent locationInWindow] fromView: nil];
	NSPoint hexLoc = [HexUtils flatHexFromX: location.x y: location.y];
	int tmpX = hexLoc.x;
	int tmpY = hexLoc.y;
	if ((tmpX != lastHexX)||(tmpY != lastHexY))
		{
		lastHexX = tmpX;
		lastHexY = tmpY;
		//[self display];
		if (mousePressed) [[WNCampaign getActiveMap] setPointToTerrain: [WNPreferences getTerrainID] x:tmpX y:tmpY];
		[[NSNotificationCenter defaultCenter] postNotificationName:@"mapChanged" object:self];
		NSMutableString *clickType = [WNMapTabViewDelegate getTab];
		if ([clickType isEqualTo:@"Info"])
			{
			WMLMapPoint *tmpPoint = [[WNCampaign getActiveMap] getMapPointAtX: tmpX Y:tmpY];
			[mapInfoTerrain setStringValue:[[WNTerrains terrainAtIndex:[tmpPoint terrainID]] name]];
			[tmpPoint mapPointUnitInfoToString:infoUnits characterTo:infoCharacters characters:[[WNCampaign getMainCampaign]getCharacters]];
			[mapInfoUnitList setString:infoUnits];
			[mapInfoCharacterList setString:infoCharacters];
			}
		}
}

- (void)mouseDragged:(NSEvent *)theEvent
{
	[self mouseMoved: theEvent];
}

-(void)mouseEntered: (NSEvent *)theEvent
{
    fprintf(stderr, "Mouse entered\n");
    [[self window] makeFirstResponder: self];
    [[self window] setAcceptsMouseMovedEvents: NO]; 
    [[self window] setAcceptsMouseMovedEvents: YES]; 
}

-(void)mapInfo:(NSNotification *)notification
{
    NSDictionary *tmpDict = [notification userInfo];
	mapInfoTerrain = [tmpDict objectForKey:@"Terrain"];
	mapInfoUnitList = [tmpDict objectForKey:@"Unit"];
	mapInfoCharacterList = [tmpDict objectForKey:@"Character"];
}
@end
