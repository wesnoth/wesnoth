#!/usr/bin/env python3
# vim: tabstop=4: shiftwidth=4: expandtab: softtabstop=4: autoindent:
#
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

import sys, os.path, argparse, tempfile, shutil, logging, socket
# in case the wesnoth python package has not been installed
sys.path.append("data/tools")

#import CampaignClient as libwml
import wesnoth.campaignserver_client as libwml

#import the github library
import wesnoth.libgithub as libgithub

# Some constants
BUILDSYS_FILE = "build-system.version"
ADDONVER_FILE = "addon.timestamp"

class tempdir:
    def __init__(self):
        self.path = tempfile.mkdtemp()
        logging.debug("created tempdir '%s'", self.path)

        # We need to add a reference to shutil, otherwise __del__() will fail.
        # This is because when the object is destructed other globals may
        # have already been torn down.
        # In C++ this is known as the static deinitialization fiasco.
        self.shutil = shutil
        self.logging = logging

    def __del__(self):
        self.shutil.rmtree(self.path)
        self.logging.debug("removed tempdir '%s'", self.path)

if __name__ == "__main__":
    git_version = None
    git_auth = None
    quiet_libwml = True

    def update_addon(addon_obj, addon_name, addon_server, temp_dir):
        """Update the add-on from addon-server

        addon_obj       Github.Addon object for the add-on
        addon_name      Name of the add-on
        addon_server    Url to the add-on server
        temp_dir        The directory where the unpacked campaign can be stored.

        returns         Whether anything was changed.
        """
        # Grab timestamp from server
        server_timestamp = get_timestamp(addon_server, addon_name)

        # Check local timestamp to see if the add-on server version is newer
        if os.path.exists(os.path.join(addon_obj.get_dir(), ADDONVER_FILE)):
            with open(os.path.join(addon_obj.get_dir(), ADDONVER_FILE), "r") as stamp_file:
                str_timestamp = stamp_file.read()
            try:
                local_timestamp = int(str_timestamp)
                if local_timestamp == server_timestamp:
                    logging.info("Addon '%s' is up-to-date.", addon_name)
                    return False
            except:
                pass

        # Download the addon.
        extract(addon_server, addon_name, temp_dir)

        # Translation needs to be prevented from the campaign to overwrite
        # the ones in wescamp.
        # The other files are present in wescamp and shouldn't be deleted.
        ignore_list = ["translations", "po", "campaign.def",
            "config.status", "Makefile", BUILDSYS_FILE, ADDONVER_FILE]
        if addon_obj.sync_from(temp_dir, ignore_list):
            # Store add-on timestamp
            with open(os.path.join(addon_obj.get_dir(), ADDONVER_FILE), "w") as timestamp_file:
                timestamp_file.write(str(server_timestamp))
            addon_obj._execute(["git", "add", ADDONVER_FILE])

            addon_obj.commit("wescamp.py: Update from add-on server")
            logging.info("New version of addon '%s' uploaded.", addon_name)
            return True
        else:
            logging.info("Addon '%s' hasn't been modified, thus not uploaded.",
                addon_name)
            return False

    def update_build_system(addon_obj, addon_name, wescamp_dir, build_sys_dir):
        """Initialize or update the build-system.

        addon_obj           libgithub.Addon objectof the addon.
        addon_name          Name of the addon.
        wescamp_dir         The directory containing a checkout of wescamp.
        build_sys_dir       Possible directory containing a checkout of build-system.
        returns             Boolean indicating whether the add-on now has a build-system.
        """
        logging.info("Checking if build system for add-on {0} needs to be updated".format(addon_name))
        previously_initialized = os.path.exists(os.path.join(addon_obj.get_dir(), "Makefile"))

        # Grab the build system
        below_branch = os.path.basename(wescamp_dir.rstrip(os.sep))
        possible_build_paths = []
        if build_sys_dir:
            possible_build_paths.append(build_sys_dir)
        possible_build_paths.append(os.path.join(below_branch, "build-system"))
        build_system = libgithub.get_build_system(possible_build_paths)
        init_script = os.path.join(build_system.get_dir(), "init-build-sys.sh")

        # Grab master build system's version
        # Ugliness
        out, err, res = build_system._execute(["git", "show", "--pretty=oneline", "--summary"])
        build_system_version = out.split()[0]
        if len(build_system_version) != 40:
            logging.warn("Incorrect SHA1 for build system checkout: {0}".format(build_system_version))

        # Check if build system version in add-on is up-to-date
        if os.path.exists(os.path.join(addon_obj.get_dir(), BUILDSYS_FILE)):
            with open(os.path.join(addon_obj.get_dir(), BUILDSYS_FILE), "r") as stamp_file:
                addon_build_version = stamp_file.read()
            if addon_build_version == build_system_version:
                logging.info("Build system for add-on {0} is up-to-date".format(addon_name))
                return True

        # Ugliness
        out, err, res = addon_obj._execute([init_script, "--{0}".format(git_version), addon_name, "."], check_error=False)
        if len(err):
            logging.warn("init-build-sys.sh in add-on {0}:\n{1}".format(addon_name, err))

        if not out.strip().endswith("Done.") or res != 0:
            logging.error("Failed to init the build-system for add-on {0}".format(addon_name))
            addon_obj._execute(["rm", "-rf", "po", "campaign.def", "Makefile"])
            addon_obj._execute(["git", "reset", "--hard"])
            return False

        # Store build system version
        with open(os.path.join(addon_obj.get_dir(), BUILDSYS_FILE), "w") as version_file:
            version_file.write(build_system_version)

        addon_obj._execute(["git", "add", "po", "campaign.def", "Makefile", BUILDSYS_FILE], check_error=True)
        if previously_initialized:
            logging.info("Updated build system for add-on {0}".format(addon_name))
            addon_obj.commit("wescamp.py: Update build-system")
        else:
            logging.info("Initialized build system for add-on {0}".format(addon_name))
            addon_obj.commit("wescamp.py: Initialize build-system")
        return True

    def pot_update(addon_obj, addon_name):
        """Update the translation catalogs.

        addon_obj           libgithub.Addon objectof the addon.
        addon_name          Name of the addon.
        returns             Boolean indicating whether the operation was successful.
        """
        if not os.path.exists(os.path.join(addon_obj.get_dir(), "Makefile")):
            logging.warn("Cannot pot-update: build system does not exist for add-on {0}.".format(addon_name))
            return False
        # Ugliness, again
        out, err, res = addon_obj._execute(["make"])
        if len(err):
            logging.warn("pot-update in addon {0}:\n{1}".format(addon_name, err))
            # TODO: bail?

        if res != 0:
            logging.error("Failed to pot-update for add-on {0}".format(addon_name))
            addon_obj._execute(["rm", "-rf", "po", addon_name])
            addon_obj._execute(["git", "reset", "--hard"])
            return False

        outlines = addon_obj._status()

        to_rm = []
        to_add = []
        longname = "wesnoth-{0}".format(addon_name)
        for line in outlines:
            mod, name = line.split()
            if mod == "D":
                to_rm.append(name)
            elif mod == "M" and name.endswith((".po", "LC_MESSAGES/{0}.mo".format(longname), "po/{0}.pot".format(longname))):
                to_add.append(name)
            else:
                logging.info("Ignoring {0}".format(line))
        if to_rm:
            addon_obj._execute(["git", "rm"] + to_rm, check_error=True)
        if to_add:
            addon_obj._execute(["git", "add"] + to_add, check_error=True)
        addon_obj.commit("wescamp.py: pot-update")
        return True

    def extract(server, addon, path):
        """Download an addon from the server.

        server              The url of the addon server eg
                            add-ons.wesnoth.org:15005.
        addon               The name of the addon.
        path                Directory to unpack the campaign in.
        returns             Nothing.
        """

        logging.debug("extract addon server = '%s' addon = '%s' path = '%s'",
            server, addon, path)

        wml = libwml.CampaignClient(server, quiet_libwml)
        data = wml.get_campaign(addon)
        wml.unpackdir(data, path)

    def list_addons(server, translatable_only):
        """Get a list of addons on the server.

        server              The url of the addon server eg
                            add-ons.wesnoth.org:15005.
        translatable_only   If True only returns translatable addons.
        returns             A dictionary with the addon as key and the translatable
                            status as value.
        """

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

    def get_timestamp(server, addon):
        """Get the timestamp of a campaign on the server.

        server              The url of the addon server eg
                            add-ons.wesnoth.org:15005.
        addon               The name of the addon.
        returns             The timestamp of the campaign, -1 if not on the server.
        """

        logging.debug("get_timestamp server = '%s' addon = %s",
            server, addon)

        wml = libwml.CampaignClient(server, quiet_libwml)
        data = wml.list_campaigns(addon)

        # Item [0] hardcoded seems to work
        campaigns = data.data[0]
        result = {}
        for c in campaigns.get_all("campaign"):
            if(c.get_text_val("name") != addon):
                continue

            return int(c.get_text_val("timestamp"))

        return -1

    def upload(server, addon, temp_dir, wescamp_dir, build_sys_dir=None):
        """Upload a addon from the server to wescamp.

        server              The url of the addon server eg
                            add-ons.wesnoth.org:15005.
        addon               The name of the addon.
        temp_dir            The directory where the unpacked campaign can be stored.
        wescamp_dir         The directory containing a checkout of wescamp.
        build_sys_dir       Possible directory containing a checkout of build-system.
        """

        logging.debug("upload addon to wescamp server = '%s' addon = '%s' "
            + "temp_dir = '%s' wescamp_dir = '%s'",
            server, addon, temp_dir, wescamp_dir)

        # Is the addon in the list with campaigns to be translated.
        campaigns = list_addons(server, True)
        if((addon in campaigns) == False):
            logging.info("Addon '%s' is not marked as translatable "
                + "upload aborted.", addon)
            return

        github = libgithub.GitHub(wescamp_dir, git_version, authorization=git_auth)

        has_updated = False

        # If the checkout doesn't exist we need to create it.
        if not os.path.isdir(os.path.join(wescamp_dir, addon)):

            logging.info("Checking out '%s'.",
                os.path.join(wescamp_dir, addon))

            if not github.addon_exists(addon):
                github.create_addon(addon)

        # Update the directory
        addon_obj = github.addon(addon)
        addon_obj.update()

        has_updated = update_addon(addon_obj, addon, server, temp_dir)

        has_build_system = update_build_system(addon_obj, addon, wescamp_dir, build_sys_dir)

        if has_updated and has_build_system:
            pot_update(addon_obj, addon)


    def checkout(wescamp, auth=None, readonly=False):
        """Checkout all add-ons of one wesnoth version from wescamp.

        wescamp             The directory where all checkouts should be stored.
        auth                Authentication data. Shouldn't be needed.
        readonly            Makes a read-only checkout that doesn't require authentication.
        """

        logging.debug("checking out add-ons from wesnoth version = '%s' to directory '%s'", git_version, wescamp)

        github = libgithub.GitHub(wescamp, git_version, authorization=git_auth)

        for addon in github.list_addons():
            addon_obj = github.addon(addon, readonly=readonly)
            addon_obj.update()

    def assert_campaignd(configured):
        if not configured:
            logging.error("No branch or port specified. Unable to determine which addon server to use.")
            sys.exit(2)
    def assert_wescamp(configured):
        if not configured:
            logging.error("No branch or wescamp checkout specified. Unable to determine which version branch to use.")
            sys.exit(2)


    argumentparser = argparse.ArgumentParser("%(prog)s [options]")

    argumentparser.add_argument("-l", "--list", action = "store_true",
        help = "List available addons. Usage [SERVER [PORT] [VERBOSE]")

    argumentparser.add_argument("-L", "--list-translatable", action = "store_true",
        help = "List addons available for translation. "
        + "Usage [SERVER [PORT] [VERBOSE]")

    argumentparser.add_argument("-u", "--upload",
        help = "Upload a addon to wescamp. Usage: 'addon' WESCAMP-CHECKOUT "
        + "[SERVER [PORT]] [TEMP-DIR] [VERBOSE]")

    argumentparser.add_argument("-U", "--upload-all", action = "store_true",
        help = "Upload all addons to wescamp. Usage WESCAMP-CHECKOUT "
        + " [SERVER [PORT]] [VERBOSE]")

    argumentparser.add_argument("-s", "--server",
        help = "Server to connect to [localhost]")

    argumentparser.add_argument("-p", "--port",
        help = "Port on the server to connect to. If omitted will try to select a port based on --branch. ['']")

    argumentparser.add_argument("-t", "--temp-dir", help = "Directory to store the "
        + "tempory data, if omitted a tempdir is created and destroyed after "
        + "usage, if specified the data is left in the tempdir. ['']")

    argumentparser.add_argument("-w", "--wescamp-checkout",
        help = "The directory containing the wescamp checkout root. ['']")

    argumentparser.add_argument("-v", "--verbose", action = "store_const", const="verbose", dest="verbosity",
        help = "Show more verbose output. [FALSE]")

    argumentparser.add_argument("-q", "--quiet", action = "store_const", const="quiet", dest="verbosity",
        help = "Show less verbose output. [FALSE]")

    argumentparser.add_argument("-P", "--password",
        help = "The master password for the addon server. ['']")

    argumentparser.add_argument("-G", "--github-auth",
        help = "Username and password for github in the user:pass format, or an OAuth2 token.")

    argumentparser.add_argument("-B", "--branch",
        help = "WesCamp version branch to use. If omitted, we try to determine this from the wescamp directory.")

    argumentparser.add_argument("-c", "--checkout", action = "store_true",
        help = "Create a new branch checkout directory. "
        + "Can also be used to update existing checkout directories.")

    argumentparser.add_argument("-C", "--checkout-readonly", action = "store_true",
        help = "Create a read-only branch checkout directory. "
        + "Can also be used to update existing checkout directories.")

    argumentparser.add_argument("-b", "--build-system",
        help = "Path to a github.com/wescamp/build-system checkout.")

    argumentparser.add_argument("-e", "--error-log",
        help = "File to append errors and warnings to.")

    args = argumentparser.parse_args()

    campaignd_configured = False
    wescamp_configured = False

    if(args.verbosity == "verbose"):
        logging.basicConfig(level=logging.DEBUG,
            format='[%(levelname)s] %(message)s',
            stream=sys.stdout)
        quiet_libwml = False
    elif(args.verbosity == "quiet"):
        logging.basicConfig(level=logging.WARN,
            format='[%(levelname)s] %(message)s',
            stream=sys.stdout)
    else:
        logging.basicConfig(level=logging.INFO,
            format='[%(levelname)s] %(message)s',
            stream=sys.stdout)

    if args.error_log:
        import time
        formatter = logging.Formatter(fmt="[%(levelname)s %(asctime)s]\n%(message)s")
        formatter.converter = time.gmtime
        handler = logging.FileHandler(args.error_log)
        handler.setLevel(logging.WARN)
        handler.setFormatter(formatter)
        record = logging.LogRecord(
            name = None,
            level = logging.INFO,
            pathname = '',
            lineno = 0,
            msg = "{0}\nwescamp.py run start\n".format("="*80),
            args = [],
            exc_info = None,
            )
        handler.emit(record)
        logging.getLogger().addHandler(handler)

    server = "localhost"
    if(args.server != None):
        server = args.server

    if args.port != None:
        server += ":" + args.port
        campaignd_configured = True
    elif args.branch != None:
        for port, version in libwml.CampaignClient.portmap:
            if version.startswith(args.branch):
                server += ":" + port
                campaignd_configured = True
                break

    target = None
    tmp = tempdir()
    if(args.temp_dir != None):
        if(args.upload_all):
            logging.error("TEMP-DIR not allowed for UPLOAD-ALL.")
            sys.exit(2)

        target = args.temp_dir
    else:
        target = tmp.path

    wescamp = None
    if(args.wescamp_checkout):
        wescamp = args.wescamp_checkout

    password = args.password
    build_sys_dir = args.build_system

    git_auth = args.github_auth

    if args.branch:
        git_version = args.branch
        wescamp_configured = True
    elif wescamp:
        try:
            git_version = wescamp.split("-")[-1].strip("/").split("/")[-1]
            wescamp_configured = True
        except:
            # FIXME: this never happens
            logging.error("No branch specified and wescamp directory path does not end in a version suffix. Unable to determine which version branch to use.")
            sys.exit(2)

    # List the addons on the server and optional filter on translatable
    # addons.
    if(args.list or args.list_translatable):
        assert_campaignd(campaignd_configured)
        try:
            addons = list_addons(server, args.list_translatable)
        except libgithub.AddonError as e:
            print("[ERROR github in {0}] {1}".format(e.addon, str(e.message)))
            sys.exit(1)
        except libgithub.Error as e:
            print("[ERROR github] " + str(e))
            sys.exit(1)
        except socket.error as e:
            print("Socket error: " + str(e))
            sys.exit(e[0])
        except IOError as e:
            print("Unexpected error occured: " + str(e))
            sys.exit(e[0])

        for k, v in list(addons.items()):
            if(v):
                print(k + " translatable")
            else:
                print(k)

    # Upload an addon to wescamp.
    elif(args.upload != None):
        assert_campaignd(campaignd_configured)
        assert_wescamp(wescamp_configured)
        if(wescamp == None):
            logging.error("No wescamp checkout specified")
            sys.exit(2)

        try:
            upload(server, args.upload, target, wescamp, build_sys_dir)
        except libgithub.AddonError as e:
            print("[ERROR github in {0}] {1}".format(e.addon, str(e.message)))
            sys.exit(1)
        except libgithub.Error as e:
            print("[ERROR github] " + str(e))
            sys.exit(1)
        except socket.error as e:
            print("Socket error: " + str(e))
            sys.exit(e[0])
        except IOError as e:
            print("Unexpected error occured: " + str(e))
            sys.exit(e[0])

    # Upload all addons from wescamp.
    elif(args.upload_all):
        assert_campaignd(campaignd_configured)
        assert_wescamp(wescamp_configured)
        if(wescamp == None):
            logging.error("No wescamp checkout specified.")
            sys.exit(2)

        error = False
        try:
            addons = list_addons(server, True)
        except socket.error as e:
            print("Socket error: " + str(e))
            sys.exit(e[0])
        for k, v in list(addons.items()):
            try:
                logging.info("Processing addon '%s'", k)
                # Create a new temp dir for every upload.
                tmp = tempdir()
                upload(server, k, tmp.path, wescamp, build_sys_dir)
            except libgithub.AddonError as e:
                print("[ERROR github in {0}] {1}".format(e.addon, str(e.message)))
                error = True
            except libgithub.Error as e:
                print("[ERROR github] in addon '{0}' {1}".format(k, str(e)))
                error = True
            except socket.error as e:
                print("Socket error: " + str(e))
                error = True
            except IOError as e:
                print("Unexpected error occured: " + str(e))
                error = True

        if(error):
            sys.exit(1)

    elif(args.checkout or args.checkout_readonly):
        assert_wescamp(wescamp_configured)

        if(wescamp == None):
            logging.error("No wescamp checkout specified.")
            sys.exit(2)

        try:
            checkout(wescamp, auth=git_auth, readonly=(args.checkout_readonly))
        except libgithub.AddonError as e:
            print("[ERROR github in {0}] {1}".format(e.addon, str(e.message)))
            sys.exit(1)
        except libgithub.Error as e:
            print("[ERROR github] " + str(e))
            sys.exit(1)
        except socket.error as e:
            print("Socket error: " + str(e))
            sys.exit(e[0])
        except IOError as e:
            print("Unexpected error occured: " + str(e))
            sys.exit(e[0])

    else:
        argumentparser.print_help()
