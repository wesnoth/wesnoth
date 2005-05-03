#import "Wesnoth_Scenario_Editor_Prefix.h"
#import "WNCampaignIconView.h"

@implementation WNCampaignIconView
-(void)awakeFromNib
{
    [self registerForDraggedTypes:[NSArray arrayWithObjects:
        NSTIFFPboardType, NSPICTPboardType, NSFilenamesPboardType, nil]];
}

- (NSDragOperation)draggingEntered:(id <NSDraggingInfo>)sender 
{
    NSPasteboard *pboard;
    NSDragOperation sourceDragMask;

    sourceDragMask = [sender draggingSourceOperationMask];
    pboard = [sender draggingPasteboard];

    if ( [[pboard types] containsObject:NSColorPboardType] )
        if (sourceDragMask & NSDragOperationGeneric)
            return NSDragOperationGeneric;
            
    if ( [[pboard types] containsObject:NSFilenamesPboardType] )
        if (sourceDragMask & NSDragOperationCopy)
            return NSDragOperationCopy;
            
    return NSDragOperationNone;
}

-(BOOL)performDragOperation:(id <NSDraggingInfo>)sender 
{
    NSPasteboard *pboard;
    NSDragOperation sourceDragMask;
    NSImage *draggedImage = [NSImage alloc];

    sourceDragMask = [sender draggingSourceOperationMask];
    pboard = [sender draggingPasteboard];

    if ( [[pboard types] containsObject:NSTIFFPboardType] ) 
        {
            [draggedImage initWithPasteboard:pboard];
        } else if ( [[pboard types] containsObject:NSFilenamesPboardType] ) {
            NSArray *files = [pboard propertyListForType:NSFilenamesPboardType];
            [draggedImage initWithContentsOfFile:[files objectAtIndex:0]];
        }
    if (draggedImage != nil)
        {
        [draggedImage retain];
        [self setImage:draggedImage];
        NSDictionary *tmpDict = [NSDictionary dictionaryWithObject:draggedImage forKey:@"image"];
        [tmpDict retain];
        [[NSNotificationCenter defaultCenter] 
            postNotificationName:@"newCampaignIcon" object:self userInfo:tmpDict];
        }
    return YES;
}
@end
