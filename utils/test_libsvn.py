#!/usr/bin/env python
# vim: tabstop=4: shiftwidth=4: expandtab: softtabstop=4: autoindent:
# $Id$  
"""             
   Copyright (C) 2007 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
"""

"""
This module is meant to test the libsvn.py library.
"""

import sys, os.path, optparse
# in case the wesnoth python package has not been installed
sys.path.append("data/tools")
import wesnoth.libsvn as libsvn

"""
Evaluates the result send from the library.
"""
def evaluate(res):
    if(options.verbose):
        print "Status:", res.status 
        print res.out
    if(res.status == -1):
        print "Error: " + res.err

"""
The main program.
"""
if __name__ == "__main__":

    optionparser = optparse.OptionParser("%prog [options] path")

    # -o file:///home/mark/addond/repo /tmp/checkout
    optionparser.add_option("-o", "--checkout", help = "checkout a repo") #   V

    # -u /tmp/checkout
    optionparser.add_option("-u", "--update", help = "update a local copy",
        action = "store_true") #   V

    # -u --rev=1 /tmp/checkout
    optionparser.add_option("--rev", help = "update to a certain revision") #   V

    # -c "foo bar" /tmp/checkout/
    optionparser.add_option("-c", "--commit", help = "commits a change") # V
    
    # NOTE the checkout is overkill maybe not make it mandatory after all
    # -a /tmp/checkout/bar /tmp/checkout
    optionparser.add_option("-a", "--add", help = "add a file / directory") #   V

    # NOTE the checkout is overkill maybe not make it mandatory after all
    # -a /tmp/checkout/bar /tmp/checkout
    optionparser.add_option("-r", "--remove", help = "remove a file / directory") #   V

    optionparser.add_option("-s", "--sync", help = 
        "syncs the local checkout with a separate directory, requires PATH") #   \

    # just add a v to a command
    optionparser.add_option("-v", "--verbose", help = "show verbose output",
        action = "store_true") #   V

    optionparser.add_option("-f", "--files", help = 
        "do action only for selected files (only in combination with update/checkout)")

    optionparser.add_option("-e", "--exclude", help = 
        "exclude selected files (only in combination with sync)")

    optionparser.add_option("-t", "--stat", help = "gives the status of a file")

    options, args = optionparser.parse_args()
    if(len(args) != 1):
        optionparser.error("incorrect number of arguments")

    svn = libsvn.SVN(args[0])

### checkout
    if(options.checkout != None):

        # print debug info
        if(options.verbose):
            print "checkout " + options.checkout + " to " + args[0]

        # fire command
        res = svn.checkout(options.checkout)

        # evaluate result
        evaluate(res)

### update
    elif(options.update):

        # print debug info
        if(options.verbose):
            msg = "update " + args[0]
            if(options.rev != None):
                msg += " to rev " + options.rev

            print msg

        # fire command
        res = svn.update(options.rev)

        # evaluate result
        evaluate(res)

### add
    elif(options.add != None):

        # print debug info
        if(options.verbose):
            print "add " + options.add + " to " + args[0]

        # fire command
        res = svn.add(options.add)

        # evaluate result
        evaluate(res)

### remove
    elif(options.remove != None):

        # print debug info
        if(options.verbose):
            print "remove " + options.remove + " from " + args[0]

        # fire command
        res = svn.remove(options.remove)

        # evaluate result
        evaluate(res)

### commit
    elif(options.commit != None):

        # print debug info
        if(options.verbose):
            print "commit " + args[0] + " message: " + options.commit

        # fire command
        res = svn.commit(options.commit)

        # evaluate result
        evaluate(res)

### sync
    elif(options.sync != None):

        # print debug info
        if(options.verbose):
            print "sync " + args[0] + " path " + options.sync

        # do some sanity checks
        if(not(os.path.isdir(args[0]))):
            print "repo not found " + args[0]
            sys.exit()

        if(not(os.path.isdir(options.sync))):
            print "path not found " + options.sync
            sys.exit()

        # fire command
        res = svn.sync(options.sync)

        # evaluate result
        evaluate(res)

### status
    elif(options.stat != None):

        # print debug info
        if(options.verbose):
            print "status " + options.stat + " of " + args[0]

        # fire command
        res = svn.status(options.stat)

        # evaluate result
        evaluate(res)

    else:
        optionparser.print_help()

