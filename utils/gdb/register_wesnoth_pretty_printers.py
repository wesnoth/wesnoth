
# This file registers pretty printers

"""
Usage:


"""

import gdb
import re
import itertools

import wesnoth_type_tools
reload(wesnoth_type_tools)
from wesnoth_type_tools import strip_all_type


class NullPointerPrinter(object):
    """Print NULL for null pointers"""
    def __init__(self, val):
        pass

    def to_string(self):
        return "NULL"

    def display_hint(self):
        return 'string'

def create_wesnoth_lookup_function(pretty_printers_dict):
    """Closure for lookup function """

    def wesnoth_lookup_function(val):
        "Look-up and return a pretty-printer that can print val."

        #If it is a null pointer or object return the null pointer printer
        if (val.type.code == gdb.TYPE_CODE_PTR and long(val) == 0) or (val.address == 0):
            return NullPointerPrinter(val)

        # Get the type name.
        type = strip_all_type(val)

        # Get the type name.
        typename = type.tag

        if typename == None:
            return None

        # Iterate over local dictionary of types to determine
        # if a printer is registered for that type.  Return an
        # instantiation of the printer if found.
        for function in pretty_printers_dict:
            if function.match(typename):
                return pretty_printers_dict[function](val)

        # Cannot find a pretty printer.  Return None.
        return None

    return wesnoth_lookup_function


def register(new_pretty_printers):
    """register the regex and printers from the dictionary with gdb"""

    #delete all previous wesnoth printers
    remove_printers=[]
    for a in gdb.pretty_printers:
        if a.__name__ == 'wesnoth_lookup_function':
            remove_printers.append(a)
            for a in remove_printers:
                gdb.pretty_printers.remove(a)

    #Add the new printers with the new dictionary
    gdb.pretty_printers.append(create_wesnoth_lookup_function(new_pretty_printers))
