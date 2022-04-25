#!/usr/bin/env python3
# encoding: utf-8

# pofix - perform string fixups on incoming .po files.
#
# The purpose of this script is to save translators from having to
# apply various string fixes needed before stable release by hand.  It is
# intended to be run on each incoming .po file as the Lord of
# Translations receives it.  However, translators may run it on their
# own .po files to be sure, as a second application will harmlessly do
# nothing.
#
# This script will not alter the cfg files but only the po files. So fixes
# have to be manually applied to the source files of the strings!
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
#
# NOTE: IMPORTANT!
# When altering this file ALWAYS use the following steps:
# * from the checkout root call: ./utils/pofix.py po/wesnoth*/wesnoth*.pot
# * check if any strings were changed and if only the intended strings were changed
#   by using e.g. "normal" diff tools or git diff on the changed .pot files
# * if everything was fine, proceed, if something went wrong revert the changed pot
#   files, adjust pofix.py and rerun the above step
# * run: ./utils/pofix.py po/wesnoth*/*.po
# * commit all changed files together (pofix.py as well as the changed pot and po
#   files)
#
#
# (old) example usage:
# utils/pofix.py po/wesnoth*/*.po*
# find data/campaigns/ -name '*.cfg' -print0 | xargs -0 utils/pofix.py
#
# To make use of >1 CPU core, you have to rely on xargs. In this sample 10 files
# are handed over to 4 instances of pofix.py:
# ls po/wesnoth*/*.po* | xargs -P 4 -n 10 ./utils/pofix.py
#
#
# Please do make sure to add a comment before any new blocks of conversions
# that states when it was added (current version number is enough) so that
# the file can be cleaned up more easily every now and then.
# Example:
# # conversion added in 1.9.5+dev
# ("foo addwd bar", "foo added bar"),
# # conversion added in 1.9.8+dev
# ("fooba foo", "foobar foo"),
#
# NOTE:
# Those "textdomain" entries are *only* a hint and don't influence the files on
# which pofix will be applied. All replacements will always be applied on *ALL*
# files!

game_stringfixes = {

"wesnoth" : (
# conversion added in 1.11.10+dev
("Save and Abort game", "Save and abort game"),
("Filters on addon descripton,", "Filters on addon description,"),
("Messanger Waypoint 1", "Messenger Waypoint 1"),
("Identifer", "Identifier"),

# conversion added in 1.13.5+dev
("You don’t have a leader to", "You do not have a leader to"),
("You don’t have enough gold to recruit that unit", "You do not have enough gold to recruit that unit"),

# conversion added in 1.13.6+dev
("The server accepts versions '$version1' while you are using version '$version2'", "The server accepts versions '$required_version', but you are using version '$your_version'")
),

"wesnoth-ai": (
# conversion added in 1.14.3+dev
("http://wiki.wesnoth.org/Micro_AIs", "https://wiki.wesnoth.org/Micro_AIs"),
),

"wesnoth-aoi": (
# conversion added in 1.14.4+dev
("the bodies of both sides, ", "the bodies of both sides. "),
("there was no hale orc in sight,", "There was no hale orc in sight,"),
),

"wesnoth-editor" : (
# conversion added in 1.11.10+dev
("Choose file", "Choose File"),
),

"wesnoth-help" : (
# fix added in 1.13.10+dev
("displayed in the upper right, (under the default theme)", "displayed in the upper right (under the default theme)"),
("However the few implements", "However, the few implements"),
("humans can learn to wield it and able to learn", "humans can learn to wield it and are able to learn"),
("toggle bewteen human and AI.", "toggle between human and AI."),
#fix added in 1.13.11+dev
("This unit can lead our own units", "This unit can lead your own units"),
("moreso", "more so"),
# Typographical fixes added in 1.14.3+dev
("Despite orcs' reliance on raw strength", "Despite orcs’ reliance on raw strength"),
# FIXME: this won't work for some mysterious reason. The strings will have to be fuzzied.
#('who grow to the strength of a \\"true orc\\"', "who grow to the strength of a “true orc”"),
("making them a useful asset to aid in an army's charge.", "making them a useful asset to aid in an army’s charge."),
("The Dunefolk's inclination", "The Dunefolk’s inclination"),
("The Dunefolk's inquisitive", "The Dunefolk’s inquisitive"),
("the heritage of the Dunefolk's", "the heritage of the Dunefolk’s"),
),

"wesnoth-l" : (
# conversion added in 1.14.4+dev
("Retreat!!", "Retreat!"),
),

"wesnoth-lib" : (
# conversion added in 1.11.15+dev
("SP/MP Campaigns", "SP/MP campaigns"),

# conversion added in 1.13.5+dev
("If you don’t want to receive messages", "If you do not want to receive messages"),
),

"wesnoth-httt" : (
# fix added in 1.10.0+dev
("Second, who you most", "Second, whom you most"),
# fix added in 1.11.16+dev
("Who then is your leader? Who do we serve?", "Who then is your leader? Whom do we serve?"),
# fix added in 1.13.10+dev
("might we ask your assistance.", "might we ask your assistance?"),
("Onward men!", "Onward, men!"),
("No we have not.", "No, we have not."),
("If it is necessary, princess, I will ask", "If it is necessary, Princess, I will ask"),
("What secret Delfador?", "What secret, Delfador?"),
("I High Provost", "I, High Provost"),
("both a great combatant and leader", "both a great combatant and a leader"),
("You must lead your men to the city and help defend it. Or recapture it if it falls before you arrive.", "You must lead your men to the city. Help defend it, or recapture it if it falls before you arrive!"),
# fix added in 1.13.11+dev
("with the greatest generals, and battle tactics", "with the greatest generals and battle tactics"),
# fixes added in 1.14.3+dev
("Whatever.... I still think we should make an effort", "Whatever... I still think we should make an effort"),
("That is so very encouraging....","That is so very encouraging..."),
# fixes added in 1.15.1+dev
("This unit's grasp of melee tactics", "This unit’s grasp of melee tactics"),
("mount Elnar", "Mount Elnar"),
),

"wesnoth-low" : (
# added for 1.13.4+dev
("If you loose you still have a chance to defeat Kalenz in the next scenario.", "If you lose you still have a chance to defeat Kalenz in the next scenario."),
),

"wesnoth-nr" : (
# fixes added in 1.12.0+dev
("They are stronger then we thought.", "They are stronger than we thought."),
("Hmmm, they are stronger then we thought", "Hmmm, they are stronger than we thought"),
("torment other then self destruction.", "torment other than self destruction."),
("Rod of Justice for more then a few", "Rod of Justice for more than a few"),
("you have aided the Northern Elves more then you can imagine.", "you have aided the Northern Elves more than you can imagine."),
("been more then a few months ago", "been more than a few months ago"),
("they cannot be more then two days’ march from here.", "they cannot be more than two days’ march from here."),
("It couldna’ been more then a day now.", "It couldna’ been more than a day now."),
("It couldna’ ha’ been more then a day now.", "It couldna’ ha’ been more than a day now."),
("They are no more then a few days", "They are no more than a few days"),
("fearsome a foe then a dwarf.", "fearsome a foe than a dwarf."),
("hold the orcs off far longer and with less loss then ye could ha’ done", "hold the orcs off far longer and with less loss than ye could ha’ done"),
("Bah! I have better things to do then stamp out your insignificant life.", "Bah! I have better things to do than stamp out your insignificant life."),
# fixes added in 1.14.3+dev
("most important things about being a leader is....", "most important things about being a leader is..."),
),

"wesnoth-sof" : (
# fixes added in 1.16.2+dev
("Fire. The sceptre has a long, glorious,", "Fire. The Sceptre has a long, glorious,"),
("by Haldric the Great. Nor will I tell the story of Garard I, ", "by Haldric the Great, nor will I tell the story of Garard I "),
("challenge he set, or that of Konrad, King ", "challenge he set, nor that of Konrad, King "),
("You must come into the caves, and go to the city ", "You must come into the caves and go to the city "),
("I will need five thousand silver in advance, to ", "I will need five thousand silver in advance to "),
("make sure you are going to get it done on time.", "make sure you get it done on time."),
("Kalenz, or make deals with those who betrayed us.", "Kalenz or make deals with those who betrayed us."),
("Here’s the stone, for you dwarves!", "Here’s the stone for you dwarves!"),
("That’s a bluff, what sort of reinforcements can this little band", "That’s a bluff. What sort of reinforcements can this little band"),
("We cannot match you above ground, but in the caves we may", "We cannot match you above ground, but in the caves, we may"),
("Die, humans, in the name of Landar!", "Die, humans! In the name of Landar!"),
("Die, dwarvish scum, in the name of Landar!", "Die, dwarvish scum! In the name of Landar!"),
("This gains you nothing, we will not rest!", "This gains you nothing! We will not rest!"),
("overrun by the damn elves... argh again.", "overrun by the damn elves... argh, again."),
("Och, well, no. Lately trolls have", "Och, well, no. Lately, trolls have"),
("I think it can be sealed up somehow... Yes, look, activating", "I think it can be sealed up somehow... yes, look, activating"),
("exit these caves, to give Haldric back his jewel.", "exit these caves to give Haldric back his jewel."),
("ahead, and could feel sporadic", "ahead and could feel sporadic"),
("Um... Aren’t we needed for the salvage?", "Um... aren’t we needed for the salvage?"),
("... Let’s set up a base here.", "... let’s set up a base here."),
("This old cart still rolls smoothly on the track... It’s forged by us ", "This old cart still rolls smoothly on the track... it’s forged by us "),
("I can try to see what’s underneath... Whoops!", "I can try to see what’s underneath... whoops!"),
("Och, we ’ave... But our rails are as strong and stout as we are!", "Och, we ’ave... but our rails are as strong and stout as we are!"),
("Arg, I never should’a come to this stinkin’ cathole!", "Argh, I never should’a come to this stinkin’ cathole!"),
("came back and claimed the rails ended, the main tunnel", "came back and claimed the rails ended — the main tunnel"),
("Rugnur lead the dwarves", "Rugnur led the dwarves"),
("caves... It’s been quite harrowing, this trip", "caves... it’s been quite harrowing, this trip"),
("glances back to the dwarves.)</i>", "glances back at the dwarves.)</i>"),
("Aye, trolls and wolves are closing in... Sure, Krawg, we’ll help", "Aye, trolls and wolves are closing in... sure, Krawg, we’ll help"),
("He doesn’t want to talk to the gryphons, apparently.", "He doesn’t want to talk to gryphons, apparently."),
("there are still wild animals here... We have to get back", "there are still wild animals here... we have to get back"),
("Astounding! But what is that gryphon doing there?", "Astounding! But what is that gryphon doing here?"),
("also be prepared to spend quite some time here — mining", "also, be prepared to spend quite some time here — mining"),
("do not know where it was found, but Baglur said", "know not exactly where it can be found, but Baglur said"),
("Here is some of the coal that we need! Bring the miners to take it!", "Here is some of the coal that we need! Send a miner here to take it!"),
("Here is the mine of precious gold! Send the miners this way.", "Here is the mine of precious gold! Send a miner here to take it!"),
("So now go to the forge and make your sceptre.", "Now go to the forge and make your sceptre."),
("yes, very impressive ruby... Even in this raw form", "yes, very impressive ruby... even in this raw form"),
("Maybe... maybe... It will be hard, let me see...", "Maybe... maybe... it will be hard, let me see..."),
("no... unfortunately... It seems it can’t", "no... unfortunately... it seems it can’t"),
("Another group of dwarves that live south of here... Above ground, if", "Another group of dwarves that live south of here... above ground, if"),
("Getting this cut isn’t worth that much; what with", "Getting this cut isn’t worth that much — what with"),
("all the other expenses, if we pay more than a thousand", "all the other expenses. If we pay more than a thousand,"),
("Look at him fly... If only we could move that fast.", "Look at him fly... if only we could move that fast."),
("I thought we left the elves behind at the gates way back when... It seems", "I thought we left the elves behind at the gates way back when... it seems"),
("Ha! Now we can get those tools easily, and go back to our own caves.", "Ha! Now we can easily take their tools back to our own caves."),
("True. I suspect these elves will try to besiege us here.", "True. I suspect these elves will try to besiege us."),
("Shorbear tools. Eventually the sceptre was crafted, but ", "Shorbear tools. Eventually, the sceptre was crafted, but "),
("not right, no matter what enchantment of runes", "not right no matter what enchantment of runes"),
("I see... Well, we will not be fooled by your haggling", "I see... well, we will not be fooled by your haggling"),
("I have made it past those elves, but they will chase me, and elvish", "I have made it past these elves, but they will chase me, and elvish"),
("We will certainly help any warrior of Wesnoth who needs our help!", "We will certainly help any warrior of Wesnoth in need!"),
("I’m a dragoon, with Haldric II’s personal bodyguard.", "I’m a dragoon with Haldric II’s personal bodyguard."),
("I’m a cavalier, with Haldric II’s personal bodyguard.", "I’m a cavalier with Haldric II’s personal bodyguard."),
("Thus Alanin escaped from his elvish pursuers.", "Thus, Alanin escaped from his elvish pursuers."),
("Stop that! You leave my pets alone!", "Stop that! Leave my pets alone!"),
("Nothing!? Even this magical piece of crap lets me down!", "Nothing!? Even this magical piece of troll dung lets me down!"),
("symbols for these arcs... Well, it must describe the volcano", "symbols for these arcs... well, it must describe the volcano"),
("And thus Rugnur died a glorious death, in the eyes of the dwarven sages — ", "And thus, in the eyes of the dwarven sages, Rugnur died a glorious death — "),
("My Lord, the dwarves lead by Rugnur are dead,", "My Lord, the dwarves led by Rugnur are dead,"),
("Why were you returning without the sceptre, then?", "Why were you returning without the Sceptre, then?"),
("Rugnur sent me back, with the message that the sceptre was", "Rugnur sent me back, with the message that the Sceptre was"),
("If the sceptre was not completed in your sight, what makes", "If the Sceptre was not completed in your sight, what makes"),
("Krawg reached over his back with his beak, and brought out", "Krawg reached over his back with his beak and brought out"),
("Those are the plans for the sceptre, or at least an earlier version.", "Those are the plans for the Sceptre, or at least an earlier version."),
("Your advice is grating to my ears, not what I wish to hear... And yet,", "Your advice is grating to my ears, not what I wish to hear... and yet,"),
("The sceptre would not be found until many generations after Haldric II,", "The Sceptre would not be found until many generations after Haldric II,"),
("Increases ranged weapon accuracy 20%", "Increases ranged weapon accuracy by 20%"),
("Increases ranged weapon accuracy 10%", "Increases ranged weapon accuracy by 10%"),
),

"wesnoth-sota" : (
# fixes added in 1.13.13+dev
("Welcome to my laboratory, Ardonna of Tarynth", "Welcome to my laboratory, Ardonna of Tarrynth"),
# fixes added in 1.14.3+dev
("others.... An altar serves", "others... An altar serves"),
),

# fixes added in 1.13.13+dev
"wesnoth-sota" : (
("Welcome to my laboratory, Ardonna of Tarynth", "Welcome to my laboratory, Ardonna of Tarrynth"),
),

"wesnoth-sotbe" : (
# fixes added in 1.12.0+dev
("Easier said then done, Chief. There are many humans in that city.", "Easier said than done, Chief. There are many humans in that city."),
("then your kind can. Take the orcish prisoners and hurry to Melmog.", "than your kind can. Take the orcish prisoners and hurry to Melmog."),
("Better late then never. Now it’s time to kill!", "Better late than never. Now it’s time to kill!"),
("becomes no less then a boot-licking spy for the humans.", "becomes no less than a boot-licking spy for the humans."),
("consequently, those orcs thirsting for battle got more then", "consequently, those orcs thirsting for battle got more than"),
),

"wesnoth-tb" : (
("try to force him off of his keep", "try to force him off his keep"),
),

"wesnoth-tsg": (
# fixes added in 1.14.4+dev
("Bury me deeply my friends...", "Bury me deeply, my friends..."),
# fix added in 1.14.9+dev
("patrol the villages once again. ","patrol the villages once again."),
),

"wesnoth-trow" : (
# fixes added in 1.14.3+dev
("a noble of the line of Kings should utter the following....", "a noble of the line of Kings should utter the following..."),
),

"wesnoth-tutorial" : (
# conversion added in 1.11.0-dev
("$unit.type", "$unit.language_name"),
),

"wesnoth-units" : (
# conversion added in 1.12.5+dev
("Ghazis are", "Ghazi are"),
("Hakims are", "Hakim are"),
("Shujas are", "Shuja are"),
("Mudafis excel", "Mudafi excel"),
("uses to this advantage", "uses this to their advantage"),
("and are valuable enough that auxilliaries who specialize", "and are valuable enough that auxiliaries who specialize"),
("Their remedies cannot only heal wounds", "Their remedies can not only heal wounds"),
# fix added in 1.13.11+dev
("the singleminded tenacity of an oak.", "the single-minded tenacity of an oak."),
),

"wesnoth-utbs" : (
# fixes added in 1.12.0+dev
("On the tallest peak was build", "On the tallest peak was built"),
("He killed himself rather then surrender to us!", "He killed himself rather than surrender to us!"),
("bigger distraction then they were expecting.", "bigger distraction than they were expecting."),

# added in 1.13.14+dev
("the one thing she not will be anticipating.", "the one thing she will not be anticipating."),

# added in 1.14.13+dev
("answer to the call of duty.", "answer the call of duty."),
),

"wesnoth-multiplayer" : (
# 1.13.4+dev
("Changes the gold worth of the enemy spawns by a certain perentage", "Changes the gold worth of the enemy spawns by a certain percentage"),
# fixes added in 1.14.3+dev
("Rah Ihn Mar began the hunt.....", "Rah Ihn Mar began the hunt..."),
("Perhaps he should not have shouted quite so loudly.... ", "Perhaps he should not have shouted quite so loudly..."),
("but the two men never found one another. ", "but the two men never found one another."),
),

}

website_stringfixes = {

"1.14-announcement" : (
("better ingrate them", "better integrate them"),

# Lua version number
("5.3.4", "5.3"),

# Yes, these are really flimsy. In hindsight it was a bad idea to include
# download sizes in the announcement. Hopefully we won't end up with
# files that have the same size in a release/in the last two releases
# any time soon.

# Source
("wesnoth-1.14.16/wesnoth-1.14.16.tar.bz2", "wesnoth-1.14.17/wesnoth-1.14.17.tar.bz2"),
#("462.5", "462.5"),
# Windows
("wesnoth-1.14.16/wesnoth-1.14.16-win32.exe", "wesnoth-1.14.17/wesnoth-1.14.17-win32.exe"),
#("415.8", "415.8"),
# macOS
("wesnoth-1.14.16/Wesnoth_1.14.16.dmg", "wesnoth-1.14.17/Wesnoth_1.14.17.dmg"),
#("454.6", "454.6"),

("&#169; 2003&#8211;2020", "&#169; 2003&#8211;2021"),
("&#169; 2017&#8211;2020", "&#169; 2017&#8211;2021"),
),

}

# Whether -w was passed in the command line. Selects website_stringfixes
# instead of game_stringfixes.
website_mode = 0

# Speak, if all argument files are newer than this timestamp
# Try to use UTC here
# date --utc "+%s  # %c"
timecheck = 1462268096  # Tue May  3 09:34:56 2016

import os, sys, time, stat, re, argparse, glob, io
if sys.version_info < (3, 0):
    reload(sys)
    sys.setdefaultencoding('utf8')
try:
    from multiprocessing import Pool, cpu_count
    def parallel_map(*args, **kw):
        pool = Pool(cpu_count())
        return pool.map(*args, **kw)
except ImportError:
    print ("Failed to import 'multiprocessing' module. Multiple cpu cores won't be utilized")
    parallel_map = map

def process_file(path):
    before = io.open(path, "r", encoding="utf-8").read()
    decommented = re.sub("#.*", "", before)
    decommented_msgids = re.sub(r'^msgstr .*?' + '\n\n', '', decommented, flags = re.MULTILINE | re.DOTALL)
    lines = before.split('\n')
    if website_mode:
        stringfixes = website_stringfixes
    else:
        stringfixes = game_stringfixes
    for (domain, fixes) in stringfixes.items():
        # In case of screwed-up pairs that are hard to find, uncomment the following:
        #for fix in fixes:
        #    if len(fix) != 2:
        #        print(fix)
        for (old, new) in fixes:
            if old is new:
                #complain loudly
                print ("pofix: old string\n\t\"%s\"\n equals new string\n\t\"%s\"\nexiting." % (old, new))
                sys.exit(1)
            #this check is problematic and the last clause is added to prevent false
            #positives in case that new is a substring of old, though this can also
            #lead to "real" probs not found, the real check would be "does replacing
            #old with new lead to duplicate msgids? (including old ones marked with #~)"
            #which is not easily done in the current design...
            elif new in decommented_msgids and old in decommented_msgids and not new in old:
                print ("pofix: %s already includes the new string\n\t\"%s\"\nbut also the old\n\t\"%s\"\nthis needs handfixing for now since it likely creates duplicate msgids." % (path, new, old))
            else:
                for (i, line) in enumerate(lines):
                    if line and line[0] != '#':
                        lines[i] = lines[i].replace(old, new)
    after = '\n'.join(lines)
    if after != before:
        print ("pofix: %s modified" % path)
        # Save a backup
        os.rename(path, path + "-bak")
        # Write out transformed version
        ofp = io.open(path, "w", encoding="utf-8")
        ofp.write(after)
        ofp.close()
        return 1
    else:
        return 0

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('-w', action='store_true', dest='website_mode',
                        help='selects the website string fixes table instead of the game string fixes table')
    parser.add_argument('paths', nargs='*')
    args = parser.parse_args()
    website_mode = args.website_mode
    if website_mode:
        print("pofix: Using website string fixes table")
    newer = 0
    modified = 0
    pocount = 0
    files = []
    for arg in args.paths:
      for path in glob.glob(arg):
        if not path.endswith(".po") and not path.endswith(".pot") and not path.endswith(".cfg") and not path.endswith(".html"):
            continue
        pocount += 1
        # Notice how many files are newer than the time check
        statinfo = os.stat(path)
        if statinfo.st_mtime > timecheck:
            newer += 1
        files.append(path)
    modified = sum(parallel_map(process_file, files))
    print ("pofix: %d files processed, %d files modified, %d files newer" \
          % (pocount, modified, newer))
    if pocount > 1 and newer == pocount:
        print ("pofix: script may be obsolete")
