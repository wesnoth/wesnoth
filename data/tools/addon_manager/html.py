import time, os, glob, sys

def output(path, data):
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
<link rel=stylesheet href=\"style.css\" type=\"text/css\">
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
</div>""")

    os.system("cp -u data/tools/addon_manager/style.css " + path)

    campaigns = data.get_or_create_sub("campaigns")
    w("<table id=\"campaigns\">")
    w("<tr>")
    w("<th></th>")
    w("<th>Addon</th>")
    w("<th>Size</th>")
    w("<th>Traffic</th>")
    w("<th>Date</th>")
    w("<th>Notes</th>")
    w("</tr>")
    for campaign in campaigns.get_all("campaign"):
        v = campaign.get_text_val
        w("<tr>")
        icon = v("icon", "")
        if icon:
            tilde = icon.find("~")
            if tilde >= 0: icon = icon[:tilde]
            try: os.mkdir(path + "/icons")
            except OSError: pass
            if "." not in icon: icon += ".png"
            src = "./data/core/images/" + icon
            if not os.path.exists(src):
                src = glob.glob("./data/campaigns/*/images/" + icon)
                if src: src = src[0]
                if not src or not os.path.exists(src):
                    sys.stderr.write("Cannot find icon " + icon + "\n")
                    src = "./data/core/images/units/unknown-unit.png"
                    imgurl = "icons/unknown-unit.png"
                else:
                    imgurl = "icons/" + os.path.basename(icon)
            else:
                imgurl = "icons/" + os.path.basename(icon)
            os.system("cp -u " + src + " " + path + "/icons")
                
        name = v("name", "unknown").replace("_", " ")
        w(('<td><img alt="%s" src="%s" width="72px" height="72px"/>'
            ) % (icon, imgurl))
        w('<div class="desc"><b>%s</b><br/>%s</div></td>' % (
            name, v("description", "(no description)")))
        w("<td><b>%s</b><br/>" % name)
        w("Version: %s<br/>" % v("version", "unknown"))
        w("Author: %s</td>" % v("author", "unknown"))
        w("<td>%.1fMB</td>" % (float(v("size", "unknown")) / 1024 / 1024))
        w("<td><b>%s</b> down<br/>" % v("downloads", "unknown"))
        w("%s up</td>" % v("uploads", "unknown"))
        t = time.localtime(int(v("timestamp", "unknown")))
        w("<td>%s</td>" % time.strftime("%b %d %Y", t))
        w("<td>%s</td>" % v("translate", ""))
        w("</tr>")
    w("</table>")

    w("""\
<div id="footer">
<p><a href="http://www.wesnoth.org/wiki/Site_Map">Site map</a></p>
<p><a href="http://www.wesnoth.org/wiki/Wesnoth:Copyrights">Copyright</a> &copy; 2003-2008 The Battle for Wesnoth</p>
<p>Supported by <a href="http://www.jexiste.fr/">Jexiste</a></p>
<p><a href="http://www.ibiblio.org/"><img src="http://www.ibiblio.org/hosted/images/ibiblio_hosted-110x32.png" width="110" height="32" border="0" alt="hosted by ibiblio" /></a></p>
</div>
</body></html>""")