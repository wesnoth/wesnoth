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

import sys, os.path, optparse, tempfile, shutil, logging, socket
# in case the wesnoth python package has not been installed
sys.path.append("data/tools")

#import CampaignClient as libwml
import wesnoth.campaignserver_client as libwml

#import the github library
import wesnoth.libgithub as libgithub


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
    git_version = None
    git_userpass = None
    quiet_libwml = True

    """Initialize the build-system.

    addon_obj           libgithub.Addon objectof the addon.
    addon_name          Name of the addon.
    wescamp_dir         The directory containing a checkout of wescamp.
    build_sys_dir       Possible directory containing a checkout of build-system.
    """
    def init_build_system(addon_obj, addon_name, wescamp_dir, build_sys_dir):
        # Grab the build system
        below_branch = os.path.basename(wescamp_dir.rstrip(os.sep))
        possible_build_paths = []
        if build_sys_dir:
            possible_build_paths.append(build_sys_dir)
        possible_build_paths.append(os.path.join(below_branch, "build-system"))
        build_system = libgithub.get_build_system(possible_build_paths)
        build_system.update()
        init_script = os.path.join(build_system.get_dir(), "init-build-sys.sh")

        # Uglyness
        out, err = addon_obj._execute([init_script, "--{0}".format(git_version), addon_name, "."], check_error=False)
        if len(err):
            logging.warn("In add-on {0}:\n{1}".format(addon_name, err))
            #TODO: bail?

        if not out.strip().endswith("Done."):
            logging.error("Failed to init the build-system for addon {0}".format(addon_name))
            return

        addon_obj._execute(["git", "add", "po", "campaign.def", "Makefile"], check_error=True)
        addon_obj.commit("wescamp.py: Initialize build-system")

    """Update the translation catalogs.

    addon_obj           libgithub.Addon objectof the addon.
    addon_name          Name of the addon.
    """
    def pot_update(addon_obj, addon_name):
        if not os.path.exists(os.path.join(addon_obj.get_dir(), "Makefile")):
            logging.warn("Cannot pot-update: build system does not exist for add-on {0}.".format(addon_name))
            return
        # Uglyness, again
        out, err = addon_obj._execute(["make"])
        if len(err):
            logging.warn("In addon {0}:\n{1}".format(addon_name, err))
            # TODO: bail?
        outlines = addon_obj._status()

        to_rm = []
        to_add = []
        longname = "wesnoth-{0}".format(addon_name)
        for line in outlines:
            mod, name = line.split()
            if mod == "D":
                to_rm.append(name)
            elif mod == "M" and name.endswith((".po", "LC_MESSAGES/{0}.mo".format(longname), "po/{0}.pot".format(longname), "po/Makefile")):
                to_add.append(name)
            else:
                logging.info("Ignoring {0}".format(line))
        if to_rm:
            addon_obj._execute(["git", "rm"] + to_rm, check_error=True)
        if to_add:
            addon_obj._execute(["git", "add"] + to_add, check_error=True)
        addon_obj.commit("wescamp.py: pot-update")

    """Download an addon from the server.

    server              The url of the addon server eg
                        add-ons.wesnoth.org:15005.
    addon               The name of the addon.
    path                Directory to unpack the campaign in.
    returns             Nothing.
    """
    def extract(server, addon, path):

        logging.debug("extract addon server = '%s' addon = '%s' path = '%s'",
            server, addon, path)

        wml = libwml.CampaignClient(server, quiet_libwml)
        data = wml.get_campaign(addon)
        wml.unpackdir(data, path)

    """Get a list of addons on the server.

    server              The url of the addon server eg
                        add-ons.wesnoth.org:15005.
    translatable_only   If True only returns translatable addons.
    returns             A dictonary with the addon as key and the translatable
                        status as value.
    """
    def list_addons(server, translatable_only):

        logging.debug("list addons server = '%s' translatable_only = %s",
            server, translatable_only)

        wml = libwml.CampaignClient(server, quiet_libwml)
        data = wml.list_campaigns()

        # Item [0] hardcoded seems to work
        campaigns = data.data[0]
        result = {}
        for c in campaigns.get_all("campaign"):
            translatable = c.get_text_val("translate")
            if(translatable == "yes" or translatable == "on" or translatable == "true" or translatable == "1"):
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
                        add-ons.wesnoth.org:15005.
    addon               The name of the addon.
    returns             The timestamp of the campaign, -1 if not on the server.
    """
    def get_timestamp(server, addon):

        logging.debug("get_timestamp server = '%s' addon = %s",
            server, addon)

        wml = libwml.CampaignClient(server, quiet_libwml)
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
                        add-ons.wesnoth.org:15005.
    addon               The name of the addon.
    temp_dir            The directory where the unpacked campaign can be stored.
    wescamp_dir         The directory containing a checkout of wescamp.
    build_sys_dir       Possible directory containing a checkout of build-system.
    """
    def upload(server, addon, temp_dir, wescamp_dir, build_sys_dir=None):

        logging.debug("upload addon to wescamp server = '%s' addon = '%s' "
            + "temp_dir = '%s' wescamp_dir = '%s'",
            server, addon, temp_dir, wescamp_dir)

        # Is the addon in the list with campaigns to be translated.
        campaigns = list_addons(server, True)
        if((addon in campaigns) == False):
            logging.info("Addon '%s' is not marked as translatable "
                + "upload aborted.", addon)
            return

        # Download the addon.
        extract(server,  addon, temp_dir)

        github = libgithub.GitHub(wescamp_dir, git_version, userpass=git_userpass)

        is_new_addon = False
        has_updated = False

        # If the checkout doesn't exist we need to create it.
        if(os.path.isdir(os.path.join(wescamp_dir, addon)) == False):

            logging.info("Checking out '%s'.",
                os.path.join(wescamp_dir, addon))

            if not github.addon_exists(addon):
                github.create_addon(addon)

                is_new_addon = True

        # Update the directory
        addon_obj = github.addon(addon)
        addon_obj.update()
        # Translation needs to be prevented from the campaign to overwrite
        # the ones in wescamp.
        # The other files are present in wescamp and shouldn't be deleted.
        ignore_list = ["translations", "po", "campaign.def",
            "config.status", "Makefile"]
        if(addon_obj.sync_from(temp_dir, ignore_list)):

            addon_obj.commit("wescamp.py: Update from add-on server")
            logging.info("New version of addon '%s' uploaded.", addon)
            has_updated = True
        else:
            logging.info("Addon '%s' hasn't been modified, thus not uploaded.",
                addon)

        if is_new_addon:
            init_build_system(addon_obj, addon, wescamp_dir, build_sys_dir)

        if has_updated:
            pot_update(addon_obj, addon)


    """Checkout all add-ons of one wesnoth version from wescamp.

    wescamp             The directory where all checkouts should be stored.
    userpass            Authentication data. Shouldn't be needed.
    readonly            Makes a read-only checkout that doesn't require authentication.
    """
    def checkout(wescamp, userpass=None, readonly=False):

        logging.debug("checking out add-ons from wesnoth version = '%s' to directory '%s'", git_version, wescamp)

        github = libgithub.GitHub(wescamp, git_version, userpass=git_userpass)

        for addon in github.list_addons():
            addon_obj = github.addon(addon, readonly=readonly)
            addon_obj.update()


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

    optionparser.add_option("-s", "--server",
        help = "Server to connect to [localhost]")

    optionparser.add_option("-p", "--port",
        help = "Port on the server to connect to ['']")

    optionparser.add_option("-t", "--temp-dir", help = "Directory to store the "
        + "tempory data, if omitted a tempdir is created and destroyed after "
        + "usage, if specified the data is left in the tempdir. ['']")

    optionparser.add_option("-w", "--wescamp-checkout",
        help = "The directory containing the wescamp checkout root. ['']")

    optionparser.add_option("-v", "--verbose", action = "store_const", const="verbose", dest="verbosity",
        help = "Show more verbose output. [FALSE]")

    optionparser.add_option("-q", "--quiet", action = "store_const", const="quiet", dest="verbosity",
        help = "Show less verbose output. [FALSE]")

    optionparser.add_option("-P", "--password",
        help = "The master password for the addon server. ['']")

    optionparser.add_option("-g", "--git",
        action = "store_true",
        help = "Use git instead of svn to interface with wescamp. "
        + "This is a temporary option for the conversion from berlios to github.")

    optionparser.add_option("-G", "--github-login",
        help = "Username and password for github in the user:pass format")

    optionparser.add_option("-c", "--checkout", action = "store_true",
        help = "Create a new branch checkout directory. "
        + "Can also be used to update existing checkout directories.")

    optionparser.add_option("-C", "--checkout-readonly", action = "store_true",
        help = "Create a read-only branch checkout directory. "
        + "Can also be used to update existing checkout directories.")

    optionparser.add_option("-b", "--build-system",
        help = "Path to a github.com/wescamp/build-system checkout.")

    options, args = optionparser.parse_args()

    if(options.verbosity == "verbose"):
        logging.basicConfig(level=logging.DEBUG,
            format='[%(levelname)s] %(message)s',
            stream=sys.stdout)
        quiet_libwml = False
    elif(options.verbosity == "quiet"):
        logging.basicConfig(level=logging.WARN,
            format='[%(levelname)s] %(message)s',
            stream=sys.stdout)
    else:
        logging.basicConfig(level=logging.INFO,
            format='[%(levelname)s] %(message)s',
            stream=sys.stdout)

    server = "localhost"
    if(options.server != None):
        server = options.server

    if(options.port != None):
        server += ":" + options.port

    target = None
    tmp = tempdir()
    if(options.temp_dir != None):
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
    build_sys_dir = options.build_system

    if(options.git):
        logging.warn("--git is no longer required, as svn is no longer supported")
        #TODO: remove entirely
    git_userpass = options.github_login
    if not wescamp:
        logging.error("No wescamp checkout specified. Needed for git usage.")
        sys.exit(2)
    try:
        git_version = wescamp.split("-")[-1].strip("/").split("/")[-1]
    except:
        logging.error("Wescamp directory path does not end in a version suffix. Currently needed for git usage.")
        sys.exit(2)

    # List the addons on the server and optional filter on translatable
    # addons.
    if(options.list or options.list_translatable):

        try:
            addons = list_addons(server, options.list_translatable)
        except libgithub.AddonError, e:
            print "[ERROR github in {0}] {1}".format(e.addon, str(e.message))
            sys.exit(1)
        except libgithub.Error, e:
            print "[ERROR github] " + str(e)
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
            upload(server, options.upload, target, wescamp, build_sys_dir)
        except libgithub.AddonError, e:
            print "[ERROR github in {0}] {1}".format(e.addon, str(e.message))
            sys.exit(1)
        except libgithub.Error, e:
            print "[ERROR github] " + str(e)
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
            addons = list_addons(server, True)
        except socket.error, e:
            print "Socket error: " + str(e)
            sys.exit(e[0])
        for k, v in addons.iteritems():
            try:
                logging.info("Processing addon '%s'", k)
                # Create a new temp dir for every upload.
                tmp = tempdir()
                upload(server, k, tmp.path, wescamp, build_sys_dir)
            except libgithub.AddonError, e:
                print "[ERROR github in {0}] {1}".format(e.addon, str(e.message))
                error = True
            except libgithub.Error, e:
                print "[ERROR github] in addon '" + k + "'" + str(e)
                error = True
            except socket.error, e:
                print "Socket error: " + str(e)
                error = True
            except IOError, e:
                print "Unexpected error occured: " + str(e)
                error = True

        if(error):
            sys.exit(1)

    elif(options.checkout != None or options.checkout_readonly != None):

        if(wescamp == None):
            logging.error("No wescamp checkout specified.")
            sys.exit(2)

        try:
            checkout(wescamp, userpass=git_userpass, readonly=(options.checkout_readonly != None))
        except libgithub.AddonError, e:
            print "[ERROR github in {0}] {1}".format(e.addon, str(e.message))
            sys.exit(1)
        except libgithub.Error, e:
            print "[ERROR github] " + str(e)
            sys.exit(1)
        except socket.error, e:
            print "Socket error: " + str(e)
            sys.exit(e[0])
        except IOError, e:
            print "Unexpected error occured: " + str(e)
            sys.exit(e[0])

    else:
        optionparser.print_help()
