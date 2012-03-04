#!/usr/bin/env python
import glob, os, sys, time
sys.path.append(os.path.join(os.path.dirname(__file__), ".."))
import html_output

def write_addon_overview(folder, addon):
    out = open(os.path.join(folder, "index.html"), "wb")
    def w(x): out.write(x.encode("utf8") + "\n")
    
    name = addon["name"]

    path = "../"
    title = name + " Overview"
    generation_note = "generated on " + time.ctime()
    
    w(html_output.html_header % locals())
    
    w(html_output.top_bar % locals())
    
    w('<div class="overview">')
    
    eras = addon.get("eras", [])

    w("<h2>" + name + "</h2>")

    if eras:
        w("<h3>Eras</h3><ul>")
        for era in eras:
            epath = os.path.join("en_US", era["id"] + ".html")
            w('<li><a href="' + epath + '">' + era["name"] + '</a></li>')
        w("</ul>")
    
    campaigns = addon.get("campaigns", [])
    if campaigns:
        w("<h3>Campaigns</h3><ul>")
        for campaign in campaigns:
            cpath = os.path.join("en_US", campaign["id"] + ".html")
            w('<li><a href="' + cpath + '">' + campaign["name"] + '</a></li>')
        w("</ul>")
    
    w('</div> <!-- overview -->')
    
    w(html_output.html_footer % locals())
    

def main(folder):
    out = open(os.path.join(folder, "overview.html"), "wb")
    def w(x): out.write(x.encode("utf8") + "\n")

    path = ""
    title = "Wesnoth Unit Database Overview"
    generation_note = "generated on " + time.ctime()

    w(html_output.html_header % locals())
    
    w(html_output.top_bar % locals())
    
    w('<div class="overview">')
    
    w('<table class="overview">')
    w("<tr><th>")
    w("Addon")
    w("</th><th>")
    w("Output Files")
    w("</th><th>")
    w("Error Log")
    w("</th></tr>")
    count = 0
    total_n = 0
    total_error_logs = 0
    for f in sorted(glob.glob(os.path.join(folder, "*"))):
        if not os.path.isdir(f): continue
        if f.endswith("/pics"): continue
        
        error_log = os.path.abspath(os.path.join(f, "error.log"))
        
        try:
            n = len(os.listdir(os.path.join(f, "en_US")))
        except OSError:
            n = 0
        
        total_n += n

        name = f[len(folder):].lstrip("/")
        error_name = os.path.join(name, "error.log")
        w('<tr><td>')
        w('<a href="' + os.path.join(name, "index.html") + '">' + name + '</a>')
        w('</td><td>')
        w(str(n))
        w('</td><td>')
        if os.path.exists(error_log):
            total_error_logs += 1
            w('<a class="error" href="%s">error.log</a>' % error_name)
        w("</td></tr>")
        
        count += 1
        
    w("<tr><td>")
    w("Total (for %d addons):" % count)
    w("</td><td>")
    w(str(total_n))
    w("</td><td>")
    w(str(total_error_logs))
    w("</td></tr>")

    w("</table>")
    
    w('</div> <!-- overview -->')
        
    w(html_output.html_footer % locals())

if __name__ == "__main__":
    main(sys.argv[1])
