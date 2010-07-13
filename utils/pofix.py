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

# Convert makeshift dashes/straight apostrophes:
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
("don't have to - let it", "don't have to — let it"),
("- Sir Kaylan,", "― Sir Kaylan,"),
("- Princess Li'sar,", "― Princess Li’sar,"),
("your attacks - they will", "your attacks — they will"),
("- The Scroll of Chantal,", "― The Scroll of Chantal,"),
("- Great Sage Dacyn,", "― Great Sage Dacyn,"),
("- Queen Li'sar,", "― Queen Li’sar,"),
("- Meneldur,", "― Meneldur,"),
("upload statistics - Help", "upload statistics — Help"),
("(A) - auth command", "(A) — auth command"),
("(D) - debug only, (N) - network only, (A) - auth only", "(D) — debug only, (N) — network only, (A) — auth only"),
("not empty - duplicate", "not empty — duplicate"),
("- King Konrad,", "― King Konrad,"),
("Player Info -", "Player Info —"),
("About to upload statistics - Help us make Wesnoth better for you!", "About to upload statistics — Help us make Wesnoth better for you!"),
#the following rule applies to wesnoth/*.po* and to wesnoth-manual/*.po*
("victory objectives - getting", "victory objectives — getting"),

# Fix screw up
# conversion added in 1.9.0-svn
("— Great Mage Delfador", "― Great Mage Delfador"),
("— The Wesnoth Tactical Guide", "― The Wesnoth Tactical Guide"),
("— King Konrad, 536YW", "― King Konrad, 536YW"),
("— Memoirs of Gweddry, 627YW", "― Memoirs of Gweddry, 627YW"),
("— High Lord Kalenz, 470YW", "― High Lord Kalenz, 470YW"),
("— The Wesnoth Community", "― The Wesnoth Community"),
("— Sir Kaylan,", "― Sir Kaylan,"),
("— Princess Li'sar,", "― Princess Li’sar,"),
("— The Scroll of Chantal,", "― The Scroll of Chantal,"),
("— Great Sage Dacyn,", "― Great Sage Dacyn,"),
("— Queen Li'sar,", "― Queen Li’sar,"),
("— King Konrad,", "― King Konrad,"),
("— Meneldur,", "― Meneldur,"),
("― Princess Li'sar,", "― Princess Li’sar,"),
("― Queen Li'sar,", "― Queen Li’sar,"),

# Straight apostrophes and quotes to curly ones
# conversion added in 1.9.0-svn
("Ga'ash", "Ga’ash"),
("Gart'lo", "Gart’lo"),
("Mar'Ildian", "Mar’Ildian"),
("Marra Di'lek", "Marra Di’lek"),
("Bzz'Kza", "Bzz’Kza"),
("unit's", "unit’s"),
("side's", "side’s"),
("man's", "man’s"),
("player's", "player’s"),
("elf's", "elf’s"),
("turn's", "turn’s"),
("it's best to click the", "it’s best to click the"),
("Don't send", "Don’t send"),

# Fix capitalization
# conversion added in 1.9.0-svn
("Icelandic translation", "Icelandic Translation"),
("Miscellaneous contributors", "Miscellaneous Contributors"),
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
# Straight apostrophes and quotes to curly ones
# conversion added in 1.9.0-svn
("Wesmere's furthermost-faring scouts.", "Wesmere’s furthermost-faring scouts."),
("Two days' travel later, the forward scouts", "Two days’ travel later, the forward scouts"),
("Use Elven Scouts and Linaera's power of teleportation", "Use Elven Scouts and Linaera’s power of teleportation"),
("It's of human design...but we", "It’s of human design...but we"),
("Do it. We'll have a guest soon.", "Do it. We’ll have a guest soon."),
("Lord... I'm... I am filled with grief.", "Lord... I’m... I am filled with grief."),
("else we'll never have peace again.", "else we’ll never have peace again."),
("We will come in numbers... (*cough*) You can't imagine..", "We will come in numbers... (*cough*) You can’t imagine.."),
("I'll be waiting... Among the dead...", "I’ll be waiting... Among the dead..."),
("It's done, lord. No-one escaped. No-one", "It’s done, lord. No-one escaped. No-one"),
("we won't find much forage on the march.", "we won’t find much forage on the march."),
("We can't carry on Lord, the men are to tired.", "We can’t carry on Lord, the men are to tired."),
("we'll try again when reinforcements arrive.", "we’ll try again when reinforcements arrive."),
("planning an invasion, I'm sure of it.", "planning an invasion, I’m sure of it."),
("The sun's fully over the horizon.", "The sun’s fully over the horizon."),
("We'll rest a bit on the other side;", "We’ll rest a bit on the other side;"),
("they won't take long to rally.", "they won’t take long to rally."),
("No! This can't be!", "No! This can’t be!"),
("We can't get through, my Lord.", "We can’t get through, my Lord."),
("Never in my life did I dream I'd be bested by mere trolls.", "Never in my life did I dream I’d be bested by mere trolls."),
("We'll wait for reinforcements.", "We’ll wait for reinforcements."),
("Haldric's", "Haldric’s"),
("believe it's a", "believe it’s a"),
("since then they've been", "since then they’ve been"),
("It's hopeless; we've tried everything, and they're still coming back.", "It’s hopeless; we’ve tried everything, and they’re still coming back."),
("There's", "There’s"),
("we're", "we’re"),
("Lord Erlornas didn't drive", "Lord Erlornas didn’t drive"),
("I've been bested, but the combat wasn't fair", "I’ve been bested, but the combat wasn’t fair"),
("I'll have some answers", "I’ll have some answers"),
("let's focus on the task at hand", "let’s focus on the task at hand"),
("We don't want any more undesirables", "We don’t want any more undesirables"),
("Lord... I'm... I am filled with grief", "Lord... I’m... I am filled with grief"),
("else we'll never have peace again", "else we’ll never have peace again"),
("You can't imagine", "You can’t imagine"),
("I'll be waiting", "I’ll be waiting"),
("It's done, lord. No-one escaped. No-one tried to escape. I'm... disturbed", "It’s done, lord. No-one escaped. No-one tried to escape. I’m... disturbed"),
("we'll move out at dawn", "we’ll move out at dawn"),
("we won't find much forage", "we won’t find much forage"),
("We can't carry on Lord", "We can’t carry on Lord"),
("we'll try again when reinforcements arrive", "we’ll try again when reinforcements arrive"),
("I'm sure of it", "I’m sure of it"),
("The sun's fully over the horizon", "The sun’s fully over the horizon"),
("We'll rest a bit on the other side", "We’ll rest a bit on the other side"),
("they won't take long to rally", "they won’t take long to rally"),
("No! This can't be!", "No! This can’t be!"),
("We can't get through", "We can’t get through"),
("I dream I'd be bested", "I dream I’d be bested"),
("We'll wait for reinforcements", "We’ll wait for reinforcements"),
("not frequented even by Wesmere's", "not frequented even by Wesmere’s"),
("Two days' travel later", "Two days’ travel later"),
("Linaera's power", "Linaera’s power"),
("It's of human design", "It’s of human design"),
("We'll have a guest soon", "We’ll have a guest soon"),
("Without Linaera's help", "Without Linaera’s help"),
("The Ka'lian has deliberated", "The Ka’lian has deliberated"),
("they're tired and afraid", "they’re tired and afraid"),
("I'm... disturbed", "I’m... disturbed"),
),

"wesnoth-did" : (
# Convert makeshift dashes:
# conversion added in 1.9.0-svn
("A Small Favor -", "A Small Favor —"),
("running away - my horsemen", "running away — my horsemen"),

# Convert straight apostrophes/quotation marks
# conversion added in 1.9.0-svn
("Kreg'a'shar", "Kreg’a’shar"),
("Parthyn's", "Parthyn’s"),
("orcs'", "orcs’"),
("'Allow me to introduce", "“Allow me to introduce"),
("town for a few days.'", "town for a few days.”"),
("'Surely you know that", "“Surely you know that"),
("only in dark magic.'", "only in dark magic.”"),
("You won't truly banish", "You won’t truly banish"),
("I've no wish to", "I’ve no wish to"),
("you've come", "you’ve come"),
("I won't spare", "I won’t spare"),
("Three days' travel", "Three days’ travel"),
("T'shar", "T’shar"),
("Don't say", "Don’t say"),
("it's ridiculous", "it’s ridiculous"),
("I don't see any. Maybe it's", "I don’t see any. Maybe it’s"),
("'zed'", "‘zee’"), # Use the American spelling; philosopher's quotes are being used here
("So, I've finally", "So, I’ve finally"),
("he's threatening", "he’s threatening"),
("It's time he learned", "It’s time he learned"),
("I've been itching", "I’ve been itching"),
("I'm ready", "I’m ready"),
("transformation they'll begin", "transformation they’ll begin"),
("I won't go down", "I won’t go down"),
("I won't see them", "I won’t see them"),
("orc's", "orc’s"),
("'The spells of necromancy", "“The spells of necromancy"),
("spirit world.'", "spirit world.”"),
("'To become a lich, one must first die.'", "“To become a lich, one must first die.”"),
("Malin's", "Malin’s"),
("I've just got", "I’ve just got"),
("We'll see", "We’ll see"),
("when they didn't", "when they didn’t"),
("You can't", "You can’t"),
("What's in it", "What’s in it"),
("Karres's", "Karres’s"),
("Let's get", "Let’s get"),
("bats won't stand", "bats won’t stand"),
("I'm eager to", "I’m eager to"),
("if ye dinna' want tae be a walking pile o'", "if ye dinna’ want tae be a walking pile o’"),
("they don't understand", "they don’t understand"),
("I've got the rest", "I’ve got the rest"),
("Gron'r Hronk", "Gron’r Hronk"),
("K'rrlar Oban", "K’rrlar Oban"),
("doesn't look very", "doesn’t look very"),
("lake's", "lake’s"),
("'They are quite useful in battle,'", "“They are quite useful in battle,”"),
("'but none of them have even a tenth of your potential power.'", "“but none of them have even a tenth of your potential power.”"),
("P'Gareth", "P’Gareth"),
("K'Vark", "K’Vark"),
("he's escaping", "he’s escaping"),
("Drogan's", "Drogan’s"),
("'A life curse goes beyond a joke,'", "“A life curse goes beyond a joke,”"),
("'Poor judgment,'", "“Poor judgment,”"),
("I'll have my", "I’ll have my"),
("'For your final test", "“For your final test"),
("retrieving a book,'", "retrieving a book,”"),
("'The book was", "‘The book was"),
("it from me.'", "it from me.”"),
("'They are no", "“They are no"),
("twice now.'", "twice now.”"),
("'Excellent. We travel", "“Excellent. We travel"),
("book inside.'", "book inside.”"),
("Mage Lord's", "Mage Lord’s"),
("mage lord's", "mage lord’s"),
("won't hold back", "won’t hold back"),
("We've got", "We’ve got"),
("you aren't leaving", "you aren’t leaving"),
("now you've given", "now you’ve given"),
("you've got", "you’ve got"),
("humankind's", "humankind’s"),
("I'm not ready to die", "I’m not ready to die"),
),

"wesnoth-dm" : (
# Convert makeshift dashes:
# conversion added in 1.9.0-svn
("warn you - a party", "warn you — a party"),
("each other - and you'll", "each other — and you’ll"),
("Night is falling - that's", "Night is falling — that’s"),
("work by now - I did not", "work by now — I did not"),
("seeking you - you see", "seeking you — you see"),
("Of course - do you", "Of course — do you"),
("Knalga - the rumor", "Knalga — the rumor"),
("Worse news - the", "Worse news — the"),
("been to the west - will the", "been to the west — will the"),
("the dead - should", "the dead — should"),
("Illuven - lesser", "Illuven — lesser"),
("need protection - cost", "need protection — cost"),
("No thanks - we'll manage by ourselves...", "No thanks — we’ll manage by ourselves..."),
("Let's move on - the less", "Let’s move on — the less"),
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
("No - you can't be dead!", "No — you can’t be dead!"),
("of our help too - this", "of our help too — this"),

# Fix screw up
# conversion added in 1.9.0-svn
("each other — and you'll", "each other — and you’ll"),
("Night is falling — that's", "Night is falling — that’s"),
("No thanks — we'll manage by ourselves...", "No thanks — we’ll manage by ourselves..."),
("Let's move on — the less", "Let’s move on — the less"),
("No — you can't be dead!", "No — you can’t be dead!"),

# Correct capitalization
# conversion added in 1.9.0-svn
("Clash at the manor", "Clash at the Manor"),
("Shadows in the dark", "Shadows in the Dark"),
("Face of the enemy", "Face of the Enemy"),

# Straight apostrophes and quotes to curly ones
# conversion added in 1.9.0-svn
("Delfador's Memoirs", "Delfador’s Memoirs"),
("'The Great'", "“The Great”"),
("Don't die!", "Don’t die!"),
("Methor's", "Methor’s"),
("I don't like", "I don’t like"),
("I've told you", "I’ve told you"),
("father's", "father’s"),
("After a night's rest", "After a night’s rest"),
("And if it's archers you need", "And if it’s archers you need"),
("Leollyn's", "Leollyn’s"),
("king's", "king’s"),
("Lionel's", "Lionel’s"),
("I'm honored that", "I’m honored that"),
("Here's", "Here’s"),
("It's been a pleasure", "It’s been a pleasure"),
("You'll", "You’ll"),
("I think that's all", "I think that’s all"),
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
("I aint charging gold -", "I ain’t charging gold —"),
("'T'aint safe", "’T’ain’t safe"),
# Make it unspaced...
# conversion added in 1.9.0-svn
("may be able to help us in - ", "may be able to help us in—"),
("Wait - what just happened?", "Wait — what just happened?"),

# Fix screw up
# conversion added in 1.9.0-svn
("I ain't charging gold —", "I ain’t charging gold —"),
("'T'ain't safe", "’T’ain’t safe"),

# Enforce "-gue" spelling; include the quotation marks so that pofix won't find
# "Epilogue" thinking that it is "Epilog"
# conversion added in 1.9.0-svn
("\"Epilog\"", "\"Epilogue\""),
),

"wesnoth-httt" : (
# Convert makeshift dashes:
# conversion added in 1.9.0-svn
("and the support of my men - from", "and the support of my men — from"),
("NE - Dan'Tonk", "NE — Dan’Tonk"),
("SE - Fort Tahn", "SE — Fort Tahn"),
("The Valley of Death - The Princess's Revenge", "The Valley of Death — The Princess’s Revenge"),
("the hills - there are undead about!", "the hills — there are undead about!"),
("those gryphon eggs - they", "those gryphon eggs — they"),
("- Delfador's insistence", "— Delfador’s insistence"),
("Look - orcs are", "Look — orcs are"),
("A frail human - or worse, an elf -", "A frail human — or worse, an elf —"),
("out to the heir - I", "out to the heir — I"),
("gruesome sight - a fleet", "gruesome sight — a fleet"),
("introduce myself - I", "introduce myself — I"),
("my warning - prepare", "my warning — prepare"),
("princess - the heiress", "princess — the heiress"),
("don't try to fight us - you", "don’t try to fight us — you"),
("Princess Li'sar - here?", "Princess Li’sar — here?"),
("Look - you can", "Look — you can"),
("century - a generation", "century — a generation"),
("vast human army - his", "vast human army — his"),

# Fix screw up
# conversion added in 1.9.0-svn
("NE — Dan'Tonk", "NE — Dan’Tonk"),
("The Valley of Death — The Princess's Revenge", "The Valley of Death — The Princess’s Revenge"),
("— Delfador's insistence", "— Delfador’s insistence"),
("don't try to fight us — you", "don’t try to fight us — you"),
("Princess Li'sar — here?", "Princess Li’sar — here?"),
),

"wesnoth-l" : (
# Typo fix at r44124
("devestating", "devastating"),
# Convert makeshift dashes and other stuff:
# conversion added in 1.9.0-svn
("are on the hunt - and", "are on the hunt — and"),
("and ruthlessness - and their", "and ruthlessness — and their"),
("of death - would only", "of death — would only"),
("my father - your grandfather - brought", "my father — your grandfather — brought"),
("catacombs - cover me.", "catacombs — cover me."),
("Liberty - Epilogue", "Liberty — Epilogue"),
("'If you wish to find us, head southwest. When you reach the land's end, just keep going. See you in the Three Sisters, old friend. - Relnan'", "“If you wish to find us, head southwest. When you reach the land’s end, just keep going. See you in the Three Sisters, old friend. ― Relnan”"),
# Fix screw up
# conversion added in 1.9.0-svn
("'If you wish to find us, head southwest. When you reach the land's end, just keep going. See you in the Three Sisters, old friend. — Relnan'", "“If you wish to find us, head southwest. When you reach the land’s end, just keep going. See you in the Three Sisters, old friend. ― Relnan”"),
# Straight apostrophes and quotes to curly ones
# conversion added in 1.9.0-svn
("By the sound of the wolves, the scent trail of Fal Khag's killers ends here.", "By the sound of the wolves, the scent trail of Fal Khag’s killers ends here."),
("But when the effects fall on mere peasants, the wrong of it may not seem so obvious from a noble's chair.", "But when the effects fall on mere peasants, the wrong of it may not seem so obvious from a noble’s chair."),
("Then they'll be back in force.", "Then they’ll be back in force."),
("Well, it won't be long until they report back to the local garrison with the details of your encounter.", "Well, it won’t be long until they report back to the local garrison with the details of your encounter."),
("In your many years as Delwyn's magistrate I have never known your judgment to err.", "In your many years as Delwyn’s magistrate I have never known your judgment to err."),
("there would be no help for the villagers, ground beneath the wheels of Asheviere's wrath.", "there would be no help for the villagers, ground beneath the wheels of Asheviere’s wrath."),
("After more than a week of fierce fighting, the main body of Asheviere's", "After more than a week of fierce fighting, the main body of Asheviere’s"),
("I think it's working!", "I think it’s working!"),
("It's done.", "It’s done."),
("I'm inside the tower! I'm going", "I’m inside the tower! I’m going"),
("I think... I think they're trying to storm Halstead itself... the fools!", "I think... I think they’re trying to storm Halstead itself... the fools!"),
("Baldras, Gwydion is Lord Maddock's son. This battle isn't just about Annuvin anymore.", "Baldras, Gwydion is Lord Maddock’s son. This battle isn’t just about Annuvin anymore."),
("They're in for a surprise.", "They’re in for a surprise."),
("They must think Wesnoth's army is spread out right now.", "They must think Wesnoth’s army is spread out right now."),
("I hope you're right.", "I hope you’re right."),
("I don't see how we can bring it down.", "I don’t see how we can bring it down."),
("the base of Halstead's frozen waves of stone.", "the base of Halstead’s frozen waves of stone."),
("from the earth's living rock.", "from the earth’s living rock."),
("338 years before Asheviere's betrayal,", "338 years before Asheviere’s betrayal,"),
("I'm the crazy one. Let's finish this folly.", "I’m the crazy one. Let’s finish this folly."),
("I haven't run this much in years.", "I haven’t run this much in years."),
("They've seen us, RUN!", "They’ve seen us, RUN!"),
("we're all in big trouble...", "we’re all in big trouble..."),
("so if we're careful enough we", "so if we’re careful enough we"),
("They're looking for us.", "They’re looking for us."),
("I'm not sure where we're supposed to go now.", "I’m not sure where we’re supposed to go now."),
("from sleep... we're not exactly sure.", "from sleep... we’re not exactly sure."),
("We don't. You have sought us out.", "We don’t. You have sought us out."),
("The former King's magic ministry", "The former King’s magic ministry"),
("It's not noble work, but our pursuits require plenty of gold.", "It’s not noble work, but our pursuits require plenty of gold."),
("Rest well tonight, because tomorrow's battle", "Rest well tonight, because tomorrow’s battle"),
("It's an ambush!", "It’s an ambush!"),
("I am seen so far from my Lord's borders.", "I am seen so far from my Lord’s borders."),
("It's this or nothing.", "It’s this or nothing."),
("You just said we can't beat their entire army!", "You just said we can’t beat their entire army!"),
("They mustn't take one step without", "They mustn’t take one step without"),
("What's worse is that she appears to", "What’s worse is that she appears to"),
("'We fled like criminals in the night, but we made sure that no one would harass us anymore.'", "“We fled like criminals in the night, but we made sure that no one would harass us anymore.”"),
("'Baldras, You would have been proud. We gave 'em hell. But in the end, it wasn't enough.'", "“Baldras, You would have been proud. We gave ’em hell. But in the end, it wasn’t enough.”"),
("with bitter humor he realized that Lord Maddock's men", "with bitter humor he realized that Lord Maddock’s men"),
("convinced Asheviere's second in command", "convinced Asheviere’s second in command"),
("As the main body of Asheviere's army", "As the main body of Asheviere’s army"),
("The spectacle of Halstead's destruction stunned them into", "The spectacle of Halstead’s destruction stunned them into"),
("Indeed, Asheviere's armies", "Indeed, Asheviere’s armies"),
("a second time, we're going to", "a second time, we’re going to"),
("I don't understand.", "I don’t understand."),
("Unless you want me to round up the city's", "Unless you want me to round up the city’s"),
("the King's son and betrayer.", "the King’s son and betrayer."),
("Queen's", "Queen’s"),
("the King's rule", "the King’s rule"),
("But we'll need to chase them all down if we're going to stop them.", "But we’ll need to chase them all down if we’re going to stop them."),
("And Harper... don't get yourself killed. I'm responsible for you now that your father is gone, and I won't dishonor his memory by breaking my promise to keep you safe.", "And Harper... don’t get yourself killed. I’m responsible for you now that your father is gone, and I won’t dishonor his memory by breaking my promise to keep you safe."),
("All right now, let's", "All right now, let’s"),
("They're getting closer", "They’re getting closer"),
("They'll come out when they see those riders, or us, approaching, but there aren't many of them... I wouldn't count on them being able to protect the village alone for long.", "They’ll come out when they see those riders, or us, approaching, but there aren’t many of them... I wouldn’t count on them being able to protect the village alone for long."),
("What's happening here?", "What’s happening here?"),
("Some mages are thrown out of the mage's", "Some mages are thrown out of the mage’s"),
("In the process they pick up a good deal of the Knight's", "In the process they pick up a good deal of the Knight’s"),
("take responsibility for the community's", "take responsibility for the community’s"),
("the village's wisest and most", "the village’s wisest and most"),
("but as Asheviere's grip", "but as Asheviere’s grip"),
("I don't want to touch it.", "I don’t want to touch it."),
("With bitter humor he realized that Lord Maddock's", "With bitter humor he realized that Lord Maddock’s"),
),

"wesnoth-lib" : (
# Convert makeshift dashes
# conversion added in 1.9.0-svn
("Player Info - ", "Player Info — "),
),

"wesnoth-low" : (
# Spelling fixes required at r44124
("engaged wit the defenders", "engaged with the defenders"),
# Convert makeshift dashes:
# conversion added in 1.9.0-svn
("follow you, Kalenz - but", "follow you, Kalenz — but"),
("Kalenz - lead us", "Kalenz — lead us"),
("them aid - it's clear", "them aid — it’s clear"),

# Fix screw up
# conversion added in 1.9.0-svn
("them aid — it's clear", "them aid — it’s clear"),

# Correct capitalization
# conversion added in 1.9.0-svn
("Breaking the siege", "Breaking the Siege"),
("Council ruling", "Council Ruling"),
("The Chief must die", "The Chief Must Die"),
("News from the front", "News from the Front"),
("Battle of the book", "Battle of the Book"),
("Council of hard choices", "Council of Hard Choices"),
("Ka'lian under attack", "Ka’lian under Attack"), # Also convert apostrophe to its curly form
("Hostile mountains", "Hostile Mountains"),
),

"wesnoth-manual" : (
),

"wesnoth-multiplayer" : (
# Spelling fix required at r44124.
("helps alot", "helps a lot"),
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
# Correct some English usage at revision 44124
("fortifications and siege them", "fortifications and besiege them"),
("Form, up men!", "Form up, men!"),
("the sunlit word", "the sunlit world"),
("bows and a cudgels", "bows and cudgels"),
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
("Well, if you don't mind me saying - that certainly isn't the state of ", "Well, if you don’t mind me saying — that certainly isn’t the state of "),
("Interesting. I wonder who - or what - could have created such a powerful ", "Interesting. I wonder who — or what — could have created such a powerful "),
("Thus, compelled by some strange and irresistible force, Tallin - eyes glazed ", "Thus, compelled by some strange and irresistible force, Tallin — eyes glazed "),
("and mind unthinking - did the will of his new master.", "and mind unthinking — did the will of his new master."),
("dwarves - it's because of you that we ha' made a start rebuilding Knalga in ", "dwarves — it’s because of you that we ha’ made a start rebuilding Knalga in "),
("I am here simply finishing the job Khazg Black-Tusk started years ago - the ", "I am here simply finishing the job Khazg Black-Tusk started years ago — the "),
("And I am grateful - but what should we do from here? The number of humans ", "And I am grateful — but what should we do from here? The number of humans "),
("Orcs are fickle; if we negotiated a treaty with them - assuming they would ", "Orcs are fickle; if we negotiated a treaty with them — assuming they would "),
("accept - it would just be broken as soon as the next warlord arose among ", "accept — it would just be broken as soon as the next warlord arose among "),
("I have heard all about him from my good friend Stalrag. He is - or was ", "I have heard all about him from my good friend Stalrag. He is — or was "),
("anyway, haven't heard from him in years - the chief o' the villages that lay ", "anyway, haven’t heard from him in years — the chief o’ the villages that lay "),
("As Tallin sat staring blankly at the body of his former friend - loathing ", "As Tallin sat staring blankly at the body of his former friend — loathing "),
("and hating what he had become - he received an urgent summons from his ", "and hating what he had become — he received an urgent summons from his "),
("Pew! Nick of time too - I almost bought it there!", "Pew! Nick of time too — I almost bought it there!"),
("elves - for the sorceress was, in fact, a princess of the highest rank.", "elves — for the sorceress was, in fact, a princess of the highest rank."),
("our help in rescuing their Princess? You heard their message - those elves ", "our help in rescuing their Princess? You heard their message — those elves "),
("Well, suppose we do join up with the elves - assuming they will let us - and ", "Well, suppose we do join up with the elves — assuming they will let us — and "),
("Just our luck to be here when they're mustering a field force - probably to ", "Just our luck to be here when they’re mustering a field force — probably to "),
("Later, princess, first let's get outta -- uh, let us take our leave of this ", "Later, princess, first let’s get outta — uh, let us take our leave of this "),
("him, and so he fled. As he did so, visions of his dying friends - friends ", "him, and so he fled. As he did so, visions of his dying friends — friends "),
("that he had slain - flashed before his eyes, while their voices chided him ", "that he had slain — flashed before his eyes, while their voices chided him "),
("under his control, I have done unthinkable deeds - I have studied the most ", "under his control, I have done unthinkable deeds — I have studied the most "),
("never would have risked their lives - nay, if I were not there they wouldn't ", "never would have risked their lives — nay, if I were not there they wouldn’t "),
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
("Tallin raised his eyes to look into Abhai's and Abhai met his gaze - ", "Tallin raised his eyes to look into Abhai’s and Abhai met his gaze — "),
("your doubts at rest.' Tallin held the gaze for one more long moment - and ", "your doubts at rest.” Tallin held the gaze for one more long moment — and "),
("Yes, Tallin - it is I.", "Yes, Tallin — it is I."),
("How did you get here - I thought you you were dead... and for that matter, ", "How did you get here — I thought you you were dead... and for that matter, "),
("of unimaginable splendor, glory and transcendence - the very realm of the ", "of unimaginable splendor, glory and transcendence — the very realm of the "),
("They are all here, princess! Hand picked by your father - the finest and ", "They are all here, princess! Hand picked by your father — the finest and "),
("They are all here, princess! Hand picked by your father - the finest and ", "They are all here, princess! Hand picked by your father — the finest and "),
("and leave them a small guard. Then they pressed onward to rescue Hidel - and ", "and leave them a small guard. Then they pressed onward to rescue Hidel — and "),
("Cheer up -- you won't have to live with your failure for long...*snicker*", "Cheer up — you won’t have to live with your failure for long...*snicker*"),
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

# Fix screw up
# conversion added in 1.9.0-svn
("Cheer up — you won't have to live with your failure for long...*snicker*", "Cheer up — you won’t have to live with your failure for long...*snicker*"),
("your doubts at rest.' Tallin held the gaze for one more long moment — and ", "your doubts at rest.” Tallin held the gaze for one more long moment — and "),
("Tallin raised his eyes to look into Abhai's and Abhai met his gaze — ", "Tallin raised his eyes to look into Abhai’s and Abhai met his gaze — "),
("never would have risked their lives — nay, if I were not there they wouldn't ", "never would have risked their lives — nay, if I were not there they wouldn’t "),
("Later, princess, first let's get outta — uh, let us take our leave of this ", "Later, princess, first let’s get outta — uh, let us take our leave of this "),
("Just our luck to be here when they're mustering a field force — probably to ", "Just our luck to be here when they’re mustering a field force — probably to "),
("anyway, haven't heard from him in years — the chief o' the villages that lay ", "anyway, haven’t heard from him in years — the chief o’ the villages that lay "),
("dwarves — it's because of you that we ha' made a start rebuilding Knalga in ", "dwarves — it’s because of you that we ha’ made a start rebuilding Knalga in "),
("Well, if you don't mind me saying — that certainly isn't the state of ", "Well, if you don’t mind me saying — that certainly isn’t the state of "),
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
("is time - but I'll", "is time — but I’ll"),
("SE - The River Road.", "SE — The River Road."),
("SW - The", "SW — The"),
("SW - Southbay.", "SW — Southbay."),
("Hold - I see", "Hold — I see"),
("The River Road -", "The River Road —"),
("I'm a tinkin- -", "I’m a tinkin—"),
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
("me - I'll be dead", "me — I’ll be dead"),
("to say it - but", "to say it — but"),
("is ridiculous! I -", "is ridiculous! I —"),
("all I need - prepare", "all I need — prepare"),
("much -- from both", "much — from both"),
("a Lord -- to join", "a Lord — to join"),
("best of times - so", "best of times — so"),

# Fix screw up
# conversion added in 1.9.0-svn
("is time — but I'll", "is time — but I’ll"),
("I'm a tinkin—", "I’m a tinkin—"),
("me — I'll be dead", "me — I’ll be dead"),
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
("when they have an ally directly behind the unit they are attacking - they'll ", "when they have an ally directly behind the unit they are attacking — they’ll "),
("magic. They are slow and weak - any of your units is more than a match for ", "magic. They are slow and weak — any of your units is more than a match for "),
("Finally Ethiliel told the men to halt and pitch camp - they had reached the ", "Finally Ethiliel told the men to halt and pitch camp — they had reached the "),

# Fix screw up
# conversion added in 1.9.0-svn
("when they have an ally directly behind the unit they are attacking — they'll ", "when they have an ally directly behind the unit they are attacking — they’ll "),

# Convert apostrophes and quotation marks
# conversion added in 1.9.0-svn
("I've seen your", "I’ve seen your"),
("We'll surely all", "We’ll surely all"),
("Ithelden's", "Ithelden’s"),
("We won't be able", "We won’t be able"),
("Mal M'Brin", "Mal M’Brin"),
("Gerrick's", "Gerrick’s"),
("men's", "men’s"),
("I prepare the city's defenses", "I prepare the city’s defenses"),
("works if it's the last", "works if it’s the last"),
("I've broken through", "I’ve broken through"),
("I'm lighting it", "I’m lighting it"),
("lich's", "lich’s"),
("we won't kill you", "we won’t kill you"),
("shouldn't", "shouldn’t"),
("I wouldn't have", "I wouldn’t have"),
("I've seen many", "I’ve seen many"),
("you've earned", "you’ve earned"),
("We're almost there", "We’re almost there"),
("I don't think they'll", "I don’t think they’ll"),
("Mebrin's", "Mebrin’s"),
("I'll kill you", "I’ll kill you"),
("they don't look very friendly", "they don’t look very friendly"),
("She won't listen", "She won’t listen"),
("I didn't have a choice", "I didn’t have a choice"),
("if we hadn't fallen", "if we hadn’t fallen"),
("I'm afraid it is not", "I’m afraid it is not"),
("it'd", "it’d"),
("undead aren't so tough", "undead aren’t so tough"),
("You're alive", "You’re alive"),
("You're too late", "You’re too late"),
("Mal A'kai", "Mal A’kai"),
("I hope that's the last", "I hope that’s the last"),
("Hylas's", "Hylas’s"),
("We're too late", "We’re too late"),
("They'll feel the deadly", "They’ll feel the deadly"),
("It's me", "It’s me"),
("they're in trouble", "they’re in trouble"),
("Mathin's", "Mathin’s"),
("While you're in your camp", "While you’re in your camp"),
("citadel's", "citadel’s"),
("'my kind'?", "“my kind”?"),
("I've seen your", "I’ve seen your"),
("Deoran's", "Deoran’s"),
("the capital of Wesnoth's", "the capital of Wesnoth’s"),
("Haldiel's", "Haldiel’s"),
),

"wesnoth-sof" : (
# Typo fixes required at r44124.
("going to back to the mines", "going back to the mines"),
# Convert makeshift dashes:
# conversion added in 1.9.0-svn
("So now I tell from whence it came -", "So now I tell from whence it came —"),
("The Fire-sceptre great -", "The Fire-sceptre great —"),
("Uh, no, wait! Lets talk business - how much will you pay us to do this for ", "Uh, no, wait! Lets talk business — how much will you pay us to do this for "),
("Fine then - ten thousand... now, what exactly do you want us to make the ", "Fine then — ten thousand... now, what exactly do you want us to make the "),
("Hey! You can't do that - this road is an elvish road. We're not letting you ", "Hey! You can’t do that — this road is an elvish road. We’re not letting you "),
("Ah, I see - you are one of the rebels. Our treaty with the elves does not ", "Ah, I see — you are one of the rebels. Our treaty with the elves does not "),
("We will see about that - if you ever get underground, which I doubt. Ha!", "We will see about that — if you ever get underground, which I doubt. Ha!"),
("Here - I'll go warn the council. You stay here and fight.", "Here — I’ll go warn the council. You stay here and fight."),
("dwarf-made stonecraft. We refuse - now let's close these gates!", "dwarf-made stonecraft. We refuse — now let’s close these gates!"),
("Just watch. The gates wi' close very soon. Then the elves outside - and, ", "Just watch. The gates wi’ close very soon. Then the elves outside — and, "),
("unfortunately, our dwarves who are still out there - wi' become irrelevant.", "unfortunately, our dwarves who are still out there — wi’ become irrelevant."),
("for the runesmith named Thursagan - the sage of fire.", "for the runesmith named Thursagan — the sage of fire."),
("members of their party - one expected, and one not.", "members of their party — one expected, and one not."),
("In the treasury. And leave it there until you're ready to work with it - I ", "In the treasury. And leave it there until you’re ready to work with it — I "),
("the finest coal, and many of the finest jewels in the land - we'll need ", "the finest coal, and many of the finest jewels in the land — we’ll need "),
("to fight them; also be prepared to spend quite some time here - mining can ", "to fight them; also be prepared to spend quite some time here — mining can "),
("Yes, although we will have to hire the miners - they don't work for free. ", "Yes, although we will have to hire the miners — they don’t work for free. "),
("were others - many others. Thus I present Theganli, the jeweler. His role is ", "were others — many others. Thus I present Theganli, the jeweler. His role is "),
("were no exception. They were crafters of crafters - they made tools. The ", "were no exception. They were crafters of crafters — they made tools. The "),
("How about this - I help you defeat them, and you let me keep the ruby when ", "How about this — I help you defeat them, and you let me keep the ruby when "),
("Well, back to the battle - we are all in the caves, but there are still ", "Well, back to the battle — we are all in the caves, but there are still "),
("Well, back to the battle - we need everyone to get into the Shorbear caves. ", "Well, back to the battle — we need everyone to get into the Shorbear caves. "),
("If ye'll permit me to say so, sir, ye're wrong. We could - ", "If ye’ll permit me to say so, sir, ye’re wrong. We could — "),
("If you'll permit me to say so, sir, you're wrong. We could - ", "If you’ll permit me to say so, sir, you’re wrong. We could — "),
("went south - back to the Wesnoth border.", "went south — back to the Wesnoth border."),
("And thus Rugnur died - a glorious death, in the eyes of the dwarven sages. ", "And thus Rugnur died — a glorious death, in the eyes of the dwarven sages. "),
("And Krawg - well, Krawg followed him. I know not how. But Krawg made his way ", "And Krawg — well, Krawg followed him. I know not how. But Krawg made his way "),
("But before it was found, legends grew up around it, and around its makers - ", "But before it was found, legends grew up around it, and around its makers — "),
# Straight apostrophes and quotes to curly ones
# conversion added in 1.9.0-svn
# (NOT YET)
#("The land of Wesnoth's banner bold", "The land of Wesnoth’s banner bold"),
#("Made by a runesmith's hand.", "Made by a runesmith’s hand."),

# Fix screw up:
# conversion added in 1.9.0-svn
("Yes, although we will have to hire the miners v they don't work for free. ", "Yes, although we will have to hire the miners — they don’t work for free. "),
("Hey! You can't do that — this road is an elvish road. We're not letting you ", "Hey! You can’t do that — this road is an elvish road. We’re not letting you "),
("If you'll permit me to say so, sir, you're wrong. We could — ", "If you’ll permit me to say so, sir, you’re wrong. We could — "),
("If ye'll permit me to say so, sir, ye're wrong. We could — ", "If ye’ll permit me to say so, sir, ye’re wrong. We could — "),
("the finest coal, and many of the finest jewels in the land — we'll need ", "the finest coal, and many of the finest jewels in the land — we’ll need "),
("In the treasury. And leave it there until you're ready to work with it — I ", "In the treasury. And leave it there until you’re ready to work with it — I "),
("unfortunately, our dwarves who are still out there — wi' become irrelevant.", "unfortunately, our dwarves who are still out there — wi’ become irrelevant."),
("Just watch. The gates wi' close very soon. Then the elves outside — and, ", "Just watch. The gates wi’ close very soon. Then the elves outside — and, "),
("dwarf-made stonecraft. We refuse — now let's close these gates!", "dwarf-made stonecraft. We refuse — now let’s close these gates!"),
("Here — I'll go warn the council. You stay here and fight.", "Here — I’ll go warn the council. You stay here and fight."),
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
("on their lands. Then, Earl Lanbec'h - the most powerful human warlord of the ", "on their lands. Then, Earl Lanbec’h — the most powerful human warlord of the "),
("North - determined to abolish the orcish menace raised an army and conferred ", "North — determined to abolish the orcish menace raised an army and conferred "),
("Who is this unlicked whelp? Grunts - kill him and bring me his head!", "Who is this unlicked whelp? Grunts — kill him and bring me his head!"),
("Wise decision, Kapou'e. By the size of that army, this is no mere raid - it ", "Wise decision, Kapou’e. By the size of that army, this is no mere raid — it "),
("And this is that stupid human who dares march against Kapou'e - Son of the ", "And this is that stupid human who dares march against Kapou’e — Son of the "),
("Hey, Chief, I was thinking - dwarves are pretty slow; why don't we just ", "Hey, Chief, I was thinking — dwarves are pretty slow; why don’t we just "),
("As soon as the sandstorm died down, Kapou'e and his men - eager to escape ", "As soon as the sandstorm died down, Kapou’e and his men — eager to escape "),
("the searing heat - left the oasis and continued on their trek.", "the searing heat — left the oasis and continued on their trek."),
("and all the natural sounds of the forest died away - leaving everything ", "and all the natural sounds of the forest died away — leaving everything "),
("Leave your people here Kapou'e - they are safe for the moment - and go ", "Leave your people here Kapou’e — they are safe for the moment — and go "),
("friends - get them!", "friends — get them!"),
("After a fair amount of squabbling - for some of the older warlords were ", "After a fair amount of squabbling — for some of the older warlords were "),
("reluctant to let this young upstart lead them - and a few consequent ", "reluctant to let this young upstart lead them — and a few consequent "),
("Kapou'e placed one force under the leadership of Shan Taum the Smug, who - ", "Kapou’e placed one force under the leadership of Shan Taum the Smug, who — "),
("despite his obnoxious nature - was a fierce and capable leader. He was sent ", "despite his obnoxious nature — was a fierce and capable leader. He was sent "),
("He sent the shamans, Pirk, Gork and Vraurk back to Borstep - a city just ", "He sent the shamans, Pirk, Gork and Vraurk back to Borstep — a city just "),
("north of the Mourned Hills - to organize any remaining orcish forces as well ", "north of the Mourned Hills — to organize any remaining orcish forces as well "),
("Kapou'e himself - desiring to settle this business once and for all - led ", "Kapou’e himself — desiring to settle this business once and for all — led "),
("The orcs are making headway. The city must not fall - call the reserves!", "The orcs are making headway. The city must not fall — call the reserves!"),
("fall. A few days after the first snowfall a goblin rider - half dead from ", "fall. A few days after the first snowfall a goblin rider — half dead from "),
("exhaustion - raced into Dorset with the news that Borstep was under siege by ", "exhaustion — raced into Dorset with the news that Borstep was under siege by "),
("give him a good chance to clobber the orcs together again - the old ", "give him a good chance to clobber the orcs together again — the old "),
("Kapou'e's exploits had been widely circulated - from his rescue of the ", "Kapou’e’s exploits had been widely circulated — from his rescue of the "),
("horde was dispersed he appointed three more shamans - with the consent of ", "horde was dispersed he appointed three more shamans — with the consent of "),
("all - to make the Great Council complete again.", "all — to make the Great Council complete again."),

# Fix screw up
# conversion added in 1.9.0-svn
("Kapou'e's exploits had been widely circulated — from his rescue of the ", "Kapou’e’s exploits had been widely circulated — from his rescue of the "),
("Kapou'e himself — desiring to settle this business once and for all — led ", "Kapou’e himself — desiring to settle this business once and for all — led "),
("Kapou'e placed one force under the leadership of Shan Taum the Smug, who — ", "Kapou’e placed one force under the leadership of Shan Taum the Smug, who — "),
("Leave your people here Kapou'e — they are safe for the moment — and go ", "Leave your people here Kapou’e — they are safe for the moment — and go "),
("As soon as the sandstorm died down, Kapou'e and his men — eager to escape ", "As soon as the sandstorm died down, Kapou’e and his men — eager to escape "),
("Hey, Chief, I was thinking — dwarves are pretty slow; why don't we just ", "Hey, Chief, I was thinking — dwarves are pretty slow; why don’t we just "),
("And this is that stupid human who dares march against Kapou'e — Son of the ", "And this is that stupid human who dares march against Kapou’e — Son of the "),
("Wise decision, Kapou'e. By the size of that army, this is no mere raid — it ", "Wise decision, Kapou’e. By the size of that army, this is no mere raid — it "),
("on their lands. Then, Earl Lanbec'h — the most powerful human warlord of the ", "on their lands. Then, Earl Lanbec’h — the most powerful human warlord of the "),
),

"wesnoth-tb" : (
# Convert makeshift dashes:
# conversion added in 1.9.0-svn
("A Tale of Two Brothers - Epilogue", "A Tale of Two Brothers — Epilogue"),

# Convert straight apostrophes and quotation marks
## conversion added in 1.9.0-svn
("The village's mage Bjarn", "The village’s mage Bjarn"),
("'Fear and obey Mordak the Mage!'", "“Fear and obey Mordak the Mage!”"),
("Mordak's", "Mordak’s"),
("more than two days'", "more than two days’"),
("brother's", "brother’s"),
("We're chasing after", "We’re chasing after"),
("master's", "master’s"),
("let's catch those", "let’s catch those"),
("We think there's", "We think there’s"),
("They've captured my brother", "They’ve captured my brother"),
("Arne's", "Arne’s"),
("let's kill some orcs", "let’s kill some orcs"),
("you're supposed", "you’re supposed"),
("isn't it", "isn’t it"),
("aren't our relief", "aren’t our relief"),
("they're holding Bjarn", "they’re holding Bjarn"),
("day's", "day’s"),
("I don't think we can rescue", "I don’t think we can rescue"),
),

"wesnoth-tutorial" : (

),

"wesnoth-units" : (
# Spelling fixes required at r44124
("diminuitive", "diminutive"),
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
("There might be, but I don't--", "There might be, but I don’t—"),
("You idiot--", "You idiot—"),
("unfortunately we found your men dead--", "unfortunately we found your men dead—"),
("Well, actually they were fleeing from--", "Well, actually they were fleeing from—"),
("Look, if you'll just let me explain--", "Look, if you’ll just let me explain—"),
("This is--", "This is—"),
("This entire journey has been based on a lie--", "This entire journey has been based on a lie—"),
("the last few generations--if anything the land had grown even more", "the last few generations—if anything the land had grown even more"),

# Fix screw up
# conversion added in 1.9.0-svn
("Look, if you'll just let me explain—", "Look, if you’ll just let me explain—"),
("There might be, but I don't—", "There might be, but I don’t—"),

# Convert straight apostrophes and quotation marks
# conversion added in 1.9.0-svn
("What's happened? Oh Eloh, the craters are everywhere, everything is gone, ruined. I can hardly recognize our village. I didn't think it could be this bad.", "What’s happened? Oh Eloh, the craters are everywhere, everything is gone, ruined. I can hardly recognize our village. I didn’t think it could be this bad."),
("C'mon", "Come on"),
("Tanuil's", "Tanuil’s"),
("That's", "That’s"),
("Uria's", "Uria’s"),
("If we don't stop them", "If we don’t stop them"),
("Then let's join the battle!", "Then let’s join the battle!"),
("let's get rid of them", "let’s get rid of them"),
("If we don't", "If we don’t"),
("things won't be", "things won’t be"),
("we can't dwell on the dead", "we can’t dwell on the dead"),
("Let's keep exploring the wreckage.", "Let’s keep exploring the wreckage."),
("They're destroyed at last.", "They’re destroyed at last."),
("But there's no time", "But there’s no time"),
("so we'll have", "so we’ll have"),
("I'm fine. I'm afraid only", "I’m fine. I’m afraid only"),
("Maybe they're hiding in the stables. Let's go check.", "Maybe they’re hiding in the stables. Let’s go check."),
("We'll need your help", "We’ll need your help"),
("They've agreed to", "They’ve agreed to"),
("couldn't", "couldn’t"),
("Eloh's", "Eloh’s"),
("I've fought", "I’ve fought"),
("We'll just have to find out.", "We’ll just have to find out."),
("I suppose we wouldn't", "I suppose we wouldn’t"),
("there'll be one heck", "there’ll be one heck"),
("You've been working on", "You’ve been working on"),
("Hey Kaleh, how's", "Hey Kaleh, how’s"),
("I'm not quite", "I’m not quite"),
("Yechnagoth's", "Yechnagoth’s"),
("Go'hag", "Go’hag"),
("she's", "she’s"),
("Looks like that's the", "Looks like that’s the"),
("it's a long story", "it’s a long story"),
("you're a mage", "you’re a mage"),
("We'd", "We’d"),
("Let's cleanse", "Let’s cleanse"),
("if it's a fight they want, it's a fight they'll", "if it’s a fight they want, it’s a fight they’ll"),
("That's the last", "That’s the last"),
("there's still dried blood on the stones. It's", "there’s still dried blood on the stones. It’s"),
("they're going to be sorry", "they’re going to be sorry"),
("It's a good", "It’s a good"),
("don't kill me", "don’t kill me"),
("We're just", "We’re just"),
("It's Holy Water.", "It’s Holy Water."),
("we won't be able", "we won’t be able"),
("we've made it", "we’ve made it"),
("I've been searching", "I’ve been searching"),
("I'm searching for the", "I’m searching for the"),
("No, I haven't.", "No, I haven’t."),
("I'm not sure. I've read various references to it, but nothing specific. I've been searching for it for a long time. All I know is that it was a very powerful magical wand and that it was some sort of symbol of royalty in the old empire, but I have no idea where it might be. So I scour the land, learning all I can about the olden days. I'm sure it must be somewhere.", "I’m not sure. I’ve read various references to it, but nothing specific. I’ve been searching for it for a long time. All I know is that it was a very powerful magical wand and that it was some sort of symbol of royalty in the old empire, but I have no idea where it might be. So I scour the land, learning all I can about the olden days. I’m sure it must be somewhere."),
("We've run out of provisions and our people are exhausted. We've taken", "We’ve run out of provisions and our people are exhausted. We’ve taken"),
("I don't...", "I don’t..."),
("or what's left", "or what’s left"),
("I'm impressed.", "I’m impressed."),
("You've been working", "You’ve been working"),
("couldn't be worse", "couldn’t be worse"),
("they haven't had the", "they haven’t had the"),
("I don't think we've explored", "I don’t think we’ve explored"),
("We've explored the village and I think we've", "We’ve explored the village and I think we’ve"),
("glad you're here", "glad you’re here"),
("'You must be strong, young elf", "“You must be strong, young elf"),
("you and protect you.'", "you and protect you.”"),
("We can't leave them", "We can’t leave them"),
("I don't want to tarry", "I don’t want to tarry"),
("What's that to", "What’s that to"),
("We've run out of time.", "We’ve run out of time."),
("won't be forgotten", "won’t be forgotten"),
("Zhul's", "Zhul’s"),
("friends'", "friends’"),
("you don't recruit", "you don’t recruit"),
("I don't see a thing", "I don’t see a thing"),
("poor person's body", "poor person’s body"),
("There doesn't seem", "There doesn’t seem"),
("wait...what's this", "wait...what’s this"),
("I've heard tales", "I’ve heard tales"),
("Traveler's Ring", "Traveler’s Ring"),
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
timecheck = 1278989211  # Tue 13 Jul 2010 02:46:51 AM UTC

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
	        # In case of screwed-up pairs that are hard to find, uncomment the following:
	        #for fix in fixes:
                #    if len(fix) != 2:
                #        print fix
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
