# encoding: utf-8

import html
import glob
import os
import sys
import time
import urllib.parse
from subprocess import Popen

#
# HTML template bits
#

# HTML assets that need to be copied to the destination dir.
HTML_RESOURCES = (
    "style.css", "jquery.js", "tablesorter.js",
    "asc.gif", "bg.gif", "desc.gif" # Used by style.css:
)

WESMERE_CSS_VERSION = "1.1.1"
WESMERE_CSS_PREFIX = "https://www.wesnoth.org"

WESMERE_HEADER = '''\
<!DOCTYPE html>

<html class="no-js addonsweb" lang="en">
<head>
    <meta charset="utf-8" />
    <meta name="viewport" content="width=device-width,initial-scale=1" />

    <link rel="stylesheet" href="https://fonts.googleapis.com/css?family=Montaga%%7COpen+Sans:400,400i,700,700i" type="text/css" />
    <link rel="icon" type="image/png" href="%(css_prefix)s/wesmere/img/favicon-32.png" sizes="32x32" />
    <link rel="icon" type="image/png" href="%(css_prefix)s/wesmere/img/favicon-16.png" sizes="16x16" />
    <link rel="stylesheet" type="text/css" href="%(css_prefix)s/wesmere/css/wesmere-%(css_version)s.css" />
    <link rel="stylesheet" type="text/css" href="style.css" />

    <title>Wesnoth %(server_name)s Add-ons List - The Battle for Wesnoth</title>

    <script src="%(css_prefix)s/wesmere/js/modernizr.js"></script>
    <script src="jquery.js"></script>
    <script src="tablesorter.js"></script>
    <script>
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
            <input id="searchbox" type="search" name="search" placeholder="Search" title="Search the wiki [Alt+Shift+f]" accesskey="f" />
            <span id="searchbox-controls">
                <button id="search-go" class="search-button" type="submit" title="Search">
                    <i class="search-icon" aria-hidden="true"></i>
                    <span class="sr-label">Search the wiki</span>
                </button>
            </span>
        </form>
    </div>

    <div class="reset"></div>
</div>
</div>

<div id="content">
    <h1>Wesnoth %(server_name)s Add-ons List</h1>

    <p>To install add-ons using the in-game client, choose “Add-ons” from the main menu, and click “Connect” to connect to the add-ons server. Pick the add-on you want to install from the list and click “OK” — the download will commence immediately and the add-on will be automatically installed once finished. Bear in mind that not all add-ons are singleplayer campaigns!</p>
'''

WESMERE_DOWNLOAD_HELP = '''\
    <p><strong>If</strong> you really need or would prefer to download add-ons from this web page instead of using the built-in client, use a compatible program to uncompress the full contents of the <code class="noframe">tar.bz2</code> file — including the subfolder named after the add-on — to the <code class="noframe">data/add-ons/</code> folder in your game’s <a href="https://wiki.wesnoth.org/EditingWesnoth#The_user_data_directory">user data folder</a>. The add-on will be recognized next time you launch Wesnoth or press F5 on the main menu.</p>

    <p><b>Tip:</b> Hover over the type field to see an explanation of the add-on type and over an icon to see a description for the add-on.</p>
'''

WESMERE_FOOTER = '''\
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
'''

ADDON_TYPES_INFO = {
    "scenario": {
        "short": "Scenario",
        "long": "Singleplayer scenario",
        "help": "After install the scenario will show up in the list you get when choosing “Campaign” in the main menu. (Basically it is just a campaign with only one scenario.)",
    },
    "campaign": {
        "short": "Campaign",
        "long": "Singleplayer campaign",
        "help": "After install the campaign will show up in the list you get when choosing “Campaign” in the main menu.",
    },
    "campaign_sp_mp": {
        "short": "SP/MP Campaign",
        "long": "Single/multiplayer campaign",
        "help": "After install the campaign will show up both in the list you get when choosing “Campaign” in the main menu, and in the map list in the multiplayer “Create Game” dialog.",
    },
    "campaign_mp": {
        "short": "MP Campaign",
        "long": "Multiplayer campaign",
        "help": "After install the first scenario of the campaign will be available in the map list in the multiplayer “Create Game” dialog.",
    },
    "scenario_mp": {
        "short": "MP Scenario",
        "long": "Multiplayer scenario",
        "help": "After install the scenario will be available in the map list in the multiplayer “Create Game” dialog.",
    },
    "map_pack": {
        "short": "MP Map Pack",
        "long": "Multiplayer map pack",
        "help": "After install the maps/scenarios will be available in the map list in the multiplayer “Create Game” dialog.",
    },
    "era": {
        "short": "MP Era",
        "long": "Multiplayer era",
        "help": "After install the included era(s) will be available in the multiplayer “Create Game” dialog.",
    },
    "faction": {
        "short": "MP Faction",
        "long": "Multiplayer faction",
        "help": "Usually comes with an era or is a dependency of another add-on.",
    },
    "mod_mp": {
        "short": "MP Modification",
        "long": "Multiplayer modification",
        "help": "After install the included MP gameplay modification(s) will be available in the multiplayer “Create Game” dialog.",
    },
    "media": {
        "short": "Resources",
        "long": "Miscellaneous content/media",
        "help": "Unit packs, terrain packs, music packs, etc. Usually a (perhaps optional) dependency of another add-on.",
    },
}


def htmlescape(text, quote=True):
    """Escape any HTML special characters in the given string."""
    if text is None:
        return text
    return html.escape(text, quote)

def urlencode(text):
    """
    Encode the given string to ensure it only contains valid URL characters
    (also known as percent-encoding).
    """
    if text is None:
        return text
    return urllib.parse.quote(text, encoding='utf-8')

def output(path, url, data):
    """Write the HTML index of add-ons into the specified directory."""
    try:
        os.mkdir(path)
    except OSError:
        pass

    outfile = open(path + "/index.html", "w")

    def w(line):
        outfile.write(line + "\n")

    am_dir = os.path.dirname(__file__) + "/"
    root_dir = am_dir + "../../../"
    images_to_tc = []

    # Copy required HTML assets into the destination dir.
    for filename in HTML_RESOURCES:
        Popen(["cp", "-u", am_dir + filename, path])

    server_name = os.path.basename(path)
    if server_name == "1.9":
        # 1.9 became the 1.10 add-ons server. Reflect that here.
        server_name = "1.10"
    elif server_name == "trunk":
        server_name = "Testing (Trunk)"

    w(WESMERE_HEADER % {
        "css_version": WESMERE_CSS_VERSION,
        "css_prefix": WESMERE_CSS_PREFIX,
        "server_name": server_name,
    })
    if url:
        w(WESMERE_DOWNLOAD_HELP)

    w('<table class="tablesorter" id="campaigns">\n<thead>\n<tr>')
    table_headers = [
        ("type", "Type"),
        ("icon", "Icon"),
        ("name", "Addon"),
        ("size", "Size"),
        ("stats", "Traffic"),
        ("date", "Date"),
        ("locales", "Translations")
    ]
    for header_class, header_label in table_headers:
        w('<th class="addon-%s">%s&nbsp;&nbsp;&nbsp;</th>' % (header_class, header_label))
    w('</tr>\n</thead>\n<tbody>')

    addons = data.get_all(tag="campaigns")[0]
    for addon in addons.get_all(tag="campaign"):
        v = addon.get_text_val

        addon_id = v("name") # Escaped as part of a path composition later on.
        title = htmlescape(v("title", "unknown"))
        size = float(v("size", "0")) # bytes
        display_size = size / (1024 * 1024) # MiB
        addon_type = htmlescape(v("type", "none"))
        version = htmlescape(v("version", "unknown"))
        author = htmlescape(v("author", "unknown"))

        icon = htmlescape(v("icon", ""))
        description = htmlescape(v('description', '(no description)'))
        imgurl = ""

        downloads = int(v("downloads", "0"))
        uploads = int(v("uploads", "0"))
        timestamp = int(v("timestamp", "0"))
        display_ts = time.strftime("%b %d %Y", time.localtime(timestamp))

        translations = addon.get_all(tag="translation")
        languages = [x.get_text_val("language") for x in translations]

        if icon:
            icon = icon.strip()
            tilde = icon.find("~")
            if tilde >= 0:
                icon = icon[:tilde]
            if "\\" in icon:
                icon = icon.replace("\\", "/")
            try:
                os.mkdir(path + "/icons")
            except OSError:
                pass
            if "." not in icon:
                icon += ".png"
            src = root_dir + icon
            imgurl = "icons/" + os.path.basename(icon)
            if not os.path.exists(src):
                src = root_dir + "data/core/images/" + icon
            if not os.path.exists(src):
                src = root_dir + "images/" + icon
            if not os.path.exists(src):
                src = glob.glob(root_dir + "data/campaigns/*/images/" + icon)
                if src:
                    src = src[0]
                if not src or not os.path.exists(src):
                    sys.stderr.write("Cannot find icon " + icon + "\n")
                    src = root_dir + "images/misc/missing-image.png"
                    imgurl = "icons/missing-image.png"
            images_to_tc.append((src, path + "/" + imgurl))

        w('<tr>')

        w('<td class="addon-type">')
        if addon_type in ADDON_TYPES_INFO:
            w('%(short)s<div class="type-tooltip"><b>%(long)s</b><br/>%(help)s</div>' \
                % ADDON_TYPES_INFO[addon_type])
        else:
            w(addon_type)
        w('</td>')

        w(('<td class="addon-icon"><img alt="" src="%s"/>'
           '<div class="desc-tooltip"><b>%s</b><pre>%s</pre></div></td>') % (
               imgurl, title, description))

        w('<td class="addon"><span hidden>%s</span>' % title)
        if url:
            link = htmlescape(url.rstrip("/") + "/" + urlencode(addon_id) + ".tar.bz2")
            w(('<a class="addon-download" href="%s" title="Download">'
               '<i class="fa fa-fw fa-2x fa-download" aria-hidden="true"></i>'
               '<span class="sr-only">Download</span></a>') % link)
        w(('<b>%s</b><br/>'
           '<span class="addon-meta"><span class="addon-meta-label">Version:</span> %s<br/>'
           '<span class="addon-meta-label">Author:</span> %s</span></td>') % (
               title, version, author))

        w("<td><span hidden>%d</span><b>%.2f</b>&nbsp;MiB</td>" % (size, display_size))

        w("<td><b>%d</b> down<br/>%s up</td>" % (downloads, uploads))

        w('<td><span hidden>%d</span>%s</td>' % (timestamp, display_ts))

        w("<td>%s</td>" % htmlescape(", ".join(languages), quote=False))

        w("</tr>")

    w('</tbody>\n</table>')
    w(WESMERE_FOOTER)

    sys.stderr.write("Done outputting html, now generating %d TC'ed images\n" % len(images_to_tc))
    for pair in images_to_tc:
        # wait() to ensure only one process exists at any time
        Popen([os.path.join(am_dir, "../unit_tree/TeamColorizer"), pair[0], pair[1]]).wait()

# kate: indent-mode normal; encoding utf-8; space-indent on;
