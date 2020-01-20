# This file loads Wesnoth specific code into gdb

"""
# Usage:
# Add these lines line into your .gdbinit file that you use for wesnoth, or add then
# to a different file and run it with gdb's -x option.

# Load the wesnoth pretty-printers. If you use a separate build tree, edit this and
# replace the os.path.abspath('utils/gdb/') with hardcoding the correct directory.
python import os
python sys.path.append(os.path.abspath('utils/gdb/'))
python import wesnoth_gdb

# Show a help banner when GDB starts
python wesnoth_gdb.help()

# Set expanded printing on, but hide static members
set print pretty on
set print static-members off
"""

documentation_banner = """Commands for controlling the Wesnoth pretty-printers:

python reload(wesnoth_gdb)              #Interactively reload wesnoth_gdb
python wesnoth_gdb.help()               #Help message

python wesnoth_gdb.set_levels_of_recursion( number )  #Sets the levels of recursion (default 1)
python print(wesnoth_gdb.get_levels_of_recursion())   #Gets the levels of recursion (default 1)
"""

import sys, gdb
import importlib


def help():
    print(documentation_banner)

#Force a reload, which is handy if you are interactively editing
if 'register_wesnoth_pretty_printers' in sys.modules:
    importlib.reload(register_wesnoth_pretty_printers)
else:
    import register_wesnoth_pretty_printers

if 'wesnoth_pretty_printers' in sys.modules:
    importlib.reload(wesnoth_pretty_printers)
else:
    import wesnoth_pretty_printers


pretty_printers_dict = {}
pretty_printers_dict = wesnoth_pretty_printers.add_printers(pretty_printers_dict)
register_wesnoth_pretty_printers.register(pretty_printers_dict)


#options

#get/set the default
def get_levels_of_recursion():
    return wesnoth_pretty_printers.RecursionManager.get_level()
def set_levels_of_recursion(num):
    return wesnoth_pretty_printers.RecursionManager.set_level(num)
