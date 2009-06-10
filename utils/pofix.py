#!/usr/bin/env python

# pofix - perform string fixups on incoming .po files.
#
# The purpose of this script is to save translators from having to
# apply various string fixes needed before 1.6 by hand.  It is
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
# Three lines in the structure below, marked with "#*", imply changes of
# meaning that may require a change in translation.

stringfixes = {

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
),

"wesnoth-manual" : (
# In English, if you "chop a piece of meat", there is a strong 
# implication that you regard it as food. The term "flesh" does
# not have this implication.
#
("chop pieces of meat", "chop pieces of flesh"),
),

"wesnoth-thot" : (
# These are typo fixes not caught by the spell checker
#
("It sees that guard", "It seems that guard"),
("Go thorough that rubble", "Go through that rubble"),
),

# Changes in the wesnoth domain make consistent the use of the American
# spelling "defense" in key strings.
#
"wesnoth" : (
("have good defences in water", "have good defenses in water"),
("have a low defence", "have a low defense"),
("characers", "characters"),
),

"wesnoth-trow" :(
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

"wesnoth-httt" :(
# See the explanation of the changes for TROW.
("A final blow destroys the Lich,", "A final blow destroys the lich,"),
("back yard", "back-yard"),
),

"wesnoth-tsg" :(
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

"wesnoth-nr" :(
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
),

"wesnoth-low" :(
("offensive war. This is the council's decision\"", "offensive war. This is the council's decision.\""),
('bring in reinforcements"', 'bring in reinforcements."'),
('Sure, minus expenses"', 'Sure, minus expenses."'),
('I told ye I\'d be here"', 'I told ye I\'d be here."'),
('is bringing with him, though"','is bringing with him, though."'),
),

"wesnoth-sotbe" :(
("can from from each", "can from each"),
("go check it out.", "go look."),
),

"wesnoth-tb" :(
("mage Bjarn send for", "mage Bjarn sends for"),
),

"wesnoth-ei" :(
("Wait - What just happened?", "Wait - what just happened?"),
("attack ME?", "attack <i>me?</i>"),
("passage way", "passageway"),
),

"wesnoth-utbs" :(
# "Yanqui" is an ethnic slur in Spanish
("Yanqui", "Zhangor"),
),

}

# Speak, if all argument files are newer than this timestamp
# Try to use UTC here
timecheck = 1242378306	# Fri May 15 05:04:26 EDT 2009

import os, sys, time, stat

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
            for (domain, fixes) in stringfixes.items():
                for (old, new) in fixes:
                    if old is new:
                        #complain loudly
                        print "pofix: old string\n\t\"%s\"\n equals new string\n\t\"%s\"\nexiting." % (old, new)
                        sys.exit(1)
                    elif new in after and old in after:
                        print "pofix: %s has a msgid \n\t\"%s\"\nand a typoed version \n\t\"%s\"\nthis needs handfixing for now." % (path, old, new)
                    else:
                        after = after.replace(old, new)
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
