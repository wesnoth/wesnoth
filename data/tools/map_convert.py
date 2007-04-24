#!/usr/bin/env python
# Script to convert from single char 1.2.x-style maps to multiple character
# 1.3.1-style maps.  This is a straight translation of the Perl created for
# regression-testing against the Perl version; the final version of the
# conversion logic will be folded into upconvert.

import sys, os, re, shutil

# These are the final translations from 1.2.x to 1.3.2
conversion = {
    " " : "_s",
    "&" : "Mm^Xm",
    "'" : "Uu^Ii",
    "/" : "Ww^Bw/",
    "1" : "1 _K",
    "2" : "2 _K",
    "3" : "3 _K",
    "4" : "4 _K",
    "5" : "5 _K",
    "6" : "6 _K",
    "7" : "7 _K",
    "8" : "8 _K",
    "9" : "9 _K",
    "?" : "Gg^Fet",
    "A" : "Ha^Vhha",
    "B" : "Dd^Vda",
    "C" : "Ch",
    "D" : "Uu^Vu",
    "E" : "Rd",
    "F" : "Aa^Fpa",
    "G" : "Gs",
    "H" : "Ha",
    "I" : "Dd",
    "J" : "Hd",
    "K" : "_K",
    "L" : "Gs^Vht",
    "M" : "Md",
    "N" : "Chr",
    "P" : "Dd^Do",
    "Q" : "Chw",
    "R" : "Rr",
    "S" : "Aa",
    "T" : "Gs^Ft",
    "U" : "Dd^Vdt",
    "V" : "Aa^Vha",
    "W" : "Xu",
    "X" : "Qxu",
    "Y" : "Ss^Vhs",
    "Z" : "Ww^Vm",
    "[" : "Uh",
    "\\": "Ww^Bw\\",
    "]" : "Uu^Uf",
    "a" : "Hh^Vhh",
    "b" : "Mm^Vhh",
    "c" : "Ww",
    "d" : "Ds",
    "e" : "Aa^Vea",
    "f" : "Gs^Fp",
    "g" : "Gg",
    "h" : "Hh",
    "i" : "Ai",
    "k" : "Wwf",
    "l" : "Ql",
    "m" : "Mm",
    "n" : "Ce",
    "o" : "Cud",
    "p" : "Uu^Vud",
    "q" : "Chs",
    "r" : "Re",
    "s" : "Wo",
    "t" : "Gg^Ve",
    "u" : "Uu",
    "v" : "Gg^Vh",
    "w" : "Ss",
    "|" : "Ww^Bw|",
    "~" : "_f",
}
max_len = max(*map(len, conversion.values()))

def get_adjacent(x, y, map):
    "Returns string of original location+adjacent locations on hex 1-char map"
    odd = (x) % 2
    adj = map[y][x];
    if x > 0:
	adj += map[y][x-1]
    if x < len(map[y])-1:
	adj += map[y][x+1]
    if y > 0:
	adj += map[y-1][x]
    if y < len(map)-1:
	adj += map[y+1][x]
    if x > 0 and y > 0 and not odd:
	adj += map[y-1][x-1]
    if x < len(map[y])-1 and y > 0 and not odd:
	adj += map[y-1][x+1];
    if x > 0 and y < len(map)-1 and odd:
	adj += map[y+1][x-1]
    if x < len(map[y])-1 and y < len(map)-1 and odd:
	adj += map[y+1][x+1]
    adj = adj.replace("\n", "").replace("\r", "")
    return adj

width = max_len+2

class maptransform_error:
    "Error object to be thrown by maptransform."
    def __init__(self, infile, inline, imap, x, y, type):
        self.infile = infile
        self.inline = inline
        self.x = x
        self.y = y
        self.type = type
    def __str__(self):
        return '"%s", line %d: %s at (%d, %d)\n' % \
                   (self.input, self.inline, self.type, self.x, self.y)

def maptransform(input, baseline, inmap, y):
    "Transform a map line from 1.2.x to 1.3.x format."
    format = "%%%d.%ds" % (width, max_len)
    x = 0
    if "," in inmap[y]:
        raise maptransform_error(input, baseline, x, y,
                                 "map file appears to be converted already")
    line = ''
    for char in inmap[y]:
        ohex = ''
        if char in ('\n', '\r'):
            ohex += char
        elif char in conversion:
            ohex = format % conversion[char] + ','
        else:
            raise maptransform_error(input, baseline+y+1, x, y,
                                     "unrecognized character %s (%d)" % (`char`, ord(char)))
            # ohex = format % char
            sys.exit(1)
        if "_K" in ohex:
            # Convert keeps according to adjacent hexes
            adj = get_adjacent(x, y, inmap)
            # print "adjacent: %s" % adj
            hexcount = {}
            for i in range(1, len(adj)):
                # Intentionally skipping 0 as it is original hex
                a = adj[i];
                if not a in conversion:
                    raise maptransform_error(input, baseline, x, y,
                                 "error in adjacent hexes")
                    sys.exit(1)
                ca = conversion[a]
                if ca.startswith("C"): #this is a castle hex	
                     hexcount[ca] = hexcount.get(ca, 0) + 1
            maxc = 0;
            maxk = "Ch";
            # Next line is a hack to make this code pass
            # regression testing against the Perl
            # original. Without the sort, when there are
            # two terrain types that occur in equal
            # numbers greater than any others, which one
            # gets picked will be randomly dependent on
            # Python's dictionary hash function.
            sorted = hexcount.keys()
            sorted.sort()
            for k in sorted:
                if hexcount[k] > maxc:
                    maxc = hexcount[k]
                    maxk = k
            #print "Dominated by %s" % maxk
            maxk = re.sub("^C", "K", maxk)
            ohex = ohex.replace("_K", maxk)
        line += ohex
        x += 1
    return line.replace(",\n", "\n")

def texttransform(input, lineno, line):
    "Identity transform on text lines."
    return line

def translator(input, output, mapxform, textxform):
    "Apply mapxform to map lines and textxform to non-map lines."
    modified = False
    # This hairy regexp excludes map_data lines that contain {} file
    # references, also lines that are empty or hold just one keep
    # character (somewhat pathological, but not handling these will
    # make the regression tests break).
    mapdata = re.compile(r'map_data="[A-Za-z0-9\/|\\&_~?\[\]\']{2,}') 
    mfile = []
    map_only = not input.endswith(".cfg")
    for line in open(input):
        mfile.append(line);
        if mapdata.search(line):
            map_only = False
    cont = False
    outmap = []
    newdata = []
    lineno = baseline = 0
    while mfile:
        line = mfile.pop(0)
        lineno += 1
        if map_only or mapdata.search(line):
            baseline = 0
            cont = True
            # Assumes map is more than 1 line long.
            if not map_only:
                line = line.split('"')[1]
            if line:
                outmap.append(line)
            while cont and mfile:
                line = mfile.pop(0)
                lineno += 1
                if line and line[0] == '#':
                    newdata.append(line)
                    continue
                if '"' in line:
                    cont = False
                    line = line.split('"')[0]
                if line and not line.endswith("\n"):
                    line += "\n"
                if line:
                    outmap.append(line)
            if not map_only: 
                line="map_data=\"\n";
                newdata.append(line)
            for y in range(len(outmap)):
                newline = mapxform(input, baseline, outmap, y)
                newdata.append(newline)
                if newline != outmap[y]:
                    modified = True
            # All lines of the map are processed, add the appropriate trailer
            if map_only:
                line="\n"
            else:
                line="\"\n"
            newdata.append(line)
        else:
            # Handle text (non-map) lines
            newline = textxform(input, lineno, line)
            newdata.append(newline)
            if newline != line:
                modified = True
    # Return None if the transformation functions made no changes.
    if modified:
        return "".join(newdata)
    else:
        return None

if __name__ == '__main__':
    if len(sys.argv) < 3 or len(sys.argv) > 4:
        print "map_convert.py map_file.cfg [new_map_file.cfg]\n"
        sys.exit(1)

    sys.argv.pop(0)
    map_file = sys.argv.pop(0)
    if not sys.argv:
        new_map_file = map_file;
        backup = map_file + ".bak";
        try:
            os.system("cp %s %s" % (map_file, backup))
        except:
            sys.stderr.write("map_convert: could not create backup file: %s\n" % backup)
            sys.exit(1)
    else:
        new_map_file = sys.argv.pop(0)
        if os.path.exists(new_map_file):
            sys.stderr.write("map_convert: new map file already exists: %s\n" %new_map_file)
            sys.exit(1)

    if not os.path.exists(map_file):
        sys.stderr.write("can not read map file: %s\n" % map_file)
        sys.exit(1)

    try:
        changed=translator(map_file, new_map_file, maptransform, texttransform)
        if changed == None:
            shutil.copy(map_file, new_map_file)
        else:
            ofp = open(new_map_file, "w");
            ofp.write(changed)
            ofp.close()
    except maptransform_error, e:
        sys.stderr.write("map_convert.py: " + `e` + "\n")
        sys.exit(1)

# map_convert ends here.


