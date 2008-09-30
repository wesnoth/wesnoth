# Wesnoth AI Library
#

__author__   = 'Greg Copeland'
__version__  = 0.1


__all__ = [ 'app', 'basetype', 'decorator', 'rwlock' ]


# Load __all__ wail namespace so that a simple 'import wail' gives
# access to them via wail.<name>
from wesnoth import *
print "============================================="
print "'wesnoth' module has been absorbed into wail."
print "============================================="

##import app
##import basetype
##import decorator
##import rwlock

# Fix wesnoth functions with decorated functions here to ensure
# safe locking throughout.

print "============================================="
print "wail module has been imported."
print "============================================="
