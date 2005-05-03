/* WNCharacterEditor */

#import <Cocoa/Cocoa.h>
#import "WNCharacters.h"

@interface WNCharacterEditor : NSObject
{
    IBOutlet NSPopUpButton *CharacterAI;
    IBOutlet NSPopUpButton *CharacterDeathAction;
    IBOutlet NSTextField *CharacterDeathMessage;
    IBOutlet NSImageView *CharacterDialogImage;
    IBOutlet NSButton *CharacterDialogImageSwitch;
    IBOutlet NSSlider *CharacterHP;
    IBOutlet NSTableView *CharacterList;
    IBOutlet NSPopUpButton *CharacterTrait1;
    IBOutlet NSPopUpButton *CharacterTrait2;
    IBOutlet NSTableView *CharacterUnitList;
}
- (void)awakeFromNib;
- (IBAction)CharacterAISelect:(id)sender;
- (IBAction)CharacterDeathActionSelect:(id)sender;
- (IBAction)CharacterDeathMessageUpdate:(id)sender;
- (IBAction)CharacterDialogImageSelect:(id)sender;
- (IBAction)CharacterHitpointsSelect:(id)sender;
- (IBAction)CharacterNew:(id)sender;
- (IBAction)CharacterRemove:(id)sender;
- (IBAction)CharacterSelect:(id)sender;
- (IBAction)CharacterTrait1Select:(id)sender;
- (IBAction)CharacterTrait2Select:(id)sender;
- (IBAction)CharacterUnitSelect:(id)sender;
- (IBAction)CharacterUseDialogImageSelect:(id)sender;
-(WNCharacters *)characters;
-(void)refreshCharacterInfo;
-(void)newCharacterCustomDialogImage:(NSNotification *)notification;
-(void)characterListReload:(NSNotification *)notification;
@end
