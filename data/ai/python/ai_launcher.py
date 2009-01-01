#!/usr/bin/env python
# Copyright Greg Copeland, 2008 - 2009
# Released under GPL license for Wesnoth. See Wesnoth's
# licensing terms for this module's specific license.
#

import os
import sys
import traceback

print "launch has been imported!"

def launch( script, restrict, importOnly=False ):
    # Launch one of two possible environments
    # If restrict arg is True, run inside the available
    # restrictied python environment (safe.py). If restrict
    # is False, then run without any type of restrictions at
    # all. If importOnly is True, only import but don't attempt
    # to execute.
    try:
        if restrict:
            print "restricted environment detected - running parse/safe"
            import safe
            import parse

            parse.paths = ""
            code, context = parse.parse( script )
            if not importOnly:
                safe.safe_exec( code, context, restrict )

        else:
            print "unrestricted environment detected..."
            __import__( script )
            scrpt = sys.modules[ script ]

            # Call our entry points
            if not importOnly:
                print "Running script in unrestricted environment..."
                scrpt.turn()
                print "Script has completed execution."

    except:
        err = str( traceback.format_exc() )
        raise


    print "launch has completed."




    
