# Tools to manipulate type

import gdb

def strip_all_type(val):
    "Strip the typename of all qualifiers and typedefs"
    # Get the type.
    type = val.type.unqualified()

    # If it points to a reference, get the reference.
    if type.code == gdb.TYPE_CODE_REF or type.code == gdb.TYPE_CODE_PTR:
        try: type = type.target().unqualified()
        except TypeError: type = val.type.unqualified()

    # Get the unqualified type, stripped of typedefs.
    type = type.strip_typedefs()

    return type

def dereference_if_possible(val):
    """ Derefence val if possible"""

    # Get the type.
    type = val.type.unqualified()

    # If it points to a reference, get the reference.
    if type.code == gdb.TYPE_CODE_PTR:
        return val.dereference()

    return val
