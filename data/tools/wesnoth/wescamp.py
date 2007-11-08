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
This utility provides two tools
* sync a campaign with the version on wescamp (using the packed campaign as base)
* update the translations in a campaign (in the packed campaign)
"""

import sys, os, optparse, tempfile, shutil
# in case the wesnoth python package has not been installed
sys.path.append("data/tools")
import wmldata as wmldata

#import CampaignClient as libwml
import wesnoth.campaignserver_client as libwml

#import the svn library
import wesnoth.libsvn as libsvn

class tempdir:
    def __init__(self):
        self.path = tempfile.mkdtemp()
    
        # for some reason we need to need a variable to shutil
        # otherwise the __del__() will fail
        self.dummy = shutil

    def __del__(self):
        self.dummy.rmtree(self.path)

if __name__ == "__main__":

    def extract(server, campaign, path):
        wml = libwml.CampaignClient(server)
        data = wml.get_campaign(campaign)
        wml.unpackdir(data, path, 0, True)
    

    optionparser = optparse.OptionParser("%prog [options]")

    optionparser.add_option("-u", "--upload", help = "upload a campaign to wescamp") # T

    optionparser.add_option("-d", "--download", help = "download the translations from wescamp") # 

    optionparser.add_option("-x", "--extract", help = "downloads and extracts a file from the campaign server") # T

    optionparser.add_option("-l", "--list", action = "store_true", help = "list available campaigns") # T

    optionparser.add_option("--execute", help = "executes a program on the data directory (read wmllint)") # 

    optionparser.add_option("-s", "--server", help = "server to connect to") # T

    optionparser.add_option("-t", "--target", help = "directory to write data to / read data from") # T

    optionparser.add_option("-w", "--wescamp-checkout", help = "the directory containing the wescamp checkout root") # \

    optionparser.add_option("-v", "--verbose", action = "store_true", help = "show more verbose output") # \

    optionparser.add_option("-P", "--password", help = "The master password for the campaigne server") # \

    options, args = optionparser.parse_args()

    server = "localhost"
    if(options.server != None):
        server = options.server

    target = None
    tmp = tempdir()
    if(options.target != None):
        target = options.target
    else:
        target = tmp.path

    wescamp = None
    if(options.wescamp_checkout):
        wescamp = options.wescamp_checkout

    verbose = options.verbose

    password = ""
    if(options.password):
        password = options.password

    if(options.list):
        if(server == None):
            print("No server specified")
            sys.exit(2)

        wml = libwml.CampaignClient(server)
        data = wml.list_campaigns()
        data.debug(True)

        # Item [0] hardcoded seems to work
        campaigns = data.data[0]
        for c in campaigns.get_all("campaign"):
            print c.get_text_val("title")

    elif(options.upload != None):
        if(server == None):
            print("No server specified")
            sys.exit(2)

        if(target == None):
            # FIXME this should be an optional parameter, if ommitted
            # use a temp dir which gets cleared after termination
            print("No target specified.")
            sys.exit(2)

        if(wescamp == None):
            print("No wescamp checkout specified")
            sys.exit(2)

        extract(server,  options.upload, target)
        svn = libsvn.SVN(wescamp + "/" + options.upload, 3) # debug level hard coded
        res = svn.svn_update()
        if(res == -1):
            print res.err
            sys.exit(1)
    
        res = svn.copy_to_svn(target, ["translations"])

        if(res.err != ""):
            print "Error :" + res.err
            sys.exit(1)

        res = svn.svn_commit("wescamp.py automatic update")
        if(res.err != ""):
            print "Error :" + res.err
            sys.exit(1)

    elif(options.download != None):
        if(server == None):
            print("No server specified")

        if(target == None):
            # FIXME also allow temp dir
            print("No target specified.")
            sys.exit(2)

        extract(server, options.extract, target)
        if(wescamp == None):
            print("No wescamp checkout specified")
            sys.exit(2)

        # update the wescamp checkout for the translation, 
        # if nothing changed we're done.
        # Test the entire archive now and use that, might get optimized later

        svn = libsvn.SVN(wescamp + "/" + options.download, 3) # debug level hard coded

        res = svn.svn_update()
        if(res.status == 0):
#            sys.exit(0) FIXME reenable
            pass
        elif(res.status == -1):
            sys.exit(1)

        # test whether the svn has a translations dir, if not we can stop

        # extract the campaign from the server
        extract(server, options.download, target)

        # delete translations
        if(os.path.isdir(target + "/" + options.download + "/translations")):
            shutil.rmtree(target + "/" + options.download + "/translations")

        # copy the new translations
        if(os.path.isdir(target + "/" + options.download + "/translations") == False):
            os.mkdir(target + "/" + options.download + "/translations")

        res = svn.sync_dir(svn.checkout_path + "/" + options.download + "/translations" , 
            target + "/" + options.download + "/translations", True, None) 

        # upload to the server
        wml = libwml.CampaignClient(server)
        wml.put_campaign("", options.download, "", password, "", "", "",  
            target + "/" + options.download + ".cfg",
            target + "/" + options.download)

        

    elif(options.extract != None):
        if(server == None):
            print("No server specified")
            sys.exit(2)

        if(target == None):
            print("No target specified.")
            sys.exit(2)

        extract(server, options.extract, target)

    else:
        optionparser.print_help()


