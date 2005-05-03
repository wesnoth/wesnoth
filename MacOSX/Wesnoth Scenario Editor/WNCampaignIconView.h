/* WNCampaignIconView */

#import <Cocoa/Cocoa.h>

@interface WNCampaignIconView : NSImageView
{
}
-(void)awakeFromNib;
-(NSDragOperation)draggingEntered:(id <NSDraggingInfo>)sender;
-(BOOL)performDragOperation:(id <NSDraggingInfo>)sender;
@end
