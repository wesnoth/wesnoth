#!/bin/env python
# Copyright Greg Copeland, 2008
# Released under GPL license for Wesnoth. See Wesnoth's
# licensing terms for this module's specific license.
#

import os
import sys
import traceback

print "launch has been imported!"

def launch( script, restrict ):
    # Launch one of two possible environments
    # If restrict arg is True, run inside the available
    # restrictied python environment (safe.py). If restrict
    # is False, then run without any type of restrictions.
    if restrict:
        print "restricted environment detected - running parse/safe"
        try:
            import safe
            import parse

            parse.paths = ""
            code, context = parse.parse( script )
            safe.safe_exec( code, context, restrict )

        except:
            err = str( traceback.format_exc() )
            raise

    else:
        print "unrestricted environment detected...running directly."
        __import__( script )
        scrpt = sys.modules[ script ]

        # Call our entry points
        scrpt.start()
        scrpt.turn()
        scrpt.end()

    print "launch has completed."




    
