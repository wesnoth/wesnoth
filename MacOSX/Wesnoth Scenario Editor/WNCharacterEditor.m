#import "Wesnoth_Scenario_Editor_Prefix.h"
#import "WNCharacterEditor.h"

@implementation WNCharacterEditor
-(void)awakeFromNib
{
    [[NSNotificationCenter defaultCenter] addObserver:self
        selector:@selector(newCharacterCustomDialogImage:)
        name:@"newCharacterCustomDialogImage" object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self
        selector:@selector(characterListsReset:)
        name:@"characterListsReset" object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self
        selector:@selector(characterListReload:)
        name:@"campaignReset" object:nil];
}

- (IBAction)CharacterAISelect:(id)sender
{
    WNCharacters *tmpChar = [self characters];
    int curChar = [CharacterList selectedRow];
	[tmpChar setAIAtIndex:curChar to:[CharacterAI titleOfSelectedItem]];
}

- (IBAction)CharacterDeathActionSelect:(id)sender
{
    WNCharacters *tmpChar = [self characters];
    int curChar = [CharacterList selectedRow];
	[tmpChar setDeathActionAtIndex:curChar to:[sender titleOfSelectedItem]];

}

- (IBAction)CharacterDeathMessageUpdate:(id)sender
{
    WNCharacters *tmpChar = [self characters];
    int curChar = [CharacterList selectedRow];
    
    NSString *DeathMessage = [CharacterDeathMessage stringValue];
    [tmpChar setDeathMessageAtIndex: curChar to:DeathMessage];
}

- (IBAction)CharacterDialogImageSelect:(id)sender
{
}

- (IBAction)CharacterHitpointsSelect:(id)sender
{
    WNCharacters *tmpChar = [self characters];
    int curChar = [CharacterList selectedRow];

    [tmpChar setHPAtIndex: curChar to: [CharacterHP floatValue]];
}

- (IBAction)CharacterNew:(id)sender
{
    [[self characters] newCharacter];
    [CharacterList reloadData];
}

- (IBAction)CharacterRemove:(id)sender
{
}

- (IBAction)CharacterSelect:(id)sender
{
    [self refreshCharacterInfo];
}

- (IBAction)CharacterTrait1Select:(id)sender
{
    WNCharacters *tmpChar = [self characters];
    int curChar = [CharacterList selectedRow];
	[tmpChar setTrait:1 atIndex:curChar to:[CharacterTrait1 titleOfSelectedItem]];
}

- (IBAction)CharacterTrait2Select:(id)sender
{
    WNCharacters *tmpChar = [self characters];
    int curChar = [CharacterList selectedRow];
	[tmpChar setTrait:2 atIndex:curChar to:[CharacterTrait2 titleOfSelectedItem]];
}

- (IBAction)CharacterUnitSelect:(id)sender
{
    WNCharacters *tmpChar = [self characters];
    int curChar = [CharacterList selectedRow];
    int newUnitType = [sender selectedRow];
    [tmpChar setUnitTypeAtIndex: curChar to:newUnitType];
    [CharacterList reloadData];
}

- (IBAction)CharacterUseDialogImageSelect:(id)sender
{
}

-(WNCharacters *)characters
{
    return [[WNCampaign getMainCampaign] getCharacters];
}

-(void)refreshCharacterInfo
{
    int selectedChar = [CharacterList selectedRow];
    WNCharacters *myChar = [self characters];
    [CharacterAI selectItemWithTitle: [myChar getAIAtIndex: selectedChar]];
    [CharacterUnitList selectRow:[myChar getUnitTypeAtIndex: selectedChar] byExtendingSelection:NO];
    [CharacterDialogImage setImage:[myChar getDialogImageAtIndex: selectedChar]];
    [CharacterHP setFloatValue:[myChar getHPAtIndex: selectedChar]];
    [CharacterDeathMessage setStringValue:[myChar getDeathMessageAtIndex: selectedChar]];
    [CharacterDeathAction selectItemWithTitle: [myChar getDeathActionAtIndex: selectedChar]];
    [CharacterTrait1 selectItemWithTitle: [myChar getTrait:1 atIndex: selectedChar]];
    [CharacterTrait2 selectItemWithTitle: [myChar getTrait:2 atIndex: selectedChar]];
}

-(void)characterListsReset:(NSNotification *)notification
{
    [CharacterList selectRow:0 byExtendingSelection:NO];
    [self refreshCharacterInfo];
}

-(void)newCharacterCustomDialogImage:(NSNotification *)notification
{
    WNCharacters *myChar = [self characters];
    int selectedChar = [CharacterList selectedRow];
    [myChar setDialogImageAtIndex: selectedChar to:[notification object]];
}

-(void)characterListReload:(NSNotification *)notification
{
	[CharacterList reloadData];
	[CharacterUnitList reloadData];
}
@end
