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
# Three lines in the structure below, marked with "#*", imply changes of
# meaning that may require a change in translation.

stringfixes = {

# Changes in the wesnoth domain make consistent the use of the American
# spelling "defense" in key strings.
#
"wesnoth" : (
("have good defences in water", "have good defenses in water"),
("have a low defence", "have a low defense"),
("characers", "characters"),
("That will let you chose which hex you put a new unit in.", "That will let you choose which hex you put a new unit in."),
("that have a least one", "that have at least one"),
("Apply Filer", "Apply Filter"),
("DejaVuSans.ttf,wqy-zenhei.ttc", "DejaVuSans.ttf,Andagii.ttf,wqy-zenhei.ttc"),
("DejaVu Sans,WenQuanYi Zen Hei", "DejaVu Sans,Andagii,WenQuanYi Zen Hei"),
("wqy-zenhei.ttc,DejaVuSans.ttf", "wqy-zenhei.ttc,DejaVuSans.ttf,Andagii.ttf"),
("WenQuanYi Zen Hei,DejaVu Sans,", "WenQuanYi Zen Hei,DejaVu Sans,Andagii"),
("Icelandic translation", "Icelandic Translation"),

# "Familiarise" is not used in American English
("or familiarise yourself", "or familiarize yourself"),

# Correct spelling
("has a tool-tip", "has a tooltip"),
),

"wesnoth-aoi" : (
# Typo fix.
#
("Edition", "Editing"),
("the  pass was crossed", "the pass was crossed"),
("furtherest-faring", "furthermost-faring"),
("hit and run", "hit-and-run"),
("well defended", "well-defended"),
("We can't get through lord. These whelps are not individually very dangerous, but there are huge number of them.", "We can't get through, my Lord. These whelps are not individually very dangerous, but there are huge numbers of them."),
("They waited the next few days before the expected relief caught up with them. Next morning", "The expected relief caught up with them a few days later. The following morning"),
("the woods they called home. It has been murdered.","the woods they called home. It had been murdered."),
),

"wesnoth-did" : (
# Typo shifted the meaning slightly, but changing the
# translation is optional
#
("the godly city of Tath", "the goodly city of Tath"),
# See the desription of the TROW changes
("So, I've finally found your lair, Lich.", "So, I've finally found your lair, lich."),
("made me turn into a Lich.", "made me turn into a lich."),
("you and your minions have killed, Lich", "you and your minions have killed, lich"),
),

"wesnoth-dm" : (
("Now. go forth", "Now, go forth"),
("but it the power it gives tends to magnifies any", "but the power it gives tends to magnify any"),
("an endless steam of undead.", "an endless stream of undead."),
("No! it's all over!", "No! It's all over!"),
# double space fixes
("Another  village  burned. Nobody to be seen!", "Another village burned. Nobody to be seen!"),
("Your majesty, these are  no stray undead; This  lunatic, Iliah-Malal, has", "Your majesty, these are no stray undead; This lunatic, Iliah-Malal, has"),
("The elves will not be able  to fight the", "The elves will not be able to fight the"),
("If we are  to defeat the undead", "If we are to defeat the undead"),
("but the King suggested  that everything would be clear", "but the King suggested that everything would be clear"),
("the free tribes and  demand", "the free tribes and demand"),
("and all but the  true people!", "and all but the true people!"),
),

"wesnoth-ei" : (
("Wait - What just happened?", "Wait - what just happened?"),
("attack ME?", "attack <i>me?</i>"),
("passage way", "passageway"),
),

"wesnoth-httt" : (
# Missing comma clarifies that Lisar is ordering her duelist to attack,
# not ordering someone to attack him!  Translators will probably have picked
# this up already.
#
("attack my loyal duelist", "attack, my loyal duelist"),
# Lich capitalization problem
("Lich Lord", "Lich-Lord"),
("remnants of the Lich's once-mortal skin", "remnants of the lich's once-mortal skin"),
# Bad English usage.  Translators probably got this right from context
("lodged loose", "dislodged"),
("sea sick", "seasick"),
# Concisted capitalization of species names
("Loyal Merfolk","Loyal merfolk"),
("move along, Elf", "move along, elf"),
("fight our way through the Orcs or the Undead.", "fight our way through the orcs or the undead."),
("Elven allies", "elven allies"),
("Drake base", "drake base"),
("grime and and gristle", "grime and gristle"),
# See the explanation of the changes for TROW.
("A final blow destroys the Lich,", "A final blow destroys the lich,"),
("back yard", "back-yard"),
("color='#000000'", "color='white'"),
("color='#ff0000'", "color='red'"),
("color='#00ff00'", "color='green'"),
("color='#ffff00'", "color='yellow'"),
),

"wesnoth-low" : (
("offensive war. This is the council's decision\"", "offensive war. This is the council's decision.\""),
('bring in reinforcements"', 'bring in reinforcements."'),
('Sure, minus expenses"', 'Sure, minus expenses."'),
('I told ye I\'d be here"', 'I told ye I\'d be here."'),
('is bringing with him, though"','is bringing with him, though."'),
('I an defeated.','I am defeated.'),
('The orcs are not defeated defeated,','The orcs are not defeated,'),
("but out army", "but our army"),
("arrives!.", "arrives!"),
),

"wesnoth-manual" : (
# In English, if you "chop a piece of meat", there is a strong
# implication that you regard it as food. The term "flesh" does
# not have this implication.
#
("chop pieces of meat", "chop pieces of flesh"),
),

"wesnoth-nr" : (
# Most NR changes are comma-placement or typo fixes.  The first two
# are exceptions, required by our policy of not allowing anything in
# Wesnoth to resemble real-world religious behavior.
#
("Thank god you are free!", "Thank the Bright Gods you are free!"),	#*
("My God, such a fierce", "Bright Gods, such a fierce"),		#*
("get of her high horse", "get off her high horse"),
("2 000 gold", "2,000 gold"),
("Oh, don't mind him", "Oh, dinna' mind him"),
("Southern Tunnels friends", "Southern Tunnels, friends"),
("Thank you Lord Tallin.", "Thank you, Lord Tallin."),
("Northern Elves extremely few", "Northern Elves are extremely few"),
("14, 318 AD", "14,318 AD"),
("He is just dissolving.", "He is dissolving."),
("To do this Tallin", "To do this, Tallin"),
("I refuse utter", "I refuse to utter"),
("right, the I feel that", "right, I feel that"),
("seek the surface Tallin", "seek the surface, Tallin"),
("demented Sorcerer", "demented sorcerer"),
("who lead his people", "who led his people"),
("No Tallin, you are not dead", "No, Tallin, you are not dead"),
("Greater Gods then so be it.", "Greater Gods, then so be it."),
("Shut up you little snot", "Shut up, you little snot"),
("I wasn't talking to you lich", "I wasn't talking to you, lich"),
("I am honored sir", "I am honored, sir"),
("Rest assured sir", "Rest assured, sir"),
("I propose is the creation", "I propose the creation"),
("Heck yeah", "Heck, yeah"),
("Thank you Tallin.", "Thank you, Tallin."),
("lords of light", "Lords of Light"),
("mater what race they come from - even orcs.", "matter what race they come from - even orcs."),
("I think we have a chance to make new some allies here...", "I think we have a chance to make some new allies here..."),
("Such were the last words of of Rakshas the great!", "Such were the last words of Rakshas the great!"),
('and wounded being cared for,','and wounded having been cared for,'),
('roving around praying on','roving around preying on'),
),

"wesnoth-thot" : (
# These are typo fixes not caught by the spell checker
#
("It sees that guard", "It seems that guard"),
("Go thorough that rubble", "Go through that rubble"),
),

"wesnoth-trow" : (
# Most changes to TROW simply fix inconsistent capitalization
# of the word "lich" and the phrase "lich-lord", applying the
# following rules:
# 1) "lich" and "lich-lord" as the name of type of monster is a common noun
#    and should not be capitalized.
# 2) Lich-Lord as a title of address should be capitalized.
# 3) "Lich-Lords" as the proper name of a particular set of lich-lords
#    (e.g. those of the Wesfolk) should be capitalized.
#
("funny that the lich-Lord", "funny that the lich-lord"),
("evil Lich in the catacombs", "evil lich in the catacombs"),
("The Lich was carrying", "The lich was carrying"),
("defeated the Lich and returned", "defeated the lich and returned",),
("know that Lich you",  "know that lich you"),
("Lich. Maybe you", "lich. Maybe you"),
("Young Prince Haldric.", "young Prince Haldric."),
("Kill the Lich to get his book and", "Kill the lich to get his book and"),
("The Lich is free!", "The lich is free!"),
("before we free that Lich","before we free that lich"),
# Banish an Earth-human name.
("Edmond", "Edren"),
#("great river", "Great River"),
),

"wesnoth-tsg" : (
# In The South Guard, one change fixes comma placement.
#
# One other change (marked "#*") places a question mark where needed.
# This may imply a translation change in languages with question
# inflections in their grammar.
#
# One other change replaces "menfolk" with "humans"; the former
# has an idiomatic sense in American English that you don't want.
#
("Now Deoran, take", "Now, Deoran, take"),
("can you do against the dead!", "can you do against the dead?"),
("undead are hiding in the forest.", "undead are hiding in the forest?"), #*
("menfolk", "humans"),
),

"wesnoth-sof" : (
("Not all of us Elves are", "Not all of us elves are"),
("to make an sceptre out", "to make a sceptre out"),
("us to the Eastern", "us to the eastern"),
("Mines, and I", "mines, and I"),
("years looking for you Dwarves", "years looking for you dwarves"),
("he put on the sceptre", "he put on the Sceptre"),
("give the sceptre", "give the Sceptre"),
("scepter he should send", "Sceptre he should send"),
("from his Elvish pursuers", "from his elvish pursuers"),
("getting the sceptre", "getting the Sceptre"),
("scepter of fire", "sceptre of fire"),
("his scepter intact", "his sceptre intact"),
),

"wesnoth-sotbe" :(
("can from from each", "can from each"),
("go check it out.", "go look."),
("Shan Taum doesn't seem to have to many of his", "Shan Taum doesn't seem to have too many of his"),
),


"wesnoth-tb" : (
("mage Bjarn send for", "mage Bjarn sends for"),
),

"wesnoth-tutorial" : (
# typo fixes
("effecient", "efficient"),
),

"wesnoth-units" : (
(" and and ", " and "),
("gift whiuch contributes", "gift which contributes"),
("agrssiveness to mAtch it", "aggressiveness to match it"),
("are wonerously keen-sighted", "are wonderously keen-sighted"),
("cunning wnough to", "cunning enough to"),
("find humself leading", "find himself leading"),
("and simetimes thought", "and sometimes thought"),
("the sewcrecy and fell", "the secrecy and fell"),
("rumors which sourround", "rumors which surround it"),
("instinctive loyalty fronm ", "instinctive loyalty from "),
("wekll-shaped to flie true", "well-shaped to fly true"),
),

"wesnoth-utbs" : (
# "Yanqui" is an ethnic slur in Spanish
("Yanqui", "Zhangor"),
# Addressing bug #15027
("$dwarf_name", "$intl_dwarf_name"),
("$troll_name", "$intl_troll_name"),
),

"1.8-announcement" : (
("WML events an AI components", "WML events and AI components"),
("1.7.3", "1.7.13"),
("/tags/1.8/", "/tags/1.8.0/"),
),


}

# Speak, if all argument files are newer than this timestamp
# Try to use UTC here
# date --utc "+%s  # %c"
timecheck = 1262364535  # Fri 01 Jan 2010 04:48:55 PM UTC

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
