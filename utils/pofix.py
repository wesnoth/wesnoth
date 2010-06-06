#!/usr/bin/env python
# -*- coding: utf-8 -*-

# pofix - perform string fixups on incoming .po files.
#
# The purpose of this script is to save translators from having to
# apply various string fixes needed before stable release by hand.  It is
# intended to be run on each incoming .po file as the Lord of
# Translations receives it.  However, translators may run it on their
# own .po files to be sure, as a second application will harmlessly do
# nothing.
#
# To use this script, give it one or more paths to .po files as
# command-line arguments.  Each file will be tweaked as needed.
# It should work on Windows and MacOS X as well as Linux, provided
# you have Python installed.
#
# This script will emit a report line for each file it modifies,
# and save a backup copy of the original with extension "-bak".
#
# This script will tell you when it is obsolete.  Run it against all .po
# files in the main Wesnoth tree; when it says none are older than this script,
# it can be discarded (assunming that it has in fact been used to transform
# all incoming .po files in the meantime).
#
# Example usage:
# utils/pofix.py po/wesnoth*/*.po*
# find data/campaigns/ -name '*.cfg' -print0 | xargs -0 utils/pofix.py
#
# Please do make sure to add a comment before and new blocks of conversions
# that states when it was added (current version number is enough) so that
# the file can be cleaned up more easily every now and then.
# Example:
# # conversion added in 1.9.5+svn
# ("foo addwd bar", "foo added bar"),
# # conversion added in 1.9.8+svn
# ("fooba foo", "foobar foo"),

stringfixes = {

"wesnoth" : (
# Convert hyphen-minuses that are being used as minus signs
# to the Unicode minus sign
# conversion added in 1.9.0-svn
(" -25%", " −25%"),
(" -1%", " −1%"),
(" -100%", " −100%"),

# Convert makeshift dashes and horizontal bars:
# conversion added in 1.9.0-svn
("however - from these", "however — from these"),
("campaign first - click", "campaign first — click"),
("unit type -", "unit type —"),
("experience - 4 for", "experience — 4 for"),
("both worlds - for", "both worlds — for"),
("respected - or simply", "respected — or simply"),
("feared - leader", "feared — leader"),
("- usually in blood - although", "— usually in blood — although"),
("position - although", "position — although"),
("as advisors -", "as advisors —"),
("- Great Mage Delfador", "― Great Mage Delfador"),
("- The Wesnoth Tactical Guide", "― The Wesnoth Tactical Guide"),
("- King Konrad, 536YW", "― King Konrad, 536YW"),
("- Memoirs of Gweddry, 627YW", "― Memoirs of Gweddry, 627YW"),
("- High Lord Kalenz, 470YW", "― High Lord Kalenz, 470YW"),
("- The Wesnoth Community", "― The Wesnoth Community"),
("- Royal Compendium of Battle Terminology - Volume II", "― Royal Compendium of Battle Terminology — Volume II"),
("- Handbook of Tactical Analysis Volume I - Haldric", "― Handbook of Tactical Analysis Volume I — Haldric"),
("- Handbook of Tactical Analysis Volume II - Haldric", "― Handbook of Tactical Analysis Volume II — Haldric"),
("don't have to - let it", "don't have to — let it"),
("- Sir Kaylan,", "― Sir Kaylan,"),
("- Princess Li'sar,", "― Princess Li'sar,"),
("your attacks - they will", "your attacks — they will"),
("- The Scroll of Chantal,", "― The Scroll of Chantal,"),
("- Great Sage Dacyn,", "― Great Sage Dacyn,"),
("- Queen Li'sar,", "― Queen Li'sar,"),
("upload statistics - Help", "upload statistics — Help"),
("(A) - auth command", "(A) — auth command"),
("(D) - debug only, (N) - network only, (A) - auth only", "(D) — debug only, (N) — network only, (A) — auth only"),
("not empty - duplicate", "not empty — duplicate"),
("- King Konrad,", "― King Konrad,"),
("Player Info -", "Player Info —"),
("About to upload statistics - Help us make Wesnoth better for you!", "About to upload statistics — Help us make Wesnoth better for you!"),
#the following rule applies to wesnoth/*.po* and to wesnoth-manual/*.po*
("victory objectives - getting", "victory objectives — getting"),
# Fix screw up (convert em dashes to horizontal bars)
# conversion added in 1.9.0-svn
("— Great Mage Delfador", "― Great Mage Delfador"),
("— The Wesnoth Tactical Guide", "― The Wesnoth Tactical Guide"),
("— King Konrad, 536YW", "― King Konrad, 536YW"),
("— Memoirs of Gweddry, 627YW", "― Memoirs of Gweddry, 627YW"),
("— High Lord Kalenz, 470YW", "― High Lord Kalenz, 470YW"),
("— The Wesnoth Community", "― The Wesnoth Community"),
("— Royal Compendium of Battle Terminology - Volume II", "― Royal Compendium of Battle Terminology — Volume II"),
("— Handbook of Tactical Analysis Volume I - Haldric", "― Handbook of Tactical Analysis Volume I — Haldric"),
("— Handbook of Tactical Analysis Volume II - Haldric", "― Handbook of Tactical Analysis Volume II — Haldric"),
("— Sir Kaylan,", "― Sir Kaylan,"),
("— Princess Li'sar,", "― Princess Li'sar,"),
("— The Scroll of Chantal,", "― The Scroll of Chantal,"),
("— Great Sage Dacyn,", "― Great Sage Dacyn,"),
("— Queen Li'sar,", "― Queen Li'sar,"),
("— King Konrad,", "― King Konrad,"),
),

"wesnoth-anl" : (
# Convert makeshift dashes:
# conversion added in 1.9.0-svn
("4p - A New Land", "4p — A New Land"),
("some underground mushroom mines nearby -", "some underground mushroom mines nearby —"),
("A New Land - Help", "A New Land — Help"),
("Our talks are complete -", "Our talks are complete —"),
),

"wesnoth-aoi" : (
# Convert makeshift dashes:
# conversion added in 1.9.0-svn
("was easy to follow - a wide", "was easy to follow — a wide"),
("unmistakable - tree stumps", "unmistakable — tree stumps"),
("question remained - would he", "question remained — would he"),
("this scenario - you must", "this scenario — you must"),
("worse - an orcish", "worse — an orcish"),
),

"wesnoth-did" : (
# Convert makeshift dashes:
# conversion added in 1.9.0-svn
("A Small Favor -", "A Small Favor —"),
("running away - my horsemen", "running away — my horsemen"),
),

"wesnoth-dm" : (
# Convert makeshift dashes:
# conversion added in 1.9.0-svn
("warn you - a party", "warn you — a party"),
("each other - and you'll", "each other — and you'll"),
("Night is falling - that's", "Night is falling — that's"),
("work by now - I did not", "work by now — I did not"),
("seeking you - you see", "seeking you — you see"),
("Of course - do you", "Of course — do you"),
("Knalga - the rumor", "Knalga — the rumor"),
("Worse news - the", "Worse news — the"),
("been to the west - will the", "been to the west — will the"),
("the dead - should", "the dead — should"),
("Illuven - lesser", "Illuven — lesser"),
("need protection - cost", "need protection — cost"),
("No thanks - we'll manage by ourselves...", "No thanks — we'll manage by ourselves..."),
("Let's move on - the less", "Let's move on — the less"),
("We should camp for the night now - we", "We should camp for the night now — we"),
("Those standing stones - they summon", "Those standing stones — they summon"),
("possible - I want to get us", "possible — I want to get us"),
("they are woses - tree guardians", "they are woses — tree guardians"),
("no alternative - we must get", "no alternative — we must get"),
("things in Wesnoth - we must fight", "things in Wesnoth — we must fight"),
("stirred on the island -", "stirred on the island —"),
("see my greatest achievement - an", "see my greatest achievement — an"),
("must be informed immediately -", "must be informed immediately —"),
("This forest looks quiet - too quiet.", "This forest looks quiet — too quiet."),
("No - you can't be dead!", "No — you can't be dead!"),
("of our help too - this", "of our help too — this"),
),

"wesnoth-dw" : (
# Convert makeshift dashes:
# conversion added in 1.9.0-svn
("hearten the folk. And -", "hearten the folk. And —"),
("if you will permit - you", "if you will permit — you"),
("a week ago - wanted", "a week ago — wanted"),
("disturbing that a Kai - and", "disturbing that a Kai — and"),
("- would run here", "— would run here"),
),

"wesnoth-ei" : (
# Convert makeshift dashes:
# conversion added in 1.9.0-svn
("Wait, before we go anywhere - who", "Wait, before we go anywhere — who"),
("This adept is weak - we may", "This adept is weak — we may"),
("onward - we must defeat", "onward — we must defeat"),
("playing a sick game - whenever", "playing a sick game — whenever"),
("to take care of - we must", "to take care of — we must"),
("to help us - but the orcs", "to help us — but the orcs"),
("It looks repairable - we", "It looks repairable — we"),
("Run for your - what the", "Run for your — what the"),
# ... and aint -> ain't
# conversion added in 1.9.0-svn
("I aint charging gold -", "I ain't charging gold —"),
("'T'aint safe", "'T'ain't safe"),
# Make it unspaced...
# conversion added in 1.9.0-svn
("may be able to help us in - ", "may be able to help us in—"),
("Wait - what just happened?", "Wait — what just happened?"),
),

"wesnoth-httt" : (
# Convert makeshift dashes:
# conversion added in 1.9.0-svn
("and the support of my men - from", "and the support of my men — from"),
("NE - Dan'Tonk", "NE — Dan'Tonk"),
("SE - Fort Tahn", "SE — Fort Tahn"),
("The Valley of Death - The Princess's Revenge", "The Valley of Death — The Princess's Revenge"),
("the hills - there are undead about!", "the hills — there are undead about!"),
("those gryphon eggs - they", "those gryphon eggs — they"),
("- Delfador's insistence", "— Delfador's insistence"),
("Look - orcs are", "Look — orcs are"),
("A frail human - or worse, an elf -", "A frail human — or worse, an elf —"),
("out to the heir - I", "out to the heir — I"),
("gruesome sight - a fleet", "gruesome sight — a fleet"),
("introduce myself - I", "introduce myself — I"),
("my warning - prepare", "my warning — prepare"),
("princess - the heiress", "princess — the heiress"),
("don't try to fight us - you", "don't try to fight us — you"),
("Princess Li'sar - here?", "Princess Li'sar — here?"),
("Look - you can", "Look — you can"),
("century - a generation", "century — a generation"),
("vast human army - his", "vast human army — his"),
),

"wesnoth-l" : (
# Convert makeshift dashes:
# conversion added in 1.9.0-svn
("are on the hunt - and", "are on the hunt — and"),
("and ruthlessness - and their", "and ruthlessness — and their"),
("of death - would only", "of death — would only"),
("my father - your grandfather - brought", "my father — your grandfather — brought"),
("catacombs - cover me.", "catacombs — cover me."),
("Liberty - Epilogue", "Liberty — Epilogue"),
("old friend. - Relnan", "old friend. — Relnan"),
),

"wesnoth-lib" : (
# Convert makeshift dashes
# conversion added in 1.9.0-svn
("Player Info - ", "Player Info — "),
),

"wesnoth-low" : (
# Convert makeshift dashes:
# conversion added in 1.9.0-svn
("follow you, Kalenz - but", "follow you, Kalenz — but"),
("Kalenz - lead us", "Kalenz — lead us"),
("them aid - it's clear", "them aid — it's clear"),
),

"wesnoth-manual" : (
),

"wesnoth-multiplayer" : (
# Convert makeshift dashes
# conversion added in 1.9.0-svn
("2p -", "2p —"),
("3p -", "3p —"),
("4p -", "4p —"),
("5p -", "5p —"),
("6p -", "6p —"),
("8p -", "8p —"),
("9p -", "9p —"),
("- - Seven Stones and Eleven", "— Seven Stones and Eleven"),
("- - Seven Stones - and the Elven", "— Seven Stones — and the Elven"),
("Bramwythl was left behind - in their haste, no one had remembered to find", "Bramwythl was left behind — in their haste, no one had remembered to find"),
("treasure that had been lost in these watery caves- a spear whose head was", "treasure that had been lost in these watery caves— a spear whose head was"),
("Single player mode - uses the reduced strength spawns", "Single player mode — uses the reduced strength spawns"),
("Two player mode - uses the full strength spawns.", "Two player mode — uses the full strength spawns."),
("Dark Forecast - a random survival scenario", "Dark Forecast — a random survival scenario"),
("They appear along the north, south and west map edges - though they are most", "They appear along the north, south and west map edges — though they are most"),
("- map design, spawn groups and scenario concept", "— map design, spawn groups and scenario concept"),
("- WML implementation", "— WML implementation"),
("- So this is the accursed valley - a land ours for the taking.", "So this is the accursed valley — a land ours for the taking."),
("watery caves- a spear whose", "watery caves— a spear whose"),
("-- map design, spawn groups and scenario concept", "— map design, spawn groups and scenario concept"),
("- WML implementation", "— WML implementation"),
),

"wesnoth-nr" : (
# Convert makeshift dashes:
# conversion added in 1.9.0-svn
("afar -- hence they kept their weapons sharp, and reckoned themselves well ", "afar — hence they kept their weapons sharp, and reckoned themselves well "),
("forgetting that their ancestors had been free - and might have completely ", "forgetting that their ancestors had been free — and might have completely "),
("lives of the people of Dwarven Doors - forever.", "lives of the people of Dwarven Doors — forever."),
("It was an early spring day like any other; the humans - joyless beneath the ", "It was an early spring day like any other; the humans — joyless beneath the "),
("orcish whip - were dispiritedly planting the yearly crop. All at once, the ", "orcish whip — were dispiritedly planting the yearly crop. All at once, the "),
("The orcs have gotten careless - look how easily I stole these weapons and ", "The orcs have gotten careless — look how easily I stole these weapons and "),
("help, unlooked for, arrived in the form of a small band of woodsrunners -- ", "help, unlooked for, arrived in the form of a small band of woodsrunners — "),
("of them to hell! Oh, and just between me and you, it is actually good fun - ", "of them to hell! Oh, and just between me and you, it is actually good fun — "),
("Tallin, this situation is hopeless - there are endless monsters swarming ", "Tallin, this situation is hopeless — there are endless monsters swarming "),
("We are in a dire situation indeed - but just see - the trolls and the ", "We are in a dire situation indeed — but just see — the trolls and the "),
("replacement - whereas for every monster we kill, it seems that two more come ", "replacement — whereas for every monster we kill, it seems that two more come "),
("creatures and they know their caves backwards and forwards - so I am sure at ", "creatures and they know their caves backwards and forwards — so I am sure at "),
("Och, the dwarves of Knalga are themselves in desperate straits - but we ", "Och, the dwarves of Knalga are themselves in desperate straits — but we "),
("this sorry state - where are they now? From what I have seen, it is mostly ", "this sorry state — where are they now? From what I have seen, it is mostly "),
("not before we surprised and slew their leader - the cursed Khazg Black-Tusk. ", "not before we surprised and slew their leader — the cursed Khazg Black-Tusk. "),
("after we were trapped here - by ones and twos in the beginning, and now by ", "after we were trapped here — by ones and twos in the beginning, and now by "),
("our axes. So, it is not their existence I fear - for we have faced far worse ", "our axes. So, it is not their existence I fear — for we have faced far worse "),
("trials - but their rising numbers gives cause for worry...", "trials — but their rising numbers gives cause for worry..."),
("the mines! Let the guardsmen stay behind along with the noncombatants - for ", "the mines! Let the guardsmen stay behind along with the noncombatants — for "),
("But my my, what do we have here - Tallin.", "But my my, what do we have here — Tallin."),
("minions - gorge on the flesh of these scurrying little rats!", "minions — gorge on the flesh of these scurrying little rats!"),
("Tallin. He's lucky, he is - or he makes his own luck.", "Tallin. He's lucky, he is — or he makes his own luck."),
("Black-Tusk, but we survived the orcs and trolls -- only to be captured by ", "Black-Tusk, but we survived the orcs and trolls — only to be captured by "),
("What was that? Oh, woe - two big slabs of rock cutting off our retreat!", "What was that? Oh, woe — two big slabs of rock cutting off our retreat!"),
("Well, if you don't mind me saying - that certainly isn't the state of ", "Well, if you don't mind me saying — that certainly isn't the state of "),
("Interesting. I wonder who - or what - could have created such a powerful ", "Interesting. I wonder who — or what — could have created such a powerful "),
("Thus, compelled by some strange and irresistible force, Tallin - eyes glazed ", "Thus, compelled by some strange and irresistible force, Tallin — eyes glazed "),
("and mind unthinking - did the will of his new master.", "and mind unthinking — did the will of his new master."),
("dwarves - it's because of you that we ha' made a start rebuilding Knalga in ", "dwarves — it's because of you that we ha' made a start rebuilding Knalga in "),
("I am here simply finishing the job Khazg Black-Tusk started years ago - the ", "I am here simply finishing the job Khazg Black-Tusk started years ago — the "),
("And I am grateful - but what should we do from here? The number of humans ", "And I am grateful — but what should we do from here? The number of humans "),
("Orcs are fickle; if we negotiated a treaty with them - assuming they would ", "Orcs are fickle; if we negotiated a treaty with them — assuming they would "),
("accept - it would just be broken as soon as the next warlord arose among ", "accept — it would just be broken as soon as the next warlord arose among "),
("I have heard all about him from my good friend Stalrag. He is - or was ", "I have heard all about him from my good friend Stalrag. He is — or was "),
("anyway, haven't heard from him in years - the chief o' the villages that lay ", "anyway, haven't heard from him in years — the chief o' the villages that lay "),
("As Tallin sat staring blankly at the body of his former friend - loathing ", "As Tallin sat staring blankly at the body of his former friend — loathing "),
("and hating what he had become - he received an urgent summons from his ", "and hating what he had become — he received an urgent summons from his "),
("Pew! Nick of time too - I almost bought it there!", "Pew! Nick of time too — I almost bought it there!"),
("elves - for the sorceress was, in fact, a princess of the highest rank.", "elves — for the sorceress was, in fact, a princess of the highest rank."),
("our help in rescuing their Princess? You heard their message - those elves ", "our help in rescuing their Princess? You heard their message — those elves "),
("Well, suppose we do join up with the elves - assuming they will let us - and ", "Well, suppose we do join up with the elves — assuming they will let us — and "),
("Just our luck to be here when they're mustering a field force - probably to ", "Just our luck to be here when they're mustering a field force — probably to "),
("Later, princess, first let's get outta -- uh, let us take our leave of this ", "Later, princess, first let's get outta — uh, let us take our leave of this "),
("him, and so he fled. As he did so, visions of his dying friends - friends ", "him, and so he fled. As he did so, visions of his dying friends — friends "),
("that he had slain - flashed before his eyes, while their voices chided him ", "that he had slain — flashed before his eyes, while their voices chided him "),
("under his control, I have done unthinkable deeds - I have studied the most ", "under his control, I have done unthinkable deeds — I have studied the most "),
("never would have risked their lives - nay, if I were not there they wouldn't ", "never would have risked their lives — nay, if I were not there they wouldn't "),
("Ruler of Men to ensure peace, harmony and above all - justice. For hundreds ", "Ruler of Men to ensure peace, harmony and above all — justice. For hundreds "),
("Tallin, accompanied by the ghost of the king - whose name he soon discovered ", "Tallin, accompanied by the ghost of the king — whose name he soon discovered "),
("to be Abhai - set off down the tunnels in search of the Rod of Justice. ", "to be Abhai — set off down the tunnels in search of the Rod of Justice. "),
("The Amulet of Strength, how fascinating. However - though it may be a useful ", "The Amulet of Strength, how fascinating. However — though it may be a useful "),
("place - nothing there but a bunch of bats.", "place — nothing there but a bunch of bats."),
("I found a way to the surface - that is if you feel like climbing straight up ", "I found a way to the surface — that is if you feel like climbing straight up "),
("We do not wish to fight you, Great One, we simply seek the Rod of Justice - ", "We do not wish to fight you, Great One, we simply seek the Rod of Justice — "),
("countless foes and raised countless generations of drakes - and now I am ", "countless foes and raised countless generations of drakes — and now I am "),
("The only problem - or the major one, anyway - is these blasted orcs. They ", "The only problem — or the major one, anyway — is these blasted orcs. They "),
("determined to make Knalga into an orcish stronghold - enslaving or killing ", "determined to make Knalga into an orcish stronghold — enslaving or killing "),
("There is a large elvish force not far from here who - we believe anyway - ", "There is a large elvish force not far from here who — we believe anyway — "),
("hesitated and found himself unable to proceed. Abhai came forth - his ", "hesitated and found himself unable to proceed. Abhai came forth — his "),
("defenders - and fearlessly lifted the Rod in his ghostly hands. He paused ", "defenders — and fearlessly lifted the Rod in his ghostly hands. He paused "),
("Tallin raised his eyes to look into Abhai's and Abhai met his gaze - ", "Tallin raised his eyes to look into Abhai's and Abhai met his gaze — "),
("your doubts at rest.' Tallin held the gaze for one more long moment - and ", "your doubts at rest.' Tallin held the gaze for one more long moment — and "),
("Yes, Tallin - it is I.", "Yes, Tallin — it is I."),
("How did you get here - I thought you you were dead... and for that matter, ", "How did you get here — I thought you you were dead... and for that matter, "),
("of unimaginable splendor, glory and transcendence - the very realm of the ", "of unimaginable splendor, glory and transcendence — the very realm of the "),
("They are all here, princess! Hand picked by your father - the finest and ", "They are all here, princess! Hand picked by your father — the finest and "),
("They are all here, princess! Hand picked by your father - the finest and ", "They are all here, princess! Hand picked by your father — the finest and "),
("and leave them a small guard. Then they pressed onward to rescue Hidel - and ", "and leave them a small guard. Then they pressed onward to rescue Hidel — and "),
("Cheer up -- you won't have to live with your failure for long...*snicker*", "Cheer up — you won't have to live with your failure for long...*snicker*"),
("have a few scores to settle with you. Take them, troops - I want no orc left ", "have a few scores to settle with you. Take them, troops — I want no orc left "),
("been crushed. This relieves a blight on our land - but if we do not address ", "been crushed. This relieves a blight on our land — but if we do not address "),
("what race they come from - even orcs.", "what race they come from — even orcs."),
("Tallin, as you know, I have been separated from my race and kinsmen - well, ", "Tallin, as you know, I have been separated from my race and kinsmen — well, "),
("except Eryssa - for many years now. I wish to go back to the elvish forests ", "except Eryssa — for many years now. I wish to go back to the elvish forests "),
("Lords of Light - or Darkness - guide you on your path. For those of you who ", "Lords of Light — or Darkness — guide you on your path. For those of you who "),
("are staying - come, we have much to do.", "are staying — come, we have much to do."),
("- and unknown - world to trade with the Dwarves of Knalga.", "— and unknown — world to trade with the Dwarves of Knalga."),
("Thus, from a small, enslaved community, the people of Dwarven Doors - by ", "Thus, from a small, enslaved community, the people of Dwarven Doors — by "),
("their fortitude, valor, and wisdom - brought the Northlands out of the ", "their fortitude, valor, and wisdom — brought the Northlands out of the "),
),

"wesnoth-thot" : (
# Convert makeshift dashes:
# conversion added in 1.9.0-svn
("then living -- all", "then living — all"),
("the Hammer -- dropped", "the Hammer — dropped"),
("to my eyes - an", "to my eyes — an"),
("captain -- but", "captain — but"),
("prisoners - which", "prisoners — which"),
("seen again - I began", "seen again — I began"),
("tightly locked - and, I", "tightly locked — and, I")
),

"wesnoth-trow" : (
# Convert makeshift dashes:
# conversion added in 1.9.0-svn
("Nay! Off with your hea- - -", "Nay! Off with your hea—"),
("is time - but I'll", "is time — but I'll"),
("SE - The River Road.", "SE — The River Road."),
("SW - The", "SW — The"),
("SW - Southbay.", "SW — Southbay."),
("Hold - I see", "Hold — I see"),
("The River Road -", "The River Road —"),
("I'm a tinkin- -", "I'm a tinkin—"),
("NW - Southbay", "NW — Southbay"),
# Also, "Like" -> "like"
("More Like NW - Every orc", "More like NW — Every orc"),
("Rarlg - argh", "Rarlg — argh"),
("Sewer - Danger", "Sewer — Danger"),
("Legend has it- -", "Legend has it—"),
("your fate you- -", "your fate you—"),
("Compan- - What?", "Compan— What?"),
("your services again- -", "your services again—"),
("people - to ally", "people — to ally"),
("meet your son -", "meet your son —"),
("- As we agreed.", "— As we agreed."),
("and your people -", "and your people —"),
("their aid at all -", "their aid at all —"),
("me - I'll be dead", "me — I'll be dead"),
("to say it - but", "to say it — but"),
("is ridiculous! I -", "is ridiculous! I —"),
("all I need - prepare", "all I need — prepare"),
("much -- from both", "much — from both"),
("a Lord -- to join", "a Lord — to join"),
("best of times - so", "best of times — so"),
),

"wesnoth-tsg" : (
# Convert makeshift dashes:
# conversion added in 1.9.0-svn
("My lord! The dawn is breaking - now is the time for us to attack and drive ", "My lord! The dawn is breaking — now is the time for us to attack and drive "),
("reinforcements - Did Sir Loris send you from Westin?", "reinforcements — Did Sir Loris send you from Westin?"),
("We will do our best to hold the city - you lead your men across the river ", "We will do our best to hold the city — you lead your men across the river "),
("Westin has fallen! This is a problem too great for me to handle - I must ", "Westin has fallen! This is a problem too great for me to handle — I must "),
("I have lost two brothers to Deoran - I shall not allow him to reach the ", "I have lost two brothers to Deoran — I shall not allow him to reach the "),
("My mount will not help me in these rocky paths - I will leave him here at ", "My mount will not help me in these rocky paths — I will leave him here at "),
("Also, soldiers with the -quick- trait will be useful in the dark.", "Also, soldiers with the — quick — trait will be useful in the dark."),
("We have escaped the great forest, but look! The undead are chasing us - we ", "We have escaped the great forest, but look! The undead are chasing us — we "),
("I too will make my stand here - I owe you my life and must atone for my ", "I too will make my stand here — I owe you my life and must atone for my "),
("Minister Hylas, ride with me to Westin - you must summon the Council of ", "Minister Hylas, ride with me to Westin — you must summon the Council of "),
("Far from home I fall - but not in vain! Guard our people, Deoran!", "Far from home I fall — but not in vain! Guard our people, Deoran!"),
("when they have an ally directly behind the unit they are attacking - they'll ", "when they have an ally directly behind the unit they are attacking — they'll "),
("magic. They are slow and weak - any of your units is more than a match for ", "magic. They are slow and weak — any of your units is more than a match for "),
("Finally Ethiliel told the men to halt and pitch camp - they had reached the ", "Finally Ethiliel told the men to halt and pitch camp — they had reached the "),

),

"wesnoth-sof" : (
# Convert makeshift dashes:
# conversion added in 1.9.0-svn
("So now I tell from whence it came -", "So now I tell from whence it came —"),
("The Fire-sceptre great -", "The Fire-sceptre great —"),
("Uh, no, wait! Lets talk business - how much will you pay us to do this for ", "Uh, no, wait! Lets talk business — how much will you pay us to do this for "),
("Fine then - ten thousand... now, what exactly do you want us to make the ", "Fine then — ten thousand... now, what exactly do you want us to make the "),
("Hey! You can't do that - this road is an elvish road. We're not letting you ", "Hey! You can't do that — this road is an elvish road. We're not letting you "),
("Ah, I see - you are one of the rebels. Our treaty with the elves does not ", "Ah, I see — you are one of the rebels. Our treaty with the elves does not "),
("We will see about that - if you ever get underground, which I doubt. Ha!", "We will see about that — if you ever get underground, which I doubt. Ha!"),
("Here - I'll go warn the council. You stay here and fight.", "Here — I'll go warn the council. You stay here and fight."),
("dwarf-made stonecraft. We refuse - now let's close these gates!", "dwarf-made stonecraft. We refuse — now let's close these gates!"),
("Just watch. The gates wi' close very soon. Then the elves outside - and, ", "Just watch. The gates wi' close very soon. Then the elves outside — and, "),
("unfortunately, our dwarves who are still out there - wi' become irrelevant.", "unfortunately, our dwarves who are still out there — wi' become irrelevant."),
("for the runesmith named Thursagan - the sage of fire.", "for the runesmith named Thursagan — the sage of fire."),
("members of their party - one expected, and one not.", "members of their party — one expected, and one not."),
("In the treasury. And leave it there until you're ready to work with it - I ", "In the treasury. And leave it there until you're ready to work with it — I "),
("the finest coal, and many of the finest jewels in the land - we'll need ", "the finest coal, and many of the finest jewels in the land — we'll need "),
("to fight them; also be prepared to spend quite some time here - mining can ", "to fight them; also be prepared to spend quite some time here — mining can "),
("Yes, although we will have to hire the miners - they don't work for free. ", "Yes, although we will have to hire the miners v they don't work for free. "),
("were others - many others. Thus I present Theganli, the jeweler. His role is ", "were others — many others. Thus I present Theganli, the jeweler. His role is "),
("were no exception. They were crafters of crafters - they made tools. The ", "were no exception. They were crafters of crafters — they made tools. The "),
("How about this - I help you defeat them, and you let me keep the ruby when ", "How about this — I help you defeat them, and you let me keep the ruby when "),
("Well, back to the battle - we are all in the caves, but there are still ", "Well, back to the battle — we are all in the caves, but there are still "),
("Well, back to the battle - we need everyone to get into the Shorbear caves. ", "Well, back to the battle — we need everyone to get into the Shorbear caves. "),
("If ye'll permit me to say so, sir, ye're wrong. We could - ", "If ye'll permit me to say so, sir, ye're wrong. We could — "),
("If you'll permit me to say so, sir, you're wrong. We could - ", "If you'll permit me to say so, sir, you're wrong. We could — "),
("went south - back to the Wesnoth border.", "went south — back to the Wesnoth border."),
("And thus Rugnur died - a glorious death, in the eyes of the dwarven sages. ", "And thus Rugnur died — a glorious death, in the eyes of the dwarven sages. "),
("And Krawg - well, Krawg followed him. I know not how. But Krawg made his way ", "And Krawg — well, Krawg followed him. I know not how. But Krawg made his way "),
("But before it was found, legends grew up around it, and around its makers - ", "But before it was found, legends grew up around it, and around its makers — "),
),

"wesnoth-sotbe" :(
# Convert makeshift dashes:
# conversion added in 1.9.0-svn
("the offer. However, on the way to the city of Dwarven Doors - the ", "the offer. However, on the way to the city of Dwarven Doors — the "),
("headquarters of the Northern Alliance - Karun and his small group of ", "headquarters of the Northern Alliance — Karun and his small group of "),
("The peace treaty still held however, partially because the treaty - although ", "The peace treaty still held however, partially because the treaty — although "),
("arranged by the Northern Alliance - was between the local earls and the ", "arranged by the Northern Alliance — was between the local earls and the "),
("Alliance. The other - and perhaps the main - reason why the treaty held was ", "Alliance. The other — and perhaps the main — reason why the treaty held was "),
("followed were relatively peaceful and prosperous - at least, for the humans.", "followed were relatively peaceful and prosperous — at least, for the humans."),
("In the thirteenth year of the Lord Protectorship of Howgarth III - the ", "In the thirteenth year of the Lord Protectorship of Howgarth III — the "),
("successor of Rahul I - tension began to rise between orcish tribes and human ", "successor of Rahul I — tension began to rise between orcish tribes and human "),
("on their lands. Then, Earl Lanbec'h - the most powerful human warlord of the ", "on their lands. Then, Earl Lanbec'h — the most powerful human warlord of the "),
("North - determined to abolish the orcish menace raised an army and conferred ", "North — determined to abolish the orcish menace raised an army and conferred "),
("Who is this unlicked whelp? Grunts - kill him and bring me his head!", "Who is this unlicked whelp? Grunts — kill him and bring me his head!"),
("Wise decision, Kapou'e. By the size of that army, this is no mere raid - it ", "Wise decision, Kapou'e. By the size of that army, this is no mere raid — it "),
("And this is that stupid human who dares march against Kapou'e - Son of the ", "And this is that stupid human who dares march against Kapou'e — Son of the "),
("Hey, Chief, I was thinking - dwarves are pretty slow; why don't we just ", "Hey, Chief, I was thinking — dwarves are pretty slow; why don't we just "),
("As soon as the sandstorm died down, Kapou'e and his men - eager to escape ", "As soon as the sandstorm died down, Kapou'e and his men — eager to escape "),
("the searing heat - left the oasis and continued on their trek.", "the searing heat — left the oasis and continued on their trek."),
("and all the natural sounds of the forest died away - leaving everything ", "and all the natural sounds of the forest died away — leaving everything "),
("Leave your people here Kapou'e - they are safe for the moment - and go ", "Leave your people here Kapou'e — they are safe for the moment — and go "),
("friends - get them!", "friends — get them!"),
("After a fair amount of squabbling - for some of the older warlords were ", "After a fair amount of squabbling — for some of the older warlords were "),
("reluctant to let this young upstart lead them - and a few consequent ", "reluctant to let this young upstart lead them — and a few consequent "),
("Kapou'e placed one force under the leadership of Shan Taum the Smug, who - ", "Kapou'e placed one force under the leadership of Shan Taum the Smug, who — "),
("despite his obnoxious nature - was a fierce and capable leader. He was sent ", "despite his obnoxious nature — was a fierce and capable leader. He was sent "),
("He sent the shamans, Pirk, Gork and Vraurk back to Borstep - a city just ", "He sent the shamans, Pirk, Gork and Vraurk back to Borstep — a city just "),
("north of the Mourned Hills - to organize any remaining orcish forces as well ", "north of the Mourned Hills — to organize any remaining orcish forces as well "),
("Kapou'e himself - desiring to settle this business once and for all - led ", "Kapou'e himself — desiring to settle this business once and for all — led "),
("The orcs are making headway. The city must not fall - call the reserves!", "The orcs are making headway. The city must not fall — call the reserves!"),
("fall. A few days after the first snowfall a goblin rider - half dead from ", "fall. A few days after the first snowfall a goblin rider — half dead from "),
("exhaustion - raced into Dorset with the news that Borstep was under siege by ", "exhaustion — raced into Dorset with the news that Borstep was under siege by "),
("give him a good chance to clobber the orcs together again - the old ", "give him a good chance to clobber the orcs together again — the old "),
("Kapou'e's exploits had been widely circulated - from his rescue of the ", "Kapou'e's exploits had been widely circulated — from his rescue of the "),
("horde was dispersed he appointed three more shamans - with the consent of ", "horde was dispersed he appointed three more shamans — with the consent of "),
("all - to make the Great Council complete again.", "all — to make the Great Council complete again."),
),

"wesnoth-tb" : (
# Convert makeshift dashes:
# conversion added in 1.9.0-svn
("A Tale of Two Brothers - Epilogue", "A Tale of Two Brothers — Epilogue"),
),

"wesnoth-tutorial" : (

),

"wesnoth-units" : (
# Convert makeshift dashes:
# conversion added in 1.9.0-svn
("allowed - grudgingly - to", "allowed — grudgingly — to"),
("horseback - in fact", "horseback — in fact"),
("is quite justified -", "is quite justified —"),
("their own race - this power", "their own race — this power"),
("archer - though the heft", "archer — though the heft"),
("dangerous enough - the deadliness", "dangerous enough — the deadliness"),
("in battle - if only", "in battle — if only"),
("accompany it - it is a sin", "accompany it — it is a sin"),
("rarely seen - standing", "rarely seen — standing"),
),

"wesnoth-utbs" : (
# Convert makeshift dashes:
# conversion added in 1.9.0-svn
("Strike hard and fast and also be careful--right, this is going to be fun.", "Strike hard and fast and also be careful—right, this is going to be fun."),
("There might be, but I don't--", "There might be, but I don't—"),
("You idiot--", "You idiot—"),
("unfortunately we found your men dead--", "unfortunately we found your men dead—"),
("Well, actually they were fleeing from--", "Well, actually they were fleeing from—"),
("Look, if you'll just let me explain--", "Look, if you'll just let me explain—"),
("This is--", "This is—"),
("This entire journey has been based on a lie--", "This entire journey has been based on a lie—"),
("the last few generations--if anything the land had grown even more", "the last few generations—if anything the land had grown even more"),
),

"1.8-announcement" : (
# conversion added shortly before 1.8.0, might be relevant for the 1.10.0 announcement
("WML events an AI components", "WML events and AI components"),
("1.7.3", "1.7.13"),
("/tags/1.8/", "/tags/1.8.0/"),
),
}

# Speak, if all argument files are newer than this timestamp
# Try to use UTC here
# date --utc "+%s  # %c"
timecheck = 1275358384  # Tue 01 Jun 2010 02:13:04 AM UTC

import os, sys, time, stat, re

if __name__ == '__main__':
    newer = 0
    modified = 0
    pocount = 0
    for path in sys.argv[1:]:
        if not path.endswith(".po") and not path.endswith(".pot") and not path.endswith(".cfg"):
            continue
        try:
            pocount += 1
            # Notice how many files are newer than the time check
            statinfo = os.stat(path)
            if statinfo.st_mtime > timecheck:
                newer += 1
            # Read the content of each file and transform it
            before = open(path, "r").read()
            after = before
            decommented = re.sub("#.*", "", before)
            for (domain, fixes) in stringfixes.items():
                for (old, new) in fixes:
                    if old is new:
                        #complain loudly
                        print "pofix: old string\n\t\"%s\"\n equals new string\n\t\"%s\"\nexiting." % (old, new)
                        sys.exit(1)
                    #this check is problematic and the last clause is added to prevent false
                    #positives in case that new is a substring of old, though this can also
                    #lead to "real" probs not found, the real check would be "does replacing
                    #old with new lead to duplicate msgids? (including old ones marked with #~)"
                    #which is not easily done in the current design...
                    elif new in decommented and old in decommented and not new in old:
                        print "pofix: %s already includes the new string\n\t\"%s\"\nbut also the old\n\t\"%s\"\nthis needs handfixing for now since it likely creates duplicate msgids." % (path, new, old)
                    else:
                        lines = after.split('\n')
                        for (i, line) in enumerate(lines):
                            if line and line[0] != '#':
                                lines[i] = lines[i].replace(old, new)
                        after = '\n'.join(lines)
            if after != before:
                print "pofix: %s modified" % path
                modified += 1
                # Save a backup
                os.rename(path, path + "-bak")
                # Write out transformed version
                ofp = open(path, "w")
                ofp.write(after)
                ofp.close()
        except OSError:
            print >>sys.stderr, "pofix: I can't see %s" % path
    print "pofix: %d files processed, %d files modified, %d files newer" \
          % (pocount, modified, newer)
    if pocount > 1 and newer == pocount:
        print "pofix: script may be obsolete"
