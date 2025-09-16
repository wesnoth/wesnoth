# Heir 2 the Throne

## (Heir to the Throne Revision)


Hello! This is a WIP HttT revision for Battle for Wesnoth. If you're interested in collaborating, please reach out to Dalas via the Wesnoth discord or forum.

**Status**: Finished, released for review and feedback

## CONTRIBUTORS - READ THIS FIRST

- Right now, let's focus on writing the scenarios for Phase 1.
- Read through this document. Some scenarios have more info, some have less.
	- These are just my thoughts, not necessarily a must-have.
- Pick out a scenario you're interested in working on. Let us know, then update this document to assign yourself.
- Write the scenario WML, following the guidelines in this document, then make a PR so we can all try it out.

## DESIGN OVERVIEW

<details>
<summary>Click to expand</summary>

### THE OVERWORLD

- after each scenario the player gets sent to the "overworld", an in-game map of the great continent
- the player chooses their next scenario via the overworld. Only some scenarios are available at a time, depending on the current "phase"
- after playing a certain number of scenarios (varies by difficulty), the player is forcibly advanced to the next phase
- each scenario has a preview, including:
	- difficulty (1-3 skulls, or 4 skulls for the final scenario, etc)
	- reward: gold carryover (no/low/high)
	- reward: new recruits
	- reward: loyal companions
	- reward: other
- after playing each scenario, the season of the year changes. Spring, Summer, Fall, Winter. Summer/Winter seasons result in battle scenarios with longer/shorter daytime.

### REWARDS

- Konrad starts with 0 recruits. In Phase 1 and Phase 2, he builds up his recruit list depending on which scenarios he chooses to play.
- many Phase 1 and Phase 2 scenarios also give loyal companions as rewards, who're auto-recalled in each scenario and have custom dialogue.
- Phase 3 scenarios focus on hindering Asheviere. For example, completing "Test of the Clans" might get rid of Asheviere's cavalry in the final confrontation.
	- possibly also scenarios that buff up Li'sar?
- rewards can be given at the beginning of the scenario, at the end, in the overworld, or anywhere in between

</details>

### NARRATIVE DESIGN
<details>
<summary>Click to expand</summary>

#### PLOT

##### Phase 1
- Konrad and Delfador are driven from the Aethenwood by Asheviere's human armies
- Konrad and Delfador complete 1 scenario together (limited choice here; only Exodus, Blackwater, or Pearls)
- Delfador leaves to help in the defense of Elensefar
- player plays 2-4 more phase 1 scenarios (varies by difficulty)
- Elensefar falls. Delfador returns. Phase 1 scenarios remain accessible throughout phase 2

###### Phase 2
- Li'sar follows Konrad north across the great river. Both race to get the sceptre
- Li'sar's actions show she's a decent person, unlike her mother. We get some insight into her relationship with Asheviere.
- player plays 2-3 more phase 2 scenarios (varies by difficulty)
- Li'sar gets close to the sceptre. Konrad chases after her.

----ONCE WE FINISH PHASE 2, LET'S STOP, POLISH, AND RELEASE TO GET FEEDBACK BEFORE MOVING FURTHER ON----

###### Phase 2.5
- Konrad gets the sceptre first. Delfador collapses the cave behind them, (presumably) killing Li'sar. After this point, previous scenarios become unreachable.
- surprise, Li'sar's back. Her army is much weaker
- Konrad has a change of heart, and refuses to fight Li'sar. He joins her / is taken prisoner / something like that
- Delfador worries Konrad will be executed once Li'sar reaches Weldyn. Delfador goes off to fight Asheviere by himself
	- or maybe he goes off for some other reason?
- Li'sar and Konrad fight Asheviere's ally Iliah'al (details TBD after AoA releases). They bond.
	- Li'sar is always a separate (genuinely helpful) allied side, not directly under the player's control

##### Phase 3
- we learn that Delfador (or maybe instead Kaylan/Ethiliel/someone?) has been captured by Asheviere, and will be executed in X months (not immediately, because Asheviere is trying to lure Konrad)
- player plays 1-2 more phase 3 scenarios (varies by difficulty)
- Konrad and Li'sar confront Asheviere at Weldyn. Delfador is rescued; Asheviere dies/flees/etc. Hooray!

#### CHARACTERS

Konrad (phase 1)
- sees the good right in front of him (contrast this with Delfador)
- calls people "Master", "Sir", "Uncle", "Ma'am", "Miss", "Aunt". Not every time, but occasionally.
- humans are foreign and weird. He understands elves, not humans.
- naive but good-hearted. Might get tricked or taken advantage of. Think samwise gamgee manners
- through the course of the campaign, he starts to lean into being the heir a bit more: a symbol, a way to rally people for good
	- then when he learn he's not the real heir, he has a momentary crisis of faith
	- Li'sar and him talk it through, possibly referencing Asheviere
- as he hits phase 2, phase 3, he starts to get more assertive and confident.

Delfador
- for the greater good; i.e. needs of the many/few (contrast this with Konrad)
- calls Konrad "young prince", "master Konrad", etc. Or just "Konrad"

Li'sar
- stoic, proud, relentless.
- prefers tricks, ambushes, and "clever" tactics, but doesn't shy away from a straight-up fight
- calls Asheviere "mother", like Eldred did. But their relationship is quite distant
- she's a decent person. For example, she wouldn't fight unarmed civilians and would try to evacuate them before a military battle in their village
	- but vs a military opponent, she'd hold back absolutely nothing and would have no problem being extremely brutal
- disapproves of Asheviere's orcs
- Li'sar recruits small, very well-trained armies of primarily HI/Fencers/Bowmen. We represent this in-game with special abilities (replacing Initiative/First-Strike)
	- Fencers get Backstab
	- Bowmen get Marksman
	- HI get March: "This unit ignores movement cost on most terrain."

</details>

## SCENARIO DESIGN

<details>
<summary>Click to expand</summary>

### GLOBAL INCLUDES

- to make it easy to tweak Konrad, please define side 1 with `{KONRAD_SIDE}`. `FOG=yes` and `SHROUD=yes` optional. Also `GOLD=#` (defaults to 75 in the campaign's first half, 125 in the second half)
- to ensure your scenario's ToD reflects the overworld, use `{SCHEDULE_DYNAMIC_DAY}`, or `{SCHEDULE_DYNAMIC_NIGHT}`
	- the first turn of each macro will always be respectively morning/midday or firstwatch/midnight (depending on the season)
	- both macros take an optional `OFFSET` argument, to delay morning/midday or firstwatch/midnight
- to ensure all companions get recalled properly, please use `{RECALL_KONRAD_AND_COMPANIONS $x $y}`
	- companions need custom dialogue in many scenarios, to help them feel alive!
- for recurring characters or NPCs, define them in unified_characters.cfg, add them to GLOBAL__AUTOSTORE_NPCs, and use UNSTORE_NPC to recall them

### DIFFICULTY

Easy should be nigh-unloseable. Nightmare should be seriously difficult even for a veteran player, requiring careful strategy and probably several restarts (e.g. highest difficulty of EI/TDG)
- assume the player is coming into the scenario with suitable units; i.e. don't balance Gryphon Mountain around massed Merfolk
- guidelines for 1-skull, 2-skull, and 3-skull difficulties can be found in _main.cfg. Use the diff1, diff2, diff3, etc events (or equivalent armies) to playtest

Recommended difficulty modifiers:
- Easy:      1x enemy strength,  70% xp to level, 12 scenarios
- Normal:    2x enemy strength, 100% xp to level, 12 scenarios
- Hard:      4x enemy strength, 100% xp to level, 11 scenarios
- Nightmare: 4x enemy strength, 100% xp to level, 8 scenarios

### XP ALLOCATION

Leveling units is fun! I want players to be able to continue to level units throughout the campaign, instead of maxing-out their army halfway through. With this in mind, please try to restrict available XP.
- small player armies; 50-75 initial gold (plus carryover) is a good guideline for the average scenario
- use lower-level enemies where possible. 1 Warlord is easy to farm for XP; 3 Grunts are much harder; 6 Goblins even more so.
- don't include a large amount of villages, unless carryover gold is the scenario's primary reward
- enemies should retreat and regroup where appropriate, instead of trickling. See EI's Xenophobia for an example.
- end scenarios sooner rather than later. If there's no threat after the first 10 turns, don't ask the player to survive for 20.

### AI DESIGN

- use `{SILENTLY_LIMIT_LEADER_MOVES}` to prevent AI leaders from running too far from their keeps. This macro provides a balance between the over-aggressive default behavior and the too-passive `passive_leader=yes`
- where applicable, reminder to have AI retreat and regroup instead of trickling. See EI's Xenophobia for an example.
- where applicable, AI should be less aggressive at unfavorable ToD (regardless of the player's favorable ToD). See EI's Xenophobia for an example.
- where applicable, if an AI side is defending, please use proper defensive AI with `[avoid]` and similar. See TDG's "Ring of Swords" for an example.
- if you have AI allies, I suggest scaling their gold so they always feel useful. For example, if my ally is 1/2 as strong as the enemy on Easy, they could be 1/2 as strong on Nightmare too.
- for consistency, I usually give AI sides a very very small general recruit list (e.g. just Spearman), and then allow for a few additional units with {LIMIT_CONTEMPORANEOUS_RECRUITS} (e.g. 0-2 Bowmen, 0-2 Cavalrymen)
	- this is also a good way to let the AI recruit a couple higher-level units (e.g. Javelineer, Pikeman) without making the entire side easy to farm for XP
- when creating AI guards, I suggest using MAIs instead of status=guardian. For example, `{ZONE_GUARDIAN 14 14 x,y,radius=11,12,2}`. This also lets you sync up multiple guards so they fight together instead of being lured out 1-by-1.
	- some campaigns use loyal icons for guards, some campaigns don't. For consistentcy, let's not use loyal icons for guards in HttT

### MAPS
credit yourself at the top of each scenario, e.g: # scenario by Dalas
create variants for autumn and winter (if applicable), and use the use the `MAP_DYNAMIC` macro to load them
	e.g. {MAP_DYNAMIC 02_Flight_of_the_Elves}
	with files 02_Flight_of_the_Elves.map, 02_Flight_of_the_Elves-autumn.map, 02_Flight_of_the_Elves-winter.map

### TEAMS AND COLORS

(not mandatory, but let's try to stay more-or-less consistent please)
- Konrad: blue
- Asheviere/Army: wesred
- Li'sar: lisarcolor
- Elves: brightgreen
- Dwarves: brown
- Blackcrests: black (these guys use lots of shamans)
- Whitefangs: white (these guys use lots of goblins and goblin riders)
- Undead: white
- Drakes: orange

### COMPANIONS

Each companion has a personality and a quest - a specific scenario that provides a special bonus if they get there.
Maybe the quest reward is automatic (just complete the scenario), or maybe there's some special bonus objective.
Quest rewards should be minor bonuses, not major rewards. Perhaps you get a bonus ability, or maybe you get the option to "trade in" your companion for some more significant bonus.

##### Moremirmu
```
- warrior monk. Someone who'll blast away zombies while quoting scripture (no specific religions plz; neither IRL ones nor Wesnoth ones)
"For at the end of days, the texts prophesy a great destruction, yea, the sun itself shall rain down from the heavens as sulfur and fire!"
"And lo! The unclean shall be smitten down, and lo! There shall be peace eternal for all men."
"But until then, ’tis up to I to do the smiting! [attacks some undead]"
- quest: bring him to Valley of Death and rescue his home.
```

##### Ulfdain:
```
- ulfserker. Loyal and healthy.
- speaks in a... interesting manner. Takes offense easily.
- ran away from Knalga when he was a child. Now the Dwarven Doors have fallen, Knalga is under siege, and he has no idea what happened to his family.
"C’mere and try it, ye yellow-bellied, cow-handed bluderbuss of a princess! Jus’ gimme the chance an’ I’ll rip that head right off yer ninnyhammerin’ neck, ye white-livered gasser!"
"Come an’ git some, ye nigmenog bolting-hutch of beastliness! Ah dare ye to square up an’ fight me, dwarf-to-dwarf!"
- quest: bring him back to West Knalga, and reclaim his father's bloodaxe.
```

##### Harper:
```
- it's Harper (from the Liberty mainline campaign)
- should we give her personality some kind of gimmick to help her stand out?  My memory on Liberty may no longer be accurate - I haven't played it since before the gender change
- quest: Halstead. No reward, but Harper comments on the battle
- quest: Dalben. Harper scrounges up a small amount of gold
- quest: Dwarven Doors. Harper gets Uncle Somf as another companion.
```

##### Chantal:
```
- Anekron's story
- quest: what? Lintanir maybe? Maybe no quest at all?
```

##### Jeniver:
```
- Custom unit, loyal and intelligent.
- alchemist and researcher. Inquisitive and curious, but also a little naive and ditzy.
- starts L3 so I only need to make one sprite. Throws smoke bombs (smoke) and uses a blowgun (poison). No melee. "Transmutation" ability, increasing your gold income.
- portrait: https://github.com/wtactics/art/blob/master/WT/Potion%20Making/Potion%20Making.png
```

##### Dosh:
```
- Troll. "Secret" companion: he normally guards the Ford of Abez and charges Konrad gold every time you want to cross
- if you pay him enough gold, he'll give up his toll job and join you
- "You ok in da head, softskin? Das one-hunnerd fifty gold you done paid me. How much you really got left?"
```

### "ADVISOR" DIALOGUE

- when something game-relevant needs to be said, please use `[companion_message]`
- this is perfect for "advisor" type dialogue with uncertain companions
```ini
	[companion_message]
		message_Delfador=_"We must make haste if we wish to save that warrior, Konrad! I sense magical power in him, but one man alone will not prevail against the hordes of undead that roam this peninsula."
		message_Kalenz=_"We must make haste if we wish to save that warrior, Konrad! I sense magical power in him, but one man alone will not prevail against the hordes of undead that roam this peninsula."
		message_Harper=_"20 against 1 is pretty long odds, Konrad. If we’re goin’ to help that warrior before he’s overwhelmed, we’d better hurry things up."
		message_Ulfdain=_"20-’gainst-1 fer that warrior, now them’s long odds e’en if he were a dwarf! Tha’ long-staff sixpenny sorcerer’ll be having a change o’ heart — literally! — once we’re through with ’im!"
		message_Jeniver=_"Oh, oh my. That magical warrior looks rather, err, powerful, doesn’t he? But that’s also a lot, really quite many undead surrounding him. We’d better hurry if we want to save him."
		message_Dosh=_"Dat holy man be lookin’ mighty strong, but Dosh is thinking there’s too many o’ dem undead e’en for him. We’d better be helpin’ him out, Konrad."
		fallback_Konrad=_"We’d better move quickly and help fight! That warrior looks powerful, but the necromancer has the advantage of numbers."
	[/companion_message]
```

### BACKSTORY/QUEST DIALOGUE

- 0-2 times in each scenario, try to include an opportunity for backstory/quest interactions between Konrad and his companions
- 1 time per scenario is best
- to play the dialogue, fire the "say_smalltalk" event
	- this event will check your existing companions, and play some backstory- or quest-related dialogue for one of them, depending on what's previously been said
	- Dalas is assigned to handle implementing this event

### OTHER

- if your scenario has a time limit, please include at least a basic "time over" cutscene as well as "we're running out of time" dialogue some turns beforehand
	- instead of using `name=time over`, trigger this event on `side 1 turn {SCENARIO_TURN_LIMIT} end`, so that we don't waste the player's time when they've already lost
- please include an achievement in every scenario! Could be a difficult challenge, a hint towards some content the player might miss, or anything else.
- please avoid giving the player non-loyal companions. This is partially a style thing, partially to give players fewer "I'll reload if this unit dies" units, and partially to keep the overworld rewards preview simpler
- include PO hints! If we don't do them now we'll need to do them later.

</details>

## TASKLIST

<details>
<summary>Click to expand</summary>
### OTHER

- [IN PROGRESS] Konrad Artwork (Mechanical)
- [IN PROGRESS] S00: The Great Continent (Dalas)
</details>

### SCENARIOS TO WRITE

- (Occasional gaps in scenario numbers are deliberate. That way if we need to add a new scenario sometime we don't have to renumber everything.)

<details>
<summary>Click to expand</summary>
```txt
//--------------------
// AREA 1 (WESTERN WESNOTH)
//--------------------
----Western Scenarios:
[COMPLETE] S01: The Elves Besieged (Dalas)

[COMPLETE] S02: Flight of the Elves (Dalas)
- 2-skull difficulty. Rewards: Elvish Archer, Elvish Fighter, Elvish Scout, No carryover.
- elves are fleeing from the top right while humans man the flanks. Similar to the first scenario of Dirty Blood
	- Ethiliel/Galdrad may or may not still be alive, depending on the player's performance in the intro scenario
- the more elves who die, the fewer recruits you get
- expect the player to have Delfador in this scenario. Balance accordingly.
	- if we have Delfador, foreshadow his impending departure

[COMPLETE] S03: Blackwater Port (ForestDragon)
- 2-skull difficulty. Rewards: Cavalryman, Horseman; possibly a couple high XP horsemen. No carryover/
- Asheviere's humans start in control of the port, but not the whole town. You need to defeat them to gain access to the port
	- if Kaylan dies, you can still win, but you won't get to recruit Horsemen/Cavalrymen
	- if Kaylan dies, set bm_kaylan_dead=yes, so I can modify the bigmap to reflect that
- lore: Sir Kaylan wanted Delfador to raise Konrad at the port among his own kind, but Delfador thought the Aethenwood was better: both more powerful and safer.
	- And on top of that, the port was too heavily militarized - no place to raise a child.
- expect the player to have Delfador in this scenario. Balance accordingly.
	- if we have Delfador, foreshadow his impending departure

[IN PROGRESS] S05: Bay of Pearls (Anekron)
- scenario number is used by Carcyn; if s05 is completed, Carcyn expects the player to have merfolk recruits
- 2-skull difficulty. Rewards: Merfolk Brawler, Merfolk Fighter, Merfolk Hunter, Merfolk Initiate
	(TO CHANGE: balance; expect Delfador)
- fight orcs and free merfolk.
- remember to include the sea orc event, and use the new image/portrait once that PR finishes
- no storm trident please. It's a really cool item, but this campaign is already complicated enough.
- expect the player to have Delfador in this scenario. Balance accordingly.
	- if we have Delfador, foreshadow his impending departure

[IN PROGRESS] S06: Isle of Alduin (ForestDragon)
- 1-skull difficulty. Rewards: Rogue Mage, Mage (Red advancement only)
- the original gives you a loyal mage called Elrian. We don't want a loyal mage reward, but we should still have "Elrian" appear somewhere in some context

[COMPLETE] S07: Muff Malal's Peninsula (Dalas)
- 1-skull difficulty. Rewards: companion Moremirmu

[COMPLETE] S08a: Isle of the Damned, part 1 (Dalas)
- 2-skull difficulty. Rewards: Thug, Poacher, access to northlands (via Elensefar Reef)
- we learn that Harper is trapped inside those catacombs where Moremirmu used to be, setting up S08b (if the player chooses to go there)
- need a good lore reason for the undead. Presumably the bandits have been living there peacefully since liberty, so the undead must be new
	- maybe Harper awakened them from down in the catacombs, or maybe something else

[COMPLETE] S08b: Isle of the Damned, part 2 (Dalas)
- 2-skull difficulty. Rewards: Footpad, companion Harper
- reference TRoW's vampire lady. Harper heard legends/rumors of someone like that, and came to see, inadvertantly awakening undead from the catacombs
	- possibly motivated by Asheviere. Looking for a weapon, powerful magic, etc to use against her
- need an older portrait

[COMPLETE] S10 Elensefar - before it's conquered (Dalas)
- 3-skill difficulty, high carryover. Rewards: Swordsman, Pikeman, Thief
	- combat involves Delfador/Maddock inside and Konrad outside, with a huge human army in between. Also some orcs in the north
	- at the beginning the loyalists send a messenger to the queen. Li'sar arrives some turns later
	- if you win here, you get the Elensefar achievement
[COMPLETE] S11 Elensefar - after it's conquered (Dalas)
- 2-skill difficulty, high carryover. Rewards: Thief, Seafarer's Sandals
	- the original HttT Elensefar scenario
- even after Elensefar, there should still be royal ships blocking travel up the river

//--------------------
// AREA 2 (CENTRAL WESNOTH)
//--------------------
[COMPLETE] S13 Fort Tahn (Dalas)
- 1 skull difficulty. Rewards: Moonbow, Low (or no?) carryover
- some kind of smuggling. A stealth mission, perhaps? (the player can ofc bypass this by just not playing the scenario)
- perhaps we meet the elven pike lady, but she doesn't follow us. https://forums.wesnoth.org/viewtopic.php?t=49283&start=15
	- maybe she makes a cameo later, as a medium-rank enemy leader somewhere?
	- she's seen plenty of kings/queens come and go (long life), and thus she doesn't really care?
- New thoughts; either Fort Tahn OR Aehtenwood (maybe "and"?) are a requirement for Kalenz to let you into Wesmere to get Shamans

[COMPLETE] S14: Gryphon Mountain (Dalas)
- scenario number is used by Carcyn; if s14 is completed and bm_s14_fought_burlin not_equals yes, Carcyn expects the player to have merfolk recruits
- if I ever let the player get Gryphons (not Gryphon Riders), make sure to change the "race=dwarf,gryhon" filter for the "fall of Knalga" dialogue, and the dialogue in 28_The_Lost_General
- 1-to-2-skull difficulty. Rewards: Gryphon Rider (maybe dwarves too, to ride the gryphons?)

[COMPLETE] S15a: Valley of Death, part 1 (Dalas)
- 1-skull difficulty. Rewards: Heavy Infantryman
- needs to make sense even if we did part 2 first
- we encounter some HI trying to fight through undead and get help. They say they're warrior monks, whose home is besieged by undead.
	 - HI names (including future recruits) should all start with "Brother"

[COMPLETE] S15b: Valley of Death, part 2 (Dalas)
- 2-skull difficulty. Rewards: Mage (White advancement only)
- Konrad reaches the monastery in a dense fog. The monks (a White Mage and some normal Mages) are overjoyed. Everyone gets ready to leave.
	- White Mage names (including future promotions) should start with "Brother"/"Sister"
- Monks think the fog is suspicious. They cast a spell to illuminate the map, revealing the 3 liches from the original Valley of Death.
- overworld: ensure we can leave the area after completing part 2, even if we didn't do part 1 beforehand

[COMPLETE] S17: Crossroads (Dalas)
- 2-skull difficulty. Rewards: companion Ulfdain, Dwarvish Miner
- we could have Li'sar leave mid-scenario to go to Elensefar, but for now I'm leaving this as-is; Elensefar gets conquered even though Lisar stays at Crossroads

[COMPLETE] S18 Dan'tonk (Dalas)
- 2-skull difficulty. Rewards: Spearman, Bowman, Fencer

[COMPLETE] S20 The Ford of Abez (Dalas)
- 1-skull difficulty. Rewards: none
- on the bigmap, orcs give you an opportunity to pay and get let through without combat (and without taking any time). If you refuse, you have to fight the orcs.
	- if you fight and win, you get back any gold you've previously paid: $bm_ford_amount_paid
- map needs to work properly even if Konrad comes from the north!
	or maybe the trolls are fine with people entering Wesnoth, just not leaving?
- do NOT work on this scenario until TDG/AD Ford of Abez is finalized, since terrain will be dependent

//--------------------
// AREA 3 (NORTHLANDS)
//--------------------
[COMPLETE] S22 Glamdrol (Dalas)
- 2-skull difficulty, Rewards: Horn of Glamdrol, High carryover
- Maybe multiple orcish clans are fighting for control of Glamdrol?
	- if so, make sure to introduce at least 1 new clan. Not just Foxtails and Stoneskins
	- can I tie in the "Bloody Sword Clan" from Northern Rebirth?

[COMPLETE] S23 Northern Winter (Dalas)
- 1-skull difficulty, Rewards: companion Raewan, No carryover
- keep the name, but include an explanation if it's not actually winter. And keep the "terrain turns to snow" mechanic, and add to it
- fight against wild animals; Raewan needs various ingredients to prepare her concoctions. Each objective gives Raewan a new ability/attack (until she has all of her normal stuff)
- harmful environmental effects that need to be dealt with. Probably either global or something like "get slowed if you stand on a snow hex"

[COMPLETE] S24 An Old Friend (Dalas)
- 1-skull difficulty. Rewards: Elvish Shamans -OR- Chantal, Low carryover
- make sure you can't get in without Kalenz, even if you "unlocked" wesmere with him previously before he died

[COMPLETE] S26 Dwarven Doors (Dalas)
- TBD difficulty, Rewards: Peasant, Woodsman, High carryover
- free as many slaves as you can, and evacuate them into the underground
- scenario number is used by the Digging for Dwarves introductions check; Dwarven Doors peasants will vouch for Konrad there

[COMPLETE] S28 The Lost General (Dalas)
- 1-skull difficulty, Rewards: Bloodaxe (item for Konrad)

[COMPLETE] S29 Digging for Dwarves (Dalas)
- TBD difficulty. Rewards: ONE OF Dwarvish Fighter, Dwarvish Guardsman, Dwarvish Thunderer, Dwarvish Ulfserker
- involves using miners to mine through walls? Find a way to stop this from being a huge XP farm
- try to keep lore consistent with NR please

//--------------------
// AREA 4 (JOINING LI'SAR)
//--------------------
[COMPLETE] S30 The Sceptre of Fire (Dalas)
- 3-skull difficulty. Rewards: the sceptre of fire
- first we search for the sceptre in a small, fairly easy map
- once we find it, Li'sar is revealed to be chasing us
- the player gets a choice. 1) flee (leaving the scepter), 2) defend until the scepter can be broken free from the rock, 3) capture Li-sar (and get her special chainmail)
- regardless, once the player wins, Li'sar's silver mage will cause the volcano to erupt, ending the scenario

[COMPLETE] S31 The Swamp of Dread (Dalas)

----ONCE WE FINISH THROUGH HERE, LET'S STOP, POLISH, AND RELEASE TO GET FEEDBACK BEFORE MOVING FURTHER ON----

//--------------------
// AREA 5 (EASTERN WESNOTH)
//--------------------
[DONE] S41: Snow Plains (Dalas)
- 2-skull difficulty. Rewards: deprive Asheveire of magi

[DONE] S42: A Crisis of Leadership (Dalas)
- 2-skull difficulty. Rewards: deprive Asheviere of orcs
dosh gets a special event here? Maybe he can become the new chief and use the whitefangs against Asheviere?
	if so, make sure to explain why there'll be fewer whitefangs helping than there would have been opposing
- Bazur should be the high chief? Maybe we're trying to help a rival ovethrow him?

[DONE] S43: Cliffs of Thoria (Dalas)
- 3-skull difficulty. Rewards: Li’sar gains 2x starting gold and income.
	And Lisar levels up to L4, preserving any XP she may have had beforehand
	(and Warven joins her as a {ZONE_GUARDIAN} leader)

[DONE] S44: Underground Channels (Dalas)
- 2-skull difficulty. Rewards: Delfador learns to recruit Granite Golems (45g?)
	bigmap: delfador says he knew a grove of woses to be here once

[DONE] Soradoc event

[IN PROGRESS] S47: Test of the Clans (Dalas)
- 3-skull difficulty. Rewards: deprive Asheviere of cavalry
make sure the clansmen say that they won't follow Konrad just because of the sceptre

S48: Dan'Tonk
- 3-skull difficulty. Rewards: deprive Asheviere of footmen
- we actually sack the city this time?
make sure the soldiers (led by Isolde?) say they won't follow Konrad just because of the sceptre

[IN PROGRESS] S50: Battle for Wesnoth
	scenario number used in smalltalk
```
</details>
