#!/usr/bin/env python
# encoding: utf8

import sys, os.path, re, time, glob
# in case the wesnoth python package has not been installed
sys.path.append("data/tools")
import wesnoth.wmldata as wmldata
import wesnoth.wmlparser as wmlparser
from wesnoth.campaignserver_client import CampaignClient

if __name__ == "__main__":
    import optparse, subprocess
    try: import psyco
    except ImportError: pass
    else: psyco.full()

    optionparser = optparse.OptionParser()
    optionparser.add_option("-a", "--address", help = "specify server address",
        default = "campaigns.wesnoth.org")
    optionparser.add_option("-p", "--port",
        help = "specify server port or BfW version (%s)" % " or ".join(
        map(lambda x: x[1], CampaignClient.portmap)),
        default = CampaignClient.portmap[0][0])
    optionparser.add_option("-l", "--list", help = "list available campaigns",
        action = "store_true",)
    optionparser.add_option("-w", "--wml",
        help = "when listing campaigns, list the raw wml",
        action = "store_true",)
    optionparser.add_option("-C", "--color",
        help = "use colored WML output",
        action = "store_true",)
    optionparser.add_option("-c", "--campaigns-dir",
        help = "directory where campaigns are stored",
        default = ".")
    optionparser.add_option("-P", "--password",
        help = "password to use")
    optionparser.add_option("-d", "--download",
        help = "download the named campaign; " +
        "name may be a Python regexp matched against all campaign names " +
        "(specify the path where to put it with -c, " +
        "current directory will be used by default)")
    optionparser.add_option("-u", "--upload",
        help = "Upload campaign. " +
        "UPLOAD should be either the name of a campaign subdirectory," +
        "(in which case the client looks for _server.pbl beneath it) " +
        "or a path to the .pbl file (in which case the name of the " +
        "campaign subdirectory is the basename of the path with .pbl removed)")
    optionparser.add_option("-s", "--status",
        help = "Display the status of addons installed in the given " +
        "directory.")
    optionparser.add_option("-f", "--update",
        help = "Update all installed campaigns in the given directory. " +
        "This works by comparing the info.cfg file in each addon directory " +
        "with the version on the server.")
    optionparser.add_option("-v", "--validate",
        help = "validate python scripts in a campaign " +
        "(VALIDATE specifies the name of the campaign, " +
        "set the password with -P)")
    optionparser.add_option("-V", "--verbose",
        help = "be even more verbose for everything",
        action = "store_true",)
    optionparser.add_option("-r", "--remove",
        help = "remove the named campaign from the server, " +
        "set the password -P")
    optionparser.add_option("-R", "--raw-download",
        action = "store_true",
        help = "download as a binary WML packet")
    optionparser.add_option("-U", "--unpack",
        help = "unpack the file UNPACK as a binary WML packet " +
        "(specify the campaign path with -c)")
    optionparser.add_option("--change-passphrase", nargs = 3,
        metavar = "CAMPAIGN OLD NEW",
        help = "Change the passphrase for CAMPAIGN from OLD to NEW")
    options, args = optionparser.parse_args()

    port = options.port
    if "." in options.port:
        for (portnum, version) in CampaignClient.portmap:
            if options.port == version:
                port = portnum
                break
        else:
            sys.stderr.write("Unknown BfW version %s\n" % options.port)
            sys.exit(1)

    address = options.address
    if not ":" in address:
        address += ":" + str(port)

    def get(name, version, uploads, cdir):
        mythread = cs.get_campaign_async(name, options.raw_download)

        while not mythread.event.isSet():
            mythread.event.wait(1)
            print "%s: %d/%d" % (name, cs.counter, cs.length)

        if options.raw_download:
            file(name, "w").write(mythread.data)
        else:
            print "Unpacking %s..." % name
            cs.unpackdir(mythread.data, cdir,  verbose = options.verbose)
            d = os.path.join(cdir, name)
            info = os.path.join(d, "info.cfg")
            try:
                f = file(info, "w")
                f.write("[info]\nversion=%s\nuploads=%s\n[/info]\n" %
                    (version, uploads))
                f.close()
            except OSError:
                pass
            for message in mythread.data.find_all("message", "error"):
                print message.get_text_val("message")

    def get_info(name):
        """
        Get info for a locally installed campaign. It expects a direct path
        to the info.cfg file.
        """
        if not os.path.exists(name): return None, None
        p = wmlparser.Parser(None)
        p.parse_file(name)
        info = wmldata.DataSub("WML")
        p.parse_top(info)
        uploads = info.get_or_create_sub("info").get_text_val("uploads", "")
        version = info.get_or_create_sub("info").get_text_val("version", "")
        return uploads, version

    if options.list:
        cs = CampaignClient(address)
        data = cs.list_campaigns()
        if data:
            campaigns = data.get_or_create_sub("campaigns")
            if options.wml:
                for campaign in campaigns.get_all("campaign"):
                    campaign.debug(show_contents = True,
                        use_color = options.color)
            else:
                column_sizes = [5, 7, 8, 8, 10, 5, 10, 13]
                columns = [["name", "author", "version", "uploads", "downloads", "size", "timestamp", "translatable"]]
                for campaign in campaigns.get_all("campaign"):  
                    column = [campaign.get_text_val("name", "?"),
                        campaign.get_text_val("author", "?"),
                        campaign.get_text_val("version", "?"),
                        campaign.get_text_val("uploads", "?"),
                        campaign.get_text_val("downloads", "?"),
                        campaign.get_text_val("size", "?"),
                        time.ctime(int(campaign.get_text_val("timestamp", "?"))),
                        campaign.get_text_val("translate", "false")]
                    columns.append(column)
                    for i, s in enumerate(column_sizes):
                        if 1 + len(column[i]) > s:
                            column_sizes[i] = 1 + len(column[i])
                for c in columns:
                    for i, f in enumerate(c):
                        sys.stdout.write(f.ljust(column_sizes[i]))
                    sys.stdout.write("\n")
            for message in data.find_all("message", "error"):
                print message.get_text_val("message")
        else:
            sys.stderr.write("Could not connect.\n")
    elif options.download:
        cs = CampaignClient(address)
        fetchlist = []
        data = cs.list_campaigns()
        if data:
            campaigns = data.get_or_create_sub("campaigns")
            for campaign in campaigns.get_all("campaign"):
                name = campaign.get_text_val("name", "?")
                version = campaign.get_text_val("version", "")
                uploads = campaign.get_text_val("uploads", "")
                if re.escape(options.download).replace("\\_", "_") == options.download:
                    if name == options.download:
                        fetchlist.append((name, version, uploads))
                else:
                    if re.search(options.download, name):
                        fetchlist.append((name, version, uploads))
        for name, version, uploads in fetchlist:
            info = os.path.join(options.campaigns_dir, name, "info.cfg")
            local_uploads, local_version = get_info(info)
            if uploads != local_uploads:
                # The uploads > local_uploads likely means a server reset
                if version != local_version or uploads > local_uploads:
                    get(name, version, uploads, options.campaigns_dir)
                else:
                    print "Not downloading", name,\
                        "as the version already is", local_version,\
                        "(The add-on got re-uploaded.)"
            else:
                print "Not downloading", name,\
                    "because it is already up-to-date."
    elif options.remove:
        cs = CampaignClient(address)
        data = cs.delete_campaign(options.remove, options.password)
        for message in data.find_all("message", "error"):
            print message.get_text_val("message")
    elif options.change_passphrase:
        cs = CampaignClient(address)
        data = cs.change_passphrase(*options.change_passphrase)
        for message in data.find_all("message", "error"):
            print message.get_text_val("message")
    elif options.upload:
        cs = CampaignClient(address)
        if os.path.isdir(os.path.join(options.campaigns_dir, options.upload)):	# New style with _server.pbl
            pblfile = os.path.join(options.campaigns_dir, options.upload, "_server.pbl")
            name = os.path.basename(options.upload)
            wmldir = os.path.join(options.campaigns_dir, name) 
        else:				# Old style with external .pbl file
            pblfile = options.upload
            name = os.path.basename(options.upload)
            name = os.path.splitext(name)[0]
            wmldir = os.path.join(os.path.dirname(options.upload), name)
        pbl = wmldata.read_file(pblfile, "PBL")
        mythread = cs.put_campaign_async(
            pbl.get_text_val("title"),
            name,
            pbl.get_text_val("author"),
            pbl.get_text_val("passphrase"),
            pbl.get_text_val("description"),
            pbl.get_text_val("version"),
            pbl.get_text_val("icon"),
            options.upload.replace(".pbl", ".cfg"),
            wmldir)
        while not mythread.event.isSet():
            mythread.event.wait(1)
            print "%d/%d" % (cs.counter, cs.length)
        for message in mythread.data.find_all("message", "error"):
            print message.get_text_val("message")
    elif options.update or options.status:
        if options.status:
            cdir = options.status
        else:
            cdir = options.update
        dirs = glob.glob(os.path.join(cdir, "*"))
        dirs = [x for x in dirs if os.path.isdir(x)]
        cs = CampaignClient(address)
        data = cs.list_campaigns()
        if not data:
            sys.stderr.write("Could not connect to campaign server.\n")
            sys.exit(-1)
        campaigns = {}
        for c in data.get_or_create_sub("campaigns").get_all("campaign"):
            name = c.get_text_val("name")
            campaigns[name] = c
        for dir in dirs:
            dirname = os.path.basename(dir)
            if dirname in campaigns:
                info = os.path.join(dir, "info.cfg")
                sversion = campaigns[dirname].get_text_val("version", "")
                srev = campaigns[dirname].get_text_val("uploads", "")
                if os.path.exists(info):
                    lrev, lversion = get_info(info)
                    if not srev:
                        sys.stdout.write(" ? " + dirname + " - has no " +
                            "version info on the server.\n")
                    elif srev == lrev:
                        sys.stdout.write("   " + dirname + 
                            " - is up to date.\n")
                    elif sversion == lversion:
                        sys.stdout.write(" # " + dirname + " - is version " +
                            sversion + (" but you have revision %s not %s." +
			    " (The add-on got re-uploaded.)\n") %
			    (lrev, srev))
                        if srev > lrev: # server reset?
                            if options.update:
				get(dirname, sversion, srev, cdir)
                    else:
                        sys.stdout.write(" * " + dirname + " - you have " +
                            "revision " + lrev + " but revision " + srev +
                            " is available.\n")
                        if options.update: get(dirname, sversion, srev, cdir)
                else:
                    sys.stdout.write(" ? " + dirname + 
                        " - is installed but has no " +
                        "version info.\n")
                    if options.update: get(dirname, sversion, srev, cdir)
            else:
                sys.stdout.write(" - %s - is installed but not on server.\n" %
                    dirname)
    else:
        optionparser.print_help()

