#!/usr/bin/env python3
# encoding: utf-8

###
# Update the copyright notices year in all C++ files in src/
###

from datetime import date
from enum import Enum
import os
import re
import sys

class Found(Enum):
    MISSING = 0
    OLD = 1
    NEW = 2

previous_year = str(date.today().year-1)
if len(sys.argv) == 2:
    previous_year = sys.argv[1]

new_year = str(date.today().year)

previous_notice = r"^\tCopyright \(C\) 20[0-9]{2} - "+previous_year+"$"
current_notice = r"^\tCopyright \(C\) 20[0-9]{2} - "+new_year+"$"

extensions = re.compile(r"\..pp$|\.mm$|^wesnoth_lua_config\.h$")
old_copyright = re.compile(previous_notice)
new_copyright = re.compile(current_notice)
ignored_dirs = ["src/modules/lua", "src/modules/mariadbpp", "src/spirit_po", "src/xBRZ"]

print("Updating copyright from year '"+previous_year+"' to year '"+new_year+"'")
for root, dirs, files in os.walk('src'):
    skip = False
    for ignored_dir in ignored_dirs:
        if root.startswith(ignored_dir):
            skip = True
            break
    if skip:
        continue

    for file in files:
        if re.search(extensions, file):
            lines = []
            found = Found.MISSING
            with open(os.path.join(root, file), 'r') as f:
                lines = f.readlines()
                for index,line in enumerate(lines):
                    if re.search(old_copyright, line):
                        found = Found.OLD
                        lines[index] = lines[index][:-5] + new_year + "\n"
                        break
                    elif re.search(new_copyright, line):
                        found = Found.NEW
            if found == Found.OLD and len(lines) > 0:
                with open(os.path.join(root, file), 'w') as f:
                    f.writelines(lines)
            elif found == Found.NEW:
                print("Found existing copyright notice in file: "+root+"/"+file)
            else:
                print("Found no copyright notice to update in file: "+root+"/"+file)

print("Updated copyright from year '"+previous_year+"' to year '"+new_year+"'")
