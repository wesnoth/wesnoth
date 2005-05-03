/* WNCustomDialogView */

#import <Cocoa/Cocoa.h>

@interface WNCustomDialogView : NSImageView
{
}
-(void)awakeFromNib;
-(NSDragOperation)draggingEntered:(id <NSDraggingInfo>)sender;
-(BOOL)performDragOperation:(id <NSDraggingInfo>)sender;
@end
