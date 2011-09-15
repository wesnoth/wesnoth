
# This file loads Wesnoth specific code into gdb

"""
Usage:
1. Add these lines line into your .gdbinit that you use for wesnoth file:

#Load the wesnoth pretty-printers
python sys.path.append(os.path.abspath('utils/gdb/'))
python import wesnoth_gdb

#Set expanded printing on
set print pretty on

#Hide static members
set print static-members off



"""

"""
Notes
You can interactively reload it in gdb with
python reload(wesnoth_gdb)
"""


import sys, gdb

#Force a reload, which is handy if you are interactively editting 
if 'register_wesnoth_pretty_printers' in sys.modules:
    reload (register_wesnoth_pretty_printers)
else:
    import register_wesnoth_pretty_printers

if 'wesnoth_pretty_printers' in sys.modules:
    reload(wesnoth_pretty_printers)
else:
    import wesnoth_pretty_printers

pretty_printers_dict = {}
pretty_printers_dict = wesnoth_pretty_printers.add_printers(pretty_printers_dict)
register_wesnoth_pretty_printers.register(pretty_printers_dict)

 
