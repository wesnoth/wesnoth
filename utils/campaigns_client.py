#!/usr/bin/env python
# encoding: utf8

import sys, os.path, re, time
# in case the wesnoth python package has not been installed
sys.path.append("data/tools")
import wesnoth.wmldata as wmldata
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
        help = "upload campaign " +
        "(UPLOAD specifies the path to the .pbl file)")
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

    if options.list:
        cs = CampaignClient(address)
        data = cs.list_campaigns()
        if data:
            campaigns = data.get_or_create_sub("campaigns")
            sys.stdout.write("name\tauthor\tversion\tuploads\tdownloads\tsize\tdate\n")
            for campaign in campaigns.get_all("campaign"):
                if options.wml:
                    campaign.debug(show_contents = True,
                        use_color = options.color)
                else:
                    sys.stdout.write(campaign.get_text_val("name", "?") + "\t" +
                        campaign.get_text_val("author", "?") + "\t" +
                        campaign.get_text_val("version", "?") + "\t" +
                        campaign.get_text_val("uploads", "?") + "\t" +
                        campaign.get_text_val("downloads", "?") + "\t" +
                        campaign.get_text_val("size", "?") + "\t" +
                        time.ctime(int(campaign.get_text_val("timestamp", "?"))) + "\n")
            for message in data.find_all("message", "error"):
                print message.get_text_val("message")
        else:
            sys.stderr.write("Could not connect.\n")
    elif options.download:
        cs = CampaignClient(address)
        if re.escape(options.download).replace("\\_", "_") == options.download:
            fetchlist = [options.download]
        else:
            fetchlist = []
            data = cs.list_campaigns()
            if data:
                campaigns = data.get_or_create_sub("campaigns")
                for campaign in campaigns.get_all("campaign"):
                    name = campaign.get_text_val("name", "?")
                    if re.search(options.download, name):
                        fetchlist.append(name)
        for name in fetchlist:
            mythread = cs.get_campaign_async(name, options.raw_download)

            while not mythread.event.isSet():
                mythread.event.wait(1)
                print "%s: %d/%d" % (name, cs.counter, cs.length)

            if options.raw_download:
                file(name, "w").write(mythread.data)
            else:
                print "Unpacking %s..." % name
                cs.unpackdir(mythread.data, options.campaigns_dir,
                    verbose = options.verbose)
                for message in mythread.data.find_all("message", "error"):
                    print message.get_text_val("message")
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
        pbl = wmldata.read_file(options.upload, "PBL")
        name = os.path.basename(options.upload)
        name = os.path.splitext(name)[0]
        mythread = cs.put_campaign_async(
            pbl.get_text_val("title"),
            name,
            pbl.get_text_val("author"),
            pbl.get_text_val("passphrase"),
            pbl.get_text_val("description"),
            pbl.get_text_val("version"),
            pbl.get_text_val("icon"),
            options.upload.replace(".pbl", ".cfg"),
            os.path.join(os.path.dirname(options.upload), name)
            )
        while not mythread.event.isSet():
            mythread.event.wait(1)
            print "%d/%d" % (cs.counter, cs.length)
        for message in mythread.data.find_all("message", "error"):
            print message.get_text_val("message")
    else:
        optionparser.print_help()

