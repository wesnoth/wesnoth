# encoding: utf-8
import cgi, time, os, glob, sys, re
from subprocess import Popen

def htmlescape(str):
    return cgi.escape(str)

def output(path, url, data):
    try: os.mkdir(path)
    except OSError: pass

    f = open(path + "/index.html", "w")
    def w(x):
        f.write(x + "\n")
    server_name = os.path.basename(path)
    if server_name == "1.9":
        # 1.9 became the 1.10 add-ons server. Reflect that here.
        server_name = "1.10"
    elif server_name == "trunk":
        server_name = "Testing (Trunk)"
    w("""\
<!DOCTYPE html>

<html class="no-js addonsweb" lang="en">
<head>
    <meta charset="utf-8" />
    <meta name="viewport" content="width=device-width,initial-scale=1" />

    <link rel="stylesheet" href="https://fonts.googleapis.com/css?family=Montaga%7COpen+Sans:400,400i,700,700i" type="text/css" />
    <link rel="icon" type="image/png" href="https://www.wesnoth.org/wesmere/img/favicon-32.png" sizes="32x32" />
    <link rel="icon" type="image/png" href="https://www.wesnoth.org/wesmere/img/favicon-16.png" sizes="16x16" />
    <link rel="stylesheet" type="text/css" href="https://www.wesnoth.org/wesmere/css/wesmere-1.1.0.css" />
    <link rel="stylesheet" type="text/css" href="style.css" />""")
    w("<title>Wesnoth %s Add-ons List - The Battle for Wesnoth</title>" % server_name)
    w("""
    <script src="https://www.wesnoth.org/wesmere/js/modernizr.js"></script>
    <script type="text/javascript" src="jquery.js"></script>
    <script type="text/javascript" src="tablesorter.js"></script>
    <script type="text/javascript">
    $(document).ready(function() {
        $("#campaigns").tablesorter({
            headers: { 1: { sorter: false }, 2: { sortInitialOrder: "asc" } },
        });
    });
    </script>
</head>

<body>

<div id="main">

<div id="nav" role="banner">
<div class="centerbox">

    <div id="logo">
        <a href="https://www.wesnoth.org/" aria-label="Wesnoth logo"></a>
    </div>

    <ul id="navlinks">
        <li><a href="https://addons.wesnoth.org/">Add-ons</a></li>
        <li><a href="https://www.wesnoth.org/">Home</a></li>
        <li><a href="https://forums.wesnoth.org/viewforum.php?f=62">News</a></li>
        <li><a href="https://wiki.wesnoth.org/Play">Play</a></li>
        <li><a href="https://wiki.wesnoth.org/Create">Create</a></li>
        <li><a href="https://forums.wesnoth.org/">Forums</a></li>
        <li><a href="https://wiki.wesnoth.org/Project">About</a></li>
    </ul>

    <div id="sitesearch" role="search">
        <form method="get" action="https://wiki.wesnoth.org/">
            <input id="searchbox" type="search" name="search" placeholder="Search" title="Search the wiki [Alt+Shift+f]" accesskey="f" tabindex="1" />
            <span id="searchbox-controls">
                <button id="search-go" class="search-button" type="submit" title="Search" tabindex="2">
                    <i class="search-icon" aria-hidden="true"></i>
                    <span class="sr-label">Search the wiki</span>
                </button>
            </span>
        </form>
    </div>

    <div class="reset"></div>
</div>
</div>

<div id="content">""")
    w("<h1>Wesnoth %s Add-ons List</h1>" % server_name)
    w("""<p>To install add-ons using the in-game client, choose "Add-ons" from the main menu, and click "Connect" to connect to the add-ons server. Pick the add-on you want to install from the list and click "OK" — the download will commence immediately and the add-on will be automatically installed once finished. Bear in mind that not all add-ons are singleplayer campaigns!</p>""")
    if url:
        w("""
          <p><strong>If</strong> you really need or would prefer to download add-ons from this web page instead of using the built-in client, use a compatible program to uncompress the full contents of the <code class="noframe">tar.bz2</code> file — including the subfolder named after the add-on — to the <code class="noframe">data/add-ons/</code> folder in your game's <a href="https://wiki.wesnoth.org/EditingWesnoth#The_user_data_directory">user data folder</a>. The add-on will be recognized next time you launch Wesnoth or press F5 on the main menu.</p>
          <p><b>Tip:</b> Hover over the type field to see an explanation of the add-on type and over an icon to see a description for the add-on.</p>""")

    am_dir = os.path.dirname(__file__) + "/"
    for name in ["style.css", "jquery.js", "tablesorter.js",
        "asc.gif", "bg.gif", "desc.gif"]:
        Popen(["cp", "-u", am_dir + name, path])

    campaigns = data.get_all(tag = "campaigns")[0]
    w("<table class=\"tablesorter\" id=\"campaigns\">")
    w("<thead>")
    w("<tr>")
    table_headers = [
        ("type",    "Type"),
        ("icon",    "Icon"),
        ("name",    "Addon"),
        ("size",    "Size"),
        ("stats",   "Traffic"),
        ("date",    "Date"),
        ("locales", "Translations")
    ]
    for header_class, header_label in table_headers:
        w("<th class=\"addon-%s\">%s&nbsp;&nbsp;&nbsp;</th>" % (header_class, header_label))
    w("</tr>")
    w("</thead>")
    w("<tbody>")
    root_dir = am_dir + "../../../"
    images_to_tc = []
    for campaign in campaigns.get_all(tag = "campaign"):
        v = campaign.get_text_val
        translations = campaign.get_all(tag = "translation")
        languages = [x.get_text_val("language") for x in translations]
        w("<tr>")
        icon = htmlescape(v("icon", ""))
        imgurl = ""
        if icon:
            icon = icon.strip()
            tilde = icon.find("~")
            if tilde >= 0: icon = icon[:tilde]
            if "\\" in icon: icon = icon.replace("\\", "/")
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
        w('<td class="addon-type">')
        if type == "scenario":
            w("""\
Scenario<div class="type-tooltip"><b>single player scenario</b><br/>
After install the scenario will show up in the list you get when choosing "Campaign" in the main menu. (Basically it's just a campaign with only one scenario.)</div>""")
        elif type == "campaign":
            w("""\
Campaign<div class="type-tooltip"><b>single player campaign</b><br/>
After install the campaign will show up in the list you get when choosing "Campaign" in the main menu.</div>""")
        elif type == "campaign_sp_mp":
            w("""\
SP/SP Campaign<div class="type-tooltip"><b>single/multi player campaign</b><br />
After install the campaign will show up both in the list you get when choosing "Campaign" in the main menu, and in the map list in the multiplayer "Create Game" dialog.</div>""")
        elif type == "campaign_mp":
            w("""\
MP Campaign<div class="type-tooltip"><b>multiplayer campaign</b><br/>
After install the first scenario of the campaign will be available in the map list in the multiplayer "Create Game" dialog.</div>""")
        elif type == "scenario_mp":
            w("""\
MP Scenario<div class="type-tooltip"><b>multiplayer scenario</b><br/>
After install the scenario will be available in the map list in the multiplayer "Create Game" dialog.</div>""")
        elif type == "map_pack":
            w("""\
MP map-pack<div class="type-tooltip"><b>multiplayer map pack</b><br/>
After install the maps/scenarios will be available in the map list in the multiplayer "Create Game" dialog.</div>""")
        elif type == "era":
            w("""\
MP era<div class="type-tooltip"><b>multiplayer era</b><br/>
After install the included era(s) will be available in the multiplayer "Create Game" dialog.</div>""")
        elif type == "faction":
            w("""\
MP faction<div class="type-tooltip"><b>multiplayer faction</b><br/>
Usually comes with an era or is dependency of another add-on.</div>""")
        elif type == "mod_mp":
            w("""\
MP modification<div class="type-tooltip"><b>multiplayer modification</b><br />
After install the included MP gameplay modification(s) will be available in the multiplayer "Create Game" dialog.</div>""")
        elif type == "media":
            w("""\
Resources<div class="type-tooltip"><b>miscellaneous content/media</b><br/>
Unit packs, terrain packs, music packs, etc. Usually a (perhaps optional) dependency of another add-on.</div>""")
        else:
            w(type)
        w('</td>')
        w(('<td class="addon-icon"><img alt="" src="%s"/>') % imgurl)
        described = htmlescape(v("description", "(no description)"))
        w('<div class="desc-tooltip"><b>%s</b><pre>%s</pre></div></td>' % (
            name, described))
        w("<td class=\"addon\"><span hidden>%s</span>" % name)
        if url:
            link = url.rstrip("/") + "/" + htmlescape(v("name")) + ".tar.bz2"
            w("""\
            <a class="addon-download" href="%s" title="Download">
                <i class="fa fa-fw fa-2x fa-download" aria-hidden="true"></i>
                <span class="sr-only">Download</span>
            </a>""" % link)
        w("<b>%s</b>" % name)
        w("<br /><span class='addon-meta'><span class='addon-meta-label'>Version:</span> %s<br/>" % htmlescape(v("version", "unknown")))
        w("<span class='addon-meta-label'>Author:</span> %s</span></td>" % htmlescape(v("author", "unknown")))
        MiB = 1024 * 1024
        w("<td><span hidden>%d</span><b>%.2f</b>&nbsp;MiB</td>" % (size, size / MiB))
        downloads = int(v("downloads", "0"))
        w("<td><b>%d</b> down<br/>" % (downloads))
        w("%s up</td>" % htmlescape(v("uploads", "unknown")))
        timestamp = int(v("timestamp", "0"))
        t = time.localtime(timestamp)
        w("<td><span hidden>%d</span>%s</td>" % (timestamp,
            time.strftime("%b %d %Y", t)))
        w("<td>%s</td>" % (htmlescape(", ".join(languages))))
        w("</tr>")
    w("""\
        </tbody>
    </table>
</div> <!-- end content -->

</div> <!-- end main -->

<div id="footer-sep"></div>

<div id="footer"><div id="footer-content"><div>
	<a href="https://wiki.wesnoth.org/StartingPoints">Site Map</a> &#8226; <a href="http://status.wesnoth.org/">Site Status</a><br />
	Copyright &copy; 2003&ndash;2017 by <a rel="author" href="https://wiki.wesnoth.org/Project">The Battle for Wesnoth Project</a>.<br />
	Site design Copyright &copy; 2017 by Ignacio R. Morelle.
</div></div></div>
</body>
</html>
""")
    sys.stderr.write("Done outputting html, now generating %d TC'ed images\n" % len(images_to_tc))
    for pair in images_to_tc:
        Popen([os.path.join(am_dir, "../unit_tree/TeamColorizer"), pair[0], pair[1]]).wait() # wait() to ensure only one process is exists at any time

# kate: indent-mode normal; encoding utf-8; space-indent on;
