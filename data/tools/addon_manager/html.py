# encoding: utf-8
import cgi, time, os, glob, sys
from subprocess import Popen

def htmlescape(str):
    return cgi.escape(str)

def output(path, url, data):
    try: os.mkdir(path)
    except OSError: pass

    f = open(path + "/index.html", "w")
    def w(x):
        f.write(x + "\n")
    w("""\
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01//EN" "http://www.w3.org/TR/html4/strict.dtd">
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8">
<link rel=stylesheet href="style.css" type="text/css">
<script type="text/javascript" src="jquery.js"></script>
<script type="text/javascript" src="tablesorter.js"></script>
<script type="text/javascript">
$(document).ready(function() 
{ 
    $("#campaigns").tablesorter(
    {
        headers: { 1: { sorter: false} }
    }
    ); 
} 
); 
</script>
</head>
<body>""")

    w("""\
<div class="header">
<a href="http://www.wesnoth.org">
<img src="http://www.wesnoth.org/mw/skins/glamdrol/wesnoth-logo.jpg" alt="Wesnoth logo"/>
</a>
</div>
<div class="topnav">
<a href="index.html">Wesnoth Addons</a>
</div>
<div class="main">
<p>To install an add-on please go to the title screen of Battle for Wesnoth. Select "Add-ons" from the menu and click "OK" to connect to add-ons.wesnoth.org.
Select the add-on you want to install from the list and click "OK". The download will commence immediately. Wesnoth will then automatically install and load the add-on so you can use it. Remember that not all add-ons are campaigns!</p>
<p>Note: Hover over the type field to see an explanation of the type and over an icon to see the description of the add-on.</p>
""")
    if url:
        w("""<p>PS: If you really have to download an add-on from here uncompress it to the <a href="http://www.wesnoth.org/wiki/EditingWesnoth#Where_is_my_user_data_directory.3F">userdata</a>/data/add-ons/ directory for wesnoth to find it.
""")

    am_dir = os.path.dirname(__file__) + "/"
    for name in ["style.css", "jquery.js", "tablesorter.js",
        "asc.gif", "bg.gif", "desc.gif"]:
        Popen(["cp", "-u", am_dir + name, path])

    campaigns = data.get_or_create_sub("campaigns")
    w("<table class=\"tablesorter\" id=\"campaigns\">")
    w("<thead>")
    w("<tr>")
    for header in ["Type", "Icon", "Addon", "Size", "Traffic", "Date", "Notes"]:
        w("<th>%s&nbsp;&nbsp;&nbsp;</th>" % header)
    w("</tr>")
    w("</thead>")
    w("<tbody>")
    root_dir = am_dir + "../../../"
    images_to_tc = []
    for campaign in campaigns.get_all("campaign"):
        v = campaign.get_text_val
        translations = campaign.get_all("translation")
        languages = [x.get_text_val("language") for x in translations]
        w("<tr>")
        icon = htmlescape(v("icon", ""))
        imgurl = ""
        if icon:
            icon = icon.strip()
            tilde = icon.find("~")
            if tilde >= 0: icon = icon[:tilde]
            try: os.mkdir(path + "/icons")
            except OSError: pass
            if "." not in icon: icon += ".png"
            src = root_dir + icon
            imgurl = "icons/" + os.path.basename(icon)
            if not os.path.exists(src):
                src = root_dir + "data/core/images/" + icon
            if not os.path.exists(src):
                src = root_dir + "images/" + icon
            if not os.path.exists(src):
                src = glob.glob(root_dir + "data/campaigns/*/images/" + icon)
                if src: src = src[0]
                if not src or not os.path.exists(src):
                    sys.stderr.write("Cannot find icon " + icon + "\n")
                    src = root_dir + "images/misc/missing-image.png"
                    imgurl = "icons/missing-image.png"
            images_to_tc.append( (src, path + "/" + imgurl) )

        type = htmlescape(v("type", "none"))
        size = float(v("size", "0"))
        name = htmlescape(v("title", "unknown"))
        if type == "scenario":
            w("""\
<td>Scenario<div class="type"><b>single player scenario</b><br/>
After install the scenario will show up in the list you get when choosing "Campaign" in the main menu. (Basically it's just a campaign with only one scenario.)</div></td>""")
        elif type == "campaign":
            w("""\
<td>Campaign<div class="type"><b>single player campaign</b><br/>
After install the campaign will show up in the list you get when choosing "Campaign" in the main menu.</div></td>""")
        elif type == "campaign_mp":
            w("""\
<td>MP Campaign<div class="type"><b>multiplayer campaign</b><br/>
After install the first scenario of the campaign will be available in the map list in the multiplayer "Create Game" dialog.</div></td>""")
        elif type == "scenario_mp":
            w("""\
<td>MP Scenario<div class="type"><b>multiplayer scenario</b><br/>
After install the scenario will be available in the map list in the multiplayer "Create Game" dialog.</div></td>""")
        elif type == "map_pack":
            w("""\
<td>MP map-pack<div class="type"><b>multiplayer map pack</b><br/>
After install the maps/scenarios will be available in the map list in the multiplayer "Create Game" dialog.</div></td>""")
        elif type == "era":
            w("""\
<td>MP era<div class="type"><b>multiplayer era</b><br/>
After install the included era(s) will be available in the multiplayer "Create Game" dialog.</div></td>""")
        elif type == "faction":
            w("""\
<td>MP faction<div class="type"><b>multiplayer faction</b><br/>
Usually comes with an era or is dependency of another add-on.</div></td>""")
        elif type == "media":
            w("""\
<td>Resources<div class="type"><b>miscellaneous content/media</b><br/>
unit packs, terrain packs, music packs, etc. Usually a (perhaps optional) dependency of another add-on.</div></td>""")
        else: w(('<td>%s</td>') % type)
        w(('<td><img alt="%s" src="%s" width="72px" height="72px"/>'
            ) % (icon, imgurl))
        w('<div class="desc"><b>%s</b><br/>%s</div></td>' % (
            name, htmlescape(v("description", "(no description)"))))
        w("<td><b>%s</b><br/>" % name)
        w("Version: %s<br/>" % htmlescape(v("version", "unknown")))
        w("Author: %s</td>" % htmlescape(v("author", "unknown")))
        MiB = 1024 * 1024
        w("<td><span class=\"hidden\">%d</span><b>%.2f</b>MiB" % (size, size / MiB))
        if url:
            link = url.rstrip("/") + "/" + htmlescape(v("name")) + ".tar.bz2"
            w("<br/><a href=\"%s\">download</a></td>" % link)
        else:
            w("</td>")
        downloads = int(v("downloads", "0"))
        w("<td><b>%d</b> down<br/>" % (downloads))
        w("%s up</td>" % htmlescape(v("uploads", "unknown")))
        timestamp = int(v("timestamp", "0"))
        t = time.localtime(timestamp)
        w("<td><span class=\"hidden\">%d</span>%s</td>" % (timestamp,
            time.strftime("%b %d %Y", t)))
        w("<td>%s</td>" % (htmlescape(", ".join(languages))))
        w("</tr>")
    w("</tbody>")
    w("</table>")

    w("""\
</div>
<div id="footer">
<p><a href="http://www.wesnoth.org/wiki/Site_Map">Site map</a></p>
<p><a href="http://www.wesnoth.org/wiki/Wesnoth:Copyrights">Copyright</a> &copy; 2003-2013 The Battle for Wesnoth</p>
<p>Supported by <a href="http://www.jexiste.fr/">Jexiste</a></p>
</div>
</body></html>""")
    sys.stderr.write("Done outputting html, now generating %d TC'ed images\n" % len(images_to_tc))
    for pair in images_to_tc:
        Popen([os.path.join(am_dir, "../unit_tree/TeamColorizer"), pair[0], pair[1]]).wait() # wait() to ensure only one process is exists at any time
