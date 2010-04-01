#!/usr/bin/env python

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
),

"wesnoth-aoi" : (
),

"wesnoth-did" : (
),

"wesnoth-dm" : (
),

"wesnoth-ei" : (
),

"wesnoth-httt" : (
),

"wesnoth-low" : (
),

"wesnoth-manual" : (
),

"wesnoth-nr" : (
),

"wesnoth-thot" : (
),

"wesnoth-trow" : (
),

"wesnoth-tsg" : (
),

"wesnoth-sof" : (
),

"wesnoth-sotbe" :(
),

"wesnoth-tb" : (
),

"wesnoth-tutorial" : (
),

"wesnoth-units" : (
),

"wesnoth-utbs" : (
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
