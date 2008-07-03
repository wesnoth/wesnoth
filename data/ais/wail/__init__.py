# Wesnoth AI Library
#

__author__   = 'Greg Copeland'
__version__  = 0.1


__all__ = [ 'app', 'decorator', 'rwlock', 'basetypes' ]


# Load __all__ wail namespace so that a simple 'import wail' gives
# access to them via wail.<name>
from wesnoth import *

# Fix wesnoth functions with decorated functions here to ensure
# safe locking throughout.

