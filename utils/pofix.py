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

# conversion added in 1.13.5+dev
("You don’t have a leader to", "You do not have a leader to"),
("You don’t have enough gold to recruit that unit", "You do not have enough gold to recruit that unit"),

# conversion added in 1.13.6+dev
("The server accepts versions '$version1' while you are using version '$version2'", "The server accepts versions '$required_version', but you are using version '$your_version'")
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
),

"wesnoth-sotbe" : (
# fixes added in 1.12.0+dev
("Easier said then done, Chief. There are many humans in that city.", "Easier said than done, Chief. There are many humans in that city."),
("then your kind can. Take the orcish prisoners and hurry to Melmog.", "than your kind can. Take the orcish prisoners and hurry to Melmog."),
("Better late then never. Now it’s time to kill!", "Better late than never. Now it’s time to kill!"),
("becomes no less then a boot-licking spy for the humans.", "becomes no less than a boot-licking spy for the humans."),
("consequently, those orcs thirsting for battle got more then", "consequently, those orcs thirsting for battle got more than"),
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

),

"wesnoth-utbs" : (
# fixes added in 1.12.0+dev
("On the tallest peak was build", "On the tallest peak was built"),
("He killed himself rather then surrender to us!", "He killed himself rather than surrender to us!"),
("bigger distraction then they were expecting.", "bigger distraction than they were expecting."),
),

"wesnoth-multiplayer" : (
# 1.13.4+dev
("Changes the gold worth of the enemy spawns by a certain perentage", "Changes the gold worth of the enemy spawns by a certain percentage"),
),

}

website_stringfixes = {

"1.12-announcement" : (
("http://1.12-changelog.wesnoth.org", "//changelog.wesnoth.org/1.12"),
("http://wiki.wesnoth.org", "//wiki.wesnoth.org"),
("http://forums.wesnoth.org", "//forums.wesnoth.org"),
("http://www.wesnoth.org", "//www.wesnoth.org"),
("wesnoth-1.12.5/wesnoth-1.12.5.tar.bz2", "wesnoth-1.12.6/wesnoth-1.12.6.tar.bz2"),
("wesnoth-1.12.5/wesnoth-1.12.5-win32.exe", "wesnoth-1.12.6/wesnoth-1.12.6-win32.exe"),
("wesnoth-1.12.5/Wesnoth_1.12.5.dmg", "wesnoth-1.12.6/Wesnoth_1.12.6.dmg"),
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
    lines = before.split('\n')
    if website_mode:
        stringfixes = website_stringfixes
    else:
        stringfixes = game_stringfixes
    for (domain, fixes) in stringfixes.items():
        # In case of screwed-up pairs that are hard to find, uncomment the following:
        #for fix in fixes:
        #    if len(fix) != 2:
        #        print fix
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
            elif new in decommented and old in decommented and not new in old:
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
