
# This file loads Wesnoth specific code into gdb

"""
Usage:
1. Add these lines line into your .gdbinit that you use for wesnoth file:

#Load the wesnoth pretty-printers
python import os
python sys.path.append(os.path.abspath('utils/gdb/'))
python import wesnoth_gdb

#Get help with
python print wesnoth_gdb.__doc__

#Set expanded printing on
set print pretty on

#Hide static members
set print static-members off

"""

__doc__ = """
python reload(wesnoth_gdb)              #Interactively reload wesnoth_gdb
python wesnoth.gdb.help()               #Help message
python print wesnoth_gdb.__doc__        #Help message

python print wesnoth_gdb.set_levels_of_recursion( number )  #Sets the levels of recursion (default 1)
python print wesnoth_gdb.get_levels_of_recursion( )         #Gets the levels of recursion (default 1)

"""

import sys, gdb


def help():
    print __doc__

#Force a reload, which is handy if you are interactively editting
if 'register_wesnoth_pretty_printers' in sys.modules:
    reload(register_wesnoth_pretty_printers)
else:
    import register_wesnoth_pretty_printers

if 'wesnoth_pretty_printers' in sys.modules:
    reload(wesnoth_pretty_printers)
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
