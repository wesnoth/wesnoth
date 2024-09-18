# Modding World Conquest

This file describes how to make your addon work well together with world conquest.

## How to add an era's units to World Conquest's enemy selection pool

Making an era that works well together with world conquest, is quite easy: just add the `[world_conquest_data]` tag to
your `[multiplayer_side]` tag just as the standard world conquest era does, important: your era may **not** set
`require_era=no` for this to work.

Furthermore if your era contains new unit types don't forget to put an additional `[world_conquest_data]` in `[era]`
to describe which traits these unit types should get when they are selected as heroes (use this to compensate weaker
unit types).

To make an era/modification that also changes the enemy army unit types, make sure to define the `wc2_init_enemy` event
and use it to set the `wc2_enemy_army` variable that should contain the pool of 'armies' that the enemies are chosen
from:

```ini
[event]
    name = "wc2_init_enemy"
    [filter_conditional]
        [variable]
            name="wc2_enemy_army.length"
            equals=0
        [/variable]
    [/filter_conditional]
    [set_variables]
        name = "wc2_enemy_army"
        [literal]
            # last digit has to be the total number of factions you want to [group]
            factions_available = "1,1,1,1,2,2,2,2,3,3,3,3,4,4,4,4"
            [group]
                # allies will be the factions who should be paired with this one
                allies_available="2,3,4"
                id = "enemyfaction_id"
                recruit= "Orcish Grunt,Orcish Archer,Wolf Rider,Orcish Assassin,Troll Whelp"
                [recall]
                    level2 = "Orcish Ruler, Orcish Slayer,..."
                    level2 = "Orcish Warlord, Troll Warrior,..."
                [/recall]
                [commander]
                    level1 = "Orcish Leader, Orcish Grunt,..."
                    level2 = "Orcish Ruler, Orcish Slayer,..."
                    level2 = "Orcish Warlord, Troll Warrior,..."
                [/commander]
                [leader]
                    level2 = "Troll"
                    level3 = "Troll Warrior"
                    recruit = "Orcish Grunt,Orcish Archer,Troll Whelp"
                [/leader]
                [leader]
                    level2 = "Orcish Warrior"
                    level3 = "Orcish Warlord"
                    recruit = "Orcish Grunt,Orcish Archer,Wolf Rider,Orcish Assassin"
                [/leader]
                # add more leader specifics here
            [/group]
                        # make another group and copy-paste the previous one to make more factions
        [/literal]
    [/set_variables]
[/event]
```

## How to make a WC-style era

In your era's CFG file, code in the following:

Let's assume I want to make a World Conquest era for the...Extended Era...

We have done this to ensure the era follows the same code and functionality as the original World Conquest era.
Next, continue coding:

```ini
#ifdef LOAD_WC2
[resource]
    id= extended_era_resource_main

    # load the World Conquest era-specific Lua Code
    [lua]
        code = " wesnoth.dofile('campaigns/World_Conquest/lua/era_main.lua') "
    [/lua]

    # this defines who the commanders, deserters and heroes
    [world_conquest_data]
        [hero_types]
            [Aragwaithi] # Note this tag can be given any name (but maintain consistency)
                name= _ "Aragwaithi"
                types="Aragwaith Swordsman,Aragwaith Spearman,Aragwaith Adept,Aragwaith Archer"
            [/Aragwaithi]
            [Dark_Elves]
                name= _ "Dark Elves"
                types="Dark Elven Fighter,Dark Elven Hunter, Dark Elven Lizard Rider" # and so on...
            [/Dark_Elves]
            # Combined
            # you need this bonus all since this is what WC sees when loading up the hero availability pool in bonus points.
            [Bonus_All] # Notice how I added in the hero groups in. This is how it should be done.
                types=Aragwaithi,Dark_Elves
            [/Bonus_All]
        [/hero_types]
        ## array of [trait_extra]

        # This is used to force assign the mentioned trait to certain units
        # as compensation if the unit type is deemed weak (such as goblin spearman).
        [trait_extra]
            types=Walking Corpse
            {WORLD_CONQUEST_II_TRAIT_LEGENDARY_ZOMBIE}
        [/trait_extra]
        [trait_extra]
            types=Goblin Spearman
            {WORLD_CONQUEST_II_TRAIT_LEGENDARY_GOBLIN}
        [/trait_extra]
        [trait_extra]
            types=Orcish Assassin,Young Ogre,Ruffian,Woodsman
            {WORLD_CONQUEST_II_TRAIT_EPIC}
        [/trait_extra]
        [trait_extra]
            types=Ghoul,Poacher,Thief,Footpad,Saurian Skirmisher,Vampire Bat,Peasant,Dune Herbalist
            {WORLD_CONQUEST_II_TRAIT_EXPERT}
        [/trait_extra]
        [trait_extra]
            types=Elvish Archer,Elvish Shaman,Elvish Scout,Elvish Fighter
            {TRAIT_DEXTROUS}
        [/trait_extra]
        [trait_extra]
            types=Dwarvish Guardsman,Dwarvish Ulfserker,Dwarvish Thunderer,Dwarvish Scout
            {TRAIT_HEALTHY}
        [/trait_extra]
        [trait_extra]
            types=Naga Fighter,Wolf Rider,Orcish Grunt,Drake Glider,Dune Rover,Dune Rider
            {TRAIT_STRONG}
        [/trait_extra]
        [trait_extra]
            types=Spearman,Fencer,Cavalryman,Merman Fighter,Merman Hunter,Mermaid Initiate,Dune Burner,Thug
            {TRAIT_RESILIENT}
        [/trait_extra]
        [trait_extra]
            types=Heavy Infantryman,Bowman,Skeleton,Skeleton Archer,Saurian Augur,Troll Whelp,Orcish Archer
            {TRAIT_FEARLESS}
        [/trait_extra]
        # This filters the following hero types
        # to simply explain this...
        # if your bonus point is next to mountains and cave walls and such, then these should be far away from
        # watery bodies..thus, aquatic heroes should not be spawned in that case
        [hero_spawn_filter]
            types=Naga Fighter,Merman Fighter,Merman Hunter,Mermaid Initiate,Naga Dirkfang,Naga Guard
            [filter_location]
                [filter_radius]
                    [not]
                        terrain="M*,X*"
                    [/not]
                [/filter_radius]
                terrain="W*,S*"
                radius=2
            [/filter_location]
        [/hero_spawn_filter]
    [/world_conquest_data]
[/resource]

[era]
    id=extended_conquest_era
    name= _ "Extended Conquest Era"
    require_era=yes

    description= _ "This is a string"

    {RANDOM_SIDE}

    # This is what an MP side code should look like 
    [multiplayer_side]
        id= # ID of the faction
        name= # name of the faction
        recruit=Drake Fighter,Dwarvish Fighter,Drake Burner,Dwarvish Thunderer,Saurian Augur,Dwarvish Ulfserker,Drake Clasher,Dwarvish Guardsman,Saurian Skirmisher,Poacher,Drake Glider,Footpad,Thief,Thief
        # recruit list. For your own benefit, write the recruit list with pairs side by side
        #as not to be confused later on
        image= # image of the faction
        type=random
        leader= Dwarvish Steelclad,Dwarvish Thunderguard,Dwarvish Stalwart,Rogue,Trapper,Drake Flare,Fire Drake,Drake Arbiter,Drake Thrasher,Drake Warrior,Saurian Oracle,Saurian Soothsayer
        random_leader= # list of leader ids which should be chosen if randomised
        description= # description of the faction (you can skip this one)
        [world_conquest_data]
            commanders=Drakes,Knalgans
            heroes=Loyalists_All,Northerners_All,Young Ogre
            deserters=Rebels,Undead,Dune Burner
            deserters_names=_ "Dune Burner, Rebels and Undead"
            {WC_II_PAIR "Drake Fighter" "Dwarvish Fighter"}
            {WC_II_PAIR "Drake Burner" "Dwarvish Thunderer"}
            {WC_II_PAIR "Saurian Augur" "Dwarvish Ulfserker"}
            {WC_II_PAIR "Drake Clasher" "Dwarvish Guardsman"}
            {WC_II_PAIR "Saurian Skirmisher" "Poacher"}
            {WC_II_PAIR "Drake Glider" "Footpad"}
            {WC_II_PAIR "Thief" "Thief"}
        [/world_conquest_data]
    [/multiplayer_side]


    {QUICK_4MP_LEADERS}
    {TURNS_OVER_ADVANTAGE}

    # load the main resource
    # otherwise you will encounter errors
    [load_resource]
        id = extended_era_resource_main
    [/load_resource]
[/era]
#endif
```

## How to make your own artifacts/trainers/hero_types and have them added to World Conquest

Instead of adding them to an era you can also put them in a `[resource]` that gets loaded from a `[modification]`.
Make sure to mark the modification as required (`require_modification=yes`) or different clients may load different data in multiplayer leading to issues.
World Conquest looks for `[world_conquest_data]` tags in the era and every loaded resource.

```ini
[era]
    # era code here

    [world_conquest_data]
        [artifact]
            name= _ "Leather Bag of Herbal Stuff"
            icon=items/leather-pack.png # dont add attack icons here, just what the item image here.
            description= _ "heals +4 and regenerates +4"
            info = _ "Some story about random Dune Alchemist dude deciding to pack his stuff in a bag and it got lost."
            sound=heal.wav
            [effect]
                apply_to=remove_ability
                [abilities]
                    [heals]
                        id=healing
                    [/heals]
                [/abilities]
            [/effect]
            [effect]
                apply_to=new_ability
                [abilities]
                    {ABILITY_HEALS}
                    {ABILITY_SELF_HEAL}
                [/abilities]
            [/effect]
        [/artifact]

        [trainer]
            type=Blood Manipulator
            advanced_type=Sangel
            image=attacks/wail.png
            name= _ "Blood Magic"
            dialogue= "You have found me, mortals? Well...let us show your recruits some blood magic!"
            [grade]
            [/grade]
            [grade]
                {WCT_CHANCE_FEEDING 3}
                {WCT_CHANCE_SP 2 MELEE DRAIN}
                {WCT_CHANCE_SP 1 RANGED DRAIN}
                {WCT_CHANCE_DARK_DEFENSE CAVE 7}
                {WCT_CHANCE_DARK_DEFENSE MUSHROOM 6}
                {WCT_CHANCE_XP 73 -10%}
            [/grade]
            [grade]
                {WCT_CHANCE_FEEDING 7}
                {WCT_CHANCE_ABILITY 2 REGENERATES}
                {WCT_CHANCE_SP 4 MELEE DRAIN}
                {WCT_CHANCE_SP 2 RANGED DRAIN}
                {WCT_CHANCE_DARK_DEFENSE CAVE 16}
                {WCT_CHANCE_DARK_DEFENSE MUSHROOM 13}
                {WCT_CHANCE_XP 73 -20%}
            [/grade]
            [grade]
                {WCT_CHANCE_FEEDING 12}
                {WCT_CHANCE_ABILITY 5 REGENERATES}
                {WCT_CHANCE_SP 6 MELEE DRAIN}
                {WCT_CHANCE_SP 3 RANGED DRAIN}
                {WCT_CHANCE_SP 1 MELEE SLOW}
                {WCT_CHANCE_SP 1 RANGED SLOW}
                {WCT_CHANCE_ARCANE_BOOST 1 MELEE}
                {WCT_CHANCE_ARCANE_BOOST 1 RANGED}
                {WCT_CHANCE_DARK_DEFENSE CAVE 29}
                {WCT_CHANCE_DARK_DEFENSE MUSHROOM 21}
                {WCT_CHANCE_XP 73 -30%}
            [/grade]
        [/trainer]
        [hero_types]
            [Aragwaithi] # Note this tag can be given any name (but maintain consistency)
                name= _ "Aragwaithi"
                types="Aragwaith Swordsman,Aragwaith Spearman,Aragwaith Adept,Aragwaith Archer"
            [/Aragwaithi]
            [Dark_Elves]
                name= _ "Dark Elves"
                types="Dark Elven Fighter,Dark Elven Hunter, Dark Elven Lizard Rider" # and so on...
            [/Dark_Elves]
            # Combined
            # you need this bonus all since this is what WC sees when loading up the hero availability pool in bonus points.
            [Bonus_All] # Notice how I added in the hero groups in. This is how it should be done.
                types=Aragwaithi,Dark_Elves
            [/Bonus_All]
        [/hero_types]
    [/world_conquest_data]
[/era]
```

## Summary 

### Overall World Conquest file structure:

Normally, this is what an entire modifications/era file should look like. Notice where the `#ifdef LOAD_WC2` and `endif` have been used. This has been added to further illustrate and clarify problems. 

```ini
#ifdef LOAD_WC2
[resource]
    id= extended_era_resource_main

    # load the World Conquest era-specific Lua Code
    [lua]
        code = " wesnoth.dofile('campaigns/World_Conquest/lua/era_main.lua') "
    [/lua]

    # this defines who the commanders, deserters and heroes
    [world_conquest_data]
        [hero_types]
            [Aragwaithi] # Note this tag can be given any name (but maintain consistency)
                name= _ "Aragwaithi"
                types="Aragwaith Swordsman,Aragwaith Spearman,Aragwaith Adept,Aragwaith Archer"
            [/Aragwaithi]
            [Dark_Elves]
                name= _ "Dark Elves"
                types="Dark Elven Fighter,Dark Elven Hunter, Dark Elven Lizard Rider" # and so on...
            [/Dark_Elves]
            # Combined
            # you need this bonus all since this is what WC sees when loading up the hero availability pool in bonus points.
            [Bonus_All] # Notice how I added in the hero groups in. This is how it should be done.
                types=Aragwaithi,Dark_Elves
            [/Bonus_All]
        [/hero_types]
        ## array of [trait_extra]

        # This is used to force assign the mentioned trait to certain units
        # as compensation if the unit type is deemed weak (such as goblin spearman).
        [trait_extra]
            types=Walking Corpse
            {WORLD_CONQUEST_II_TRAIT_LEGENDARY_ZOMBIE}
        [/trait_extra]
        [trait_extra]
            types=Goblin Spearman
            {WORLD_CONQUEST_II_TRAIT_LEGENDARY_GOBLIN}
        [/trait_extra]
        [trait_extra]
            types=Orcish Assassin,Young Ogre,Ruffian,Woodsman
            {WORLD_CONQUEST_II_TRAIT_EPIC}
        [/trait_extra]
        [trait_extra]
            types=Ghoul,Poacher,Thief,Footpad,Saurian Skirmisher,Vampire Bat,Peasant,Dune Herbalist
            {WORLD_CONQUEST_II_TRAIT_EXPERT}
        [/trait_extra]
        [trait_extra]
            types=Elvish Archer,Elvish Shaman,Elvish Scout,Elvish Fighter
            {TRAIT_DEXTROUS}
        [/trait_extra]
        [trait_extra]
            types=Dwarvish Guardsman,Dwarvish Ulfserker,Dwarvish Thunderer,Dwarvish Scout
            {TRAIT_HEALTHY}
        [/trait_extra]
        [trait_extra]
            types=Naga Fighter,Wolf Rider,Orcish Grunt,Drake Glider,Dune Rover,Dune Rider
            {TRAIT_STRONG}
        [/trait_extra]
        [trait_extra]
            types=Spearman,Fencer,Cavalryman,Merman Fighter,Merman Hunter,Mermaid Initiate,Dune Burner,Thug
            {TRAIT_RESILIENT}
        [/trait_extra]
        [trait_extra]
            types=Heavy Infantryman,Bowman,Skeleton,Skeleton Archer,Saurian Augur,Troll Whelp,Orcish Archer
            {TRAIT_FEARLESS}
        [/trait_extra]
        # This filters the following hero types
        # to simply explain this...
        # if your bonus point is next to mountains and cave walls and such, then these should be far away from
        # watery bodies..thus, aquatic heroes should not be spawned in that case
        [hero_spawn_filter]
            types=Naga Fighter,Merman Fighter,Merman Hunter,Mermaid Initiate,Naga Dirkfang,Naga Guard
            [filter_location]
                [filter_radius]
                    [not]
                        terrain="M*,X*"
                    [/not]
                [/filter_radius]
                terrain="W*,S*"
                radius=2
            [/filter_location]
        [/hero_spawn_filter]
    [/world_conquest_data]
[/resource]

[era]
    id=extended_conquest_era
    name= _ "Extended Conquest Era"
    require_era=yes

    description= _ "This is a string"

    {RANDOM_SIDE}

    # This is what an MP side code should look like 
    [multiplayer_side]
        id= # ID of the faction
        name= # name of the faction
        recruit=Drake Fighter,Dwarvish Fighter,Drake Burner,Dwarvish Thunderer,Saurian Augur,Dwarvish Ulfserker,Drake Clasher,Dwarvish Guardsman,Saurian Skirmisher,Poacher,Drake Glider,Footpad,Thief,Thief
        # recruit list. For your own benefit, write the recruit list with pairs side by side
        #as not to be confused later on
        image= # image of the faction
        type=random
        leader= Dwarvish Steelclad,Dwarvish Thunderguard,Dwarvish Stalwart,Rogue,Trapper,Drake Flare,Fire Drake,Drake Arbiter,Drake Thrasher,Drake Warrior,Saurian Oracle,Saurian Soothsayer
        random_leader= # list of leader ids which should be chosen if randomised
        description= # description of the faction (you can skip this one)
        [world_conquest_data]
            commanders=Drakes,Knalgans
            heroes=Loyalists_All,Northerners_All,Young Ogre
            deserters=Rebels,Undead,Dune Burner
            deserters_names=_ "Dune Burner, Rebels and Undead"
            {WC_II_PAIR "Drake Fighter" "Dwarvish Fighter"}
            {WC_II_PAIR "Drake Burner" "Dwarvish Thunderer"}
            {WC_II_PAIR "Saurian Augur" "Dwarvish Ulfserker"}
            {WC_II_PAIR "Drake Clasher" "Dwarvish Guardsman"}
            {WC_II_PAIR "Saurian Skirmisher" "Poacher"}
            {WC_II_PAIR "Drake Glider" "Footpad"}
            {WC_II_PAIR "Thief" "Thief"}
        [/world_conquest_data]
    [/multiplayer_side]


    {QUICK_4MP_LEADERS}
    {TURNS_OVER_ADVANTAGE}

    # load the main resource
    # otherwise you will encounter errors
    [load_resource]
        id = extended_era_resource_main
    [/load_resource]

    # artifacts and trainers
    [world_conquest_data]
        [artifact]
            name= _ "Leather Bag of Herbal Stuff"
            icon=items/leather-pack.png # dont add attack icons here, just what the item image here.
            description= _ "heals +4 and regenerates +4"
            info = _ "Some story about random Dune Alchemist dude deciding to pack his stuff in a bag and it got lost."
            sound=heal.wav
            [effect]
                apply_to=remove_ability
                [abilities]
                    [heals]
                        id=healing
                    [/heals]
                [/abilities]
            [/effect]
            [effect]
                apply_to=new_ability
                [abilities]
                    {ABILITY_HEALS}
                    {ABILITY_SELF_HEAL}
                [/abilities]
            [/effect]
        [/artifact]
        [trainer]
            type=Blood Manipulator
            advanced_type=Sangel
            image=attacks/wail.png
            name= _ "Blood Magic"
            dialogue= "You have found me, mortals? Well...let us show your recruits some blood magic!"
            [grade]
            [/grade]
            [grade]
                {WCT_CHANCE_FEEDING 3}
                {WCT_CHANCE_SP 2 MELEE DRAIN}
                {WCT_CHANCE_SP 1 RANGED DRAIN}
                {WCT_CHANCE_DARK_DEFENSE CAVE 7}
                {WCT_CHANCE_DARK_DEFENSE MUSHROOM 6}
                {WCT_CHANCE_XP 73 -10%}
            [/grade]
            [grade]
                {WCT_CHANCE_FEEDING 7}
                {WCT_CHANCE_ABILITY 2 REGENERATES}
                {WCT_CHANCE_SP 4 MELEE DRAIN}
                {WCT_CHANCE_SP 2 RANGED DRAIN}
                {WCT_CHANCE_DARK_DEFENSE CAVE 16}
                {WCT_CHANCE_DARK_DEFENSE MUSHROOM 13}
                {WCT_CHANCE_XP 73 -20%}
            [/grade]
            [grade]
                {WCT_CHANCE_FEEDING 12}
                {WCT_CHANCE_ABILITY 5 REGENERATES}
                {WCT_CHANCE_SP 6 MELEE DRAIN}
                {WCT_CHANCE_SP 3 RANGED DRAIN}
                {WCT_CHANCE_SP 1 MELEE SLOW}
                {WCT_CHANCE_SP 1 RANGED SLOW}
                {WCT_CHANCE_ARCANE_BOOST 1 MELEE}
                {WCT_CHANCE_ARCANE_BOOST 1 RANGED}
                {WCT_CHANCE_DARK_DEFENSE CAVE 29}
                {WCT_CHANCE_DARK_DEFENSE MUSHROOM 21}
                {WCT_CHANCE_XP 73 -30%}
            [/grade]
        [/trainer]
    [/world_conquest_data]
[/era]
#endif
```
