#!/usr/bin/python

import sys

replacements = {
    "Bww|" : "Ww^Bw|",
    "Bww/" : "Ww^Bw/",
    "Bww\\" : "Ww^Bw\\",
    "Bwo|" : "Wo^Bw|",
    "Bwo/" : "Wo^Bw/",
    "Bwo\\" : "Wo^Bw\\",
    "Bss|" : "Ss^Bw|",
    "Bss/" : "Ss^Bw/",
    "Bss\\" : "Ss^Bw\\",
    "Chs" : "Chs^Sr",
    "Dc" : "Dd^Dc",
    "Dr" : "Dd^Dr",
    "Do" : "Dd^Do",
    "Fa" : "Aa^Fpa",
    "Fet" : "Gg^Fet",
    "Ff" : "Gs^Fp",
    "Ft" : "Gs^Ft",
    "Rfvs" : "Re^Gvs",
    "Uf" : "Uu^Uf",
    "Uui" : "Uu^Ii",
    "Uhi" : "Uh^Ii",
    "Vda" : "Dd^Vda",
    "Vdt" : "Dd^Vdt",
    "Vea" : "Aa^Vea",
    "Veg" : "Gg^Ve",
    "Vha" : "Aa^Vha",
    "Vhg" : "Gg^Vh",
    "Vhh" : "Hh^Vhh",
    "Vhha" : "Ha^Vhha",
    "Vhm" : "Mm^Vhh",
    "Vht" : "Gs^Vht",
    "Vu" : "Uu^Vu",
    "Vud" : "Uu^Vud",
    "Vwm" : "Ww^Vm",
    "Vs" : "Ss^Vhs",
    "Vsm" : "Ss^Vm",
    "Xm" : "Mm^Xm"
    }

f = open(sys.argv[1], 'r')


map = []
for line in f:
    if len(line.strip()) == 0:
        continue
    
    strings = line.split(",");
    for i, string in enumerate(strings):
        if replacements.has_key(string.strip()):
            strings[i] = replacements[string.strip()]
        else:
            strings[i] = string.strip()
    map += [strings]

f.close()


f = open(sys.argv[1], 'w')

for row in map:
    firstcol = True;
    for col in row:
        if firstcol:
           firstcol = False
        else:
            f.write(", ")
        f.write(col.ljust(12))
    f.write("\n")

f.write("\n")

f.close()
