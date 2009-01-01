# Copyright Greg Copeland, 2008 - 2009
# Released under GPL license for Wesnoth. See Wesnoth's
# licensing terms for this module's specific license.

# This file should only be called once per python instance. It
# sets up stderr to a per python instance file.
import os
import sys
import traceback


def initEnv( instName ):
    """
    Initialize a python AI's environment. Call exactly once per
    python instance. Call before an attempt to launch a python
    AI is made.
    """
    # Create an output filename for each
    # python instance.
    fn = "pyerr-%s.txt" % (str(instName))

    # Override stderr to write to a file
    try:
        errFile = file( fn, "wb" )
        sys.stderr = errFile

    except IOError, e:
        sys.stderr.write( "Python: Unable to create '%s'; '%s'." % (fn, str(e)) )

