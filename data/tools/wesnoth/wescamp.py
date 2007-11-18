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

import sys, os, optparse, tempfile, shutil, logging, socket
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
        logging.debug("created tempdir '%s'", self.path)
    
        # for some reason we need to need a variable to shutil otherwise the 
        #__del__() will fail. This is caused by import of campaignserver_client
        # or libsvn, according to esr it's a bug in Python.
        self.dummy = shutil

    def __del__(self):
        self.dummy.rmtree(self.path)
        logging.debug("removed tempdir '%s'", self.path)

if __name__ == "__main__":

    """Download an addon from the server.

    server              The url of the addon server eg 
                        campaigns.wesnoth.org:15003.
    addon               The name of the addon.
    path                Directory to unpack the campaign in.
    returns             Nothing.
    """
    def extract(server, addon, path):
        
        logging.debug("extract addon server = '%s' addon = '%s' path = '%s'",
            server, addon, path)

        wml = libwml.CampaignClient(server)
        data = wml.get_campaign(addon)
        wml.unpackdir(data, path)

    """Get a list of addons on the server.

    server              The url of the addon server eg 
                        campaigns.wesnoth.org:15003.
    translatable_only   If True only returns translatable addons.
    returns             A dictonary with the addon as key and the translatable
                        status as value.
    """
    def list(server, translatable_only):
        
        logging.debug("list addons server = '%s' translatable_only = %s",
            server, translatable_only)

        wml = libwml.CampaignClient(server)
        data = wml.list_campaigns()

        # Item [0] hardcoded seems to work
        campaigns = data.data[0]
        result = {}
        for c in campaigns.get_all("campaign"):
            translatable = c.get_text_val("translate")
            if(translatable == "true"):
                result[c.get_text_val("name")] = True
            else:
                # when the campaign isn't marked for translation skip it
                if(translatable_only):
                    continue
                else:
                    result[c.get_text_val("name")] = False

        return result

    """Get the timestamp of a campaign on the server.

    server              The url of the addon server eg 
                        campaigns.wesnoth.org:15003.
    addon               The name of the addon.
    returns             The timestamp of the campaign, -1 if not on the server.
    """
    def get_timestamp(server, addon):

        logging.debug("get_timestamp server = '%s' addon = %s",
            server, addon)

        wml = libwml.CampaignClient(server)
        data = wml.list_campaigns()

        # Item [0] hardcoded seems to work
        campaigns = data.data[0]
        result = {}
        for c in campaigns.get_all("campaign"):
            if(c.get_text_val("name") != addon):
                continue

            return int(c.get_text_val("timestamp"))

        return -1

    """Upload a addon from the server to wescamp.
    
    server              The url of the addon server eg 
                        campaigns.wesnoth.org:15003.
    addon               The name of the addon.
    temp_dir            The directory where the unpacked campaign can be stored.
    svn_dir             The directory containing a checkout of wescamp.
    """
    def upload(server, addon, temp_dir, svn_dir):

        logging.debug("upload addon to wescamp server = '%s' addon = '%s' "
            + "temp_dir = '%s' svn_dir = '%s'", 
            server, addon, temp_dir, svn_dir)

        # Is the addon in the list with campaigns to be translated.
        campaigns = list(server, True)
        if((addon in campaigns) == False):
            logging.info("Addon '%s' is not marked as translatable "
                + "upload aborted.", addon)
            return

        # Download the addon.
        extract(server,  addon, temp_dir)

        # If the directory in svn doesn't exist we need to create and commit it.
        message = "wescamp.py automatic update"
        if(os.path.isdir(svn_dir + "/" + addon) == False):
            
            logging.info("Creating directory in svn '%s'.", 
                svn_dir + "/" + addon)

            svn = libsvn.SVN(svn_dir)

            # Don't update we're in the root and if we update the status of all
            # other campaigns is lost and no more updates are executed.
            os.mkdir(svn_dir + "/" + addon)
            res = svn.add(addon)
            res = svn.commit("wescamp_client: adding directory for initial "
                + "inclusion of addon '" + addon + "'", [addon])

        # Update the directory
        svn = libsvn.SVN(svn_dir + "/" + addon)
        svn.update()
        if(svn.copy_to_svn(temp_dir, ["translations"])):
            svn.commit("wescamp_client: automatic update of addon '"
                + addon + "'")
            logging.info("New version of addon '%s' uploaded.", addon)
        else:
            logging.info("Addon '%s' hasn't been modified, thus not uploaded.",
                addon)

    """Update the translations from wescamp to the server.
    
    server              The url of the addon server eg 
                        campaigns.wesnoth.org:15003.
    addon               The name of the addon.
    temp_dir            The directory where the unpacked campaign can be stored.
    svn_dir             The directory containing a checkout of wescamp.
    password            The master upload password.
    stamp               Only upload if the timestamp is equal to this value
                        if None it's ignored. This is needed to avoid an upload
                        of wescamp overwriting a campaign authors fresh upload,
                        there's still a small possibility of a race condition
                        but it would be really bad luck if that happens.
    returns             if stamp is used it returns False if the upload failed
                        due to a newer version on the server, True otherwise.
    """
    def download(server, addon, temp_dir, svn_dir, password, stamp = None):

        logging.debug("download addon from wescamp server = '%s' addon = '%s' "
            + "temp_dir = '%s' svn_dir = '%s' password is not shown", 
            server, addon, temp_dir, svn_dir)


        extract(server, addon, temp_dir)

        # update the wescamp checkout for the translation, 
        # if nothing changed we're done.
        # Test the entire archive now and use that, might get optimized later

        svn = libsvn.SVN(wescamp + "/" + addon)

        # The result of the update can be ignored, no changes when updating
        # doesn't mean no changes to the translations.
        svn.update()

        # test whether the svn has a translations dir, if not we can stop
        if(os.path.isdir(wescamp + "/" 
            + addon + "/" + addon + "/translations") == False):

            logging.info("Wescamp has no translations directory so we can stop.")
            if(stamp == None):
                return
            else:
                return True


        # extract the campaign from the server
        extract(server, addon, target)

        # delete translations, but make sure the translations 
        # directory exists afterwards.
        if(os.path.isdir(target + "/" + addon + "/translations")):
            shutil.rmtree(target + "/" + addon + "/translations")

        os.mkdir(target + "/" + addon + "/translations")

        # copy the translations
        svn = libsvn.SVN(wescamp + "/" + addon + "/" + addon + "/translations")
        svn.copy_from_svn(target + "/" + addon + "/translations", None)

        # upload to the server
        wml = libwml.CampaignClient(server)
        if(stamp == None):
            wml.put_campaign("", addon, "", password, "", "", "",  
                target + "/" + addon + ".cfg", target + "/" + addon)
        else:
            if(stamp == get_timestamp(server, addon)):
                wml.put_campaign("", addon, "", password, "", "", "",  
                    target + "/" + addon + ".cfg", target + "/" + addon)
                return True
            else:
                return False

    def erase(addon, svn_dir):

        logging.debug("Erase addon from wescamp addon = '%s' svn_dir = '%s'",
            addon, svn_dir)

        svn = libsvn.SVN(wescamp)

        svn.update(None, addon) 

        svn.remove(addon)

        svn.commit("Erasing addon " + addon)

    optionparser = optparse.OptionParser("%prog [options]")

    optionparser.add_option("-l", "--list", action = "store_true", 
        help = "List available addons. Usage [SERVER [PORT] [VERBOSE]")

    optionparser.add_option("-L", "--list-translatable", action = "store_true", 
        help = "List addons available for translation. "
        + "Usage [SERVER [PORT] [VERBOSE]")

    optionparser.add_option("-u", "--upload", 
        help = "Upload a addon to wescamp. Usage: 'addon' WESCAMP-CHECKOUT "
        + "[SERVER [PORT]] [TEMP-DIR] [VERBOSE]")

    optionparser.add_option("-U", "--upload-all", action = "store_true", 
        help = "Upload all addons to wescamp. Usage WESCAMP-CHECKOUT "
        + " [SERVER [PORT]] [VERBOSE]")

    optionparser.add_option("-d", "--download", 
        help = "Download the translations from wescamp and upload to the addon "
        + "server. Usage 'addon' WESCAMP-CHECKOUT PASSWORD [SERVER [PORT]] "
        + "[TEMP-DIR] [VERBOSE]")

    optionparser.add_option("-D", "--download-all", action = "store_true", 
        help = "Download all translations from wescamp and upload them to the "
        + "addon server. Usage WESCAMP-CHECKOUT PASSWORD [SERVER [PORT]] "
        + " [VERBOSE]")

    optionparser.add_option("-e", "--erase",
        help = "Erase an addon from wescamp. Usage 'addon' WESCAMP-CHECKOUT "
                + "[VERBOSE]")

    optionparser.add_option("-s", "--server", 
        help = "Server to connect to [localhost]")

    optionparser.add_option("-p", "--port", 
        help = "Port on the server to connect to ['']") 

    optionparser.add_option("-t", "--temp-dir", help = "Directory to store the "
        + "tempory data, if omitted a tempdir is created and destroyed after "
        + "usage, if specified the data is left in the tempdir. ['']")

    optionparser.add_option("-w", "--wescamp-checkout", 
        help = "The directory containing the wescamp checkout root. ['']") 

    optionparser.add_option("-v", "--verbose", action = "store_true", 
        help = "Show more verbose output. [FALSE]")

    optionparser.add_option("-P", "--password", 
        help = "The master password for the addon server. ['']")

    options, args = optionparser.parse_args()

    if(options.verbose):
        logging.basicConfig(level=logging.DEBUG, 
            format='[%(levelname)s] %(message)s')
    else:
        logging.basicConfig(level=logging.INFO, 
            format='[%(levelname)s] %(message)s')

    server = "localhost"
    if(options.server != None):
        server = options.server

    if(options.port != None):
        server += ":" + options.port

    target = None
    tmp = tempdir()
    if(options.temp_dir != None):
        if(options.download_all != None):
            logging.error("TEMP-DIR not allowed for DOWNLOAD-ALL.")
            sys.exit(2)

        if(options.upload_all != None):
            logging.error("TEMP-DIR not allowed for UPLOAD-ALL.")
            sys.exit(2)

        target = options.temp_dir
    else:
        target = tmp.path

    wescamp = None
    if(options.wescamp_checkout):
        wescamp = options.wescamp_checkout

    password = options.password

    # List the addons on the server and optional filter on translatable
    # addons.
    if(options.list or options.list_translatable):

        try:
            addons = list(server, options.list_translatable)
        except libsvn.error, e:
            print "[ERROR svn] " + str(e)
            sys.exit(1)
        except socket.error, e:
            print "Socket error: " + str(e)
            sys.exit(e[0])
        except IOError, e:
            print "Unexpected error occured: " + str(e)
            sys.exit(e[0])

        for k, v in addons.iteritems():
            if(v):
                print k + " translatable"
            else:
                print k

    # Upload an addon to wescamp.
    elif(options.upload != None):

        if(wescamp == None):
            logging.error("No wescamp checkout specified")
            sys.exit(2)

        try:
            upload(server, options.upload, target, wescamp)
        except libsvn.error, e:
            print "[ERROR svn] " + str(e)
            sys.exit(1)
        except socket.error, e:
            print "Socket error: " + str(e)
            sys.exit(e[0])
        except IOError, e:
            print "Unexpected error occured: " + str(e)
            sys.exit(e[0])

    # Upload all addons from wescamp.
    elif(options.upload_all != None):

        if(wescamp == None):
            logging.error("No wescamp checkout specified.")
            sys.exit(2)

        error = False
        try:
            addons = list(server, True)
        except socket.error, e:
            print "Socket error: " + str(e)
            sys.exit(e[0])
        for k, v in addons.iteritems():
            try:
                logging.info("Processing addon '%s'", k)
                # Create a new temp dir for every upload.
                tmp = tempdir()
                upload(server, k, tmp.path, wescamp)
            except libsvn.error, e:
                print "[ERROR svn] in addon '" + k + "'" + str(e)
                error = True
            except socket.error, e:
                print "Socket error: " + str(e)
                error = True
            except IOError, e:
                print "Unexpected error occured: " + str(e)
                error = True
        
        if(error):
            sys.exit(1)

    # Download an addon from wescamp.
    elif(options.download != None):

        if(wescamp == None):
            logging.error("No wescamp checkout specified.")
            sys.exit(2)

        if(password == None):
            logging.error("No upload password specified.")
            sys.exit(2)

        try:
            download(server, options.download, target, wescamp, password)
        except libsvn.error, e:
            print "[ERROR svn] " + str(e)
            sys.exit(1)
        except socket.error, e:
            print "Socket error: " + str(e)
            sys.exit(e[0])
        except IOError, e:
            print "Unexpected error occured: " + str(e)
            sys.exit(e[0])

    # Download all addons from wescamp.
    elif(options.download_all != None):

        if(wescamp == None):
            logging.error("No wescamp checkout specified.")
            sys.exit(2)

        if(password == None):
            logging.error("No upload password specified.")
            sys.exit(2)

        error = False
        try:
            addons = list(server, True)
        except socket.error, e:
            print "Socket error: " + str(e)
            sys.exit(e[0])
        for k, v in addons.iteritems():
            try:
                # since we modify the data on the campaign server and the author
                # can do the same we need to try to minimize the odds of our
                # upload to wipe out the new upload

                logging.info("Processing addon '%s'", k)
                while(True): # download loop

                    timestamp = 0
                    while(True): # upload loop
                        
                        # get the upload timestamp of the addon
                        timestamp = get_timestamp(server, k)

                        # upload in wescamp
                        tmp = tempdir()
                        upload(server, k , tmp.path, wescamp)

                        # if the timestamp has changed we need to download again
                        if(get_timestamp(server, k) == timestamp):
                            break
                        else:
                            logging.warning("Addon '%s' has been modified on "
                                + "the campaign server, force another" 
                                + "wescamp sync", k)

                    # Create a new temp dir for every download.
                    tmp = tempdir()
                    if(download(server, k, tmp.path, wescamp, password, timestamp)):
                        break
                    else:
                        logging.warning("Addon '%s' has been modified on "
                            + "the campaign server and isn't uploaded "
                            + "force another full sync cycle", k)

            except libsvn.error, e:
                print "[ERROR svn] in addon '" + k + "'" + str(e)
                error = True
            except socket.error, e:
                print "Socket error: " + str(e)
                error = True
            except IOError, e:
                print "Unexpected error occured: " + str(e)
                error = True
        
        if(error):
            sys.exit(1)

    # Erase an addon from wescamp
    elif(options.erase != None):

        if(wescamp == None):
            logging.error("No wescamp checkout specified.")
            sys.exit(2)

        try:
            erase(options.erase, wescamp)
        except libsvn.error, e:
            print "[ERROR svn] " + str(e)
            sys.exit(1)
        except socket.error, e:
            print "Socket error: " + str(e)
            sys.exit(e[0])
        except IOError, e:
            print "Unexpected error occured: " + str(e)
            sys.exit(e[0])

    else:
        optionparser.print_help()


