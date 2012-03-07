#!/usr/bin/env python
import glob, os, sys, time, re
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
        error_html = os.path.abspath(os.path.join(f, "error.html"))
        
        try:
            n = len(os.listdir(os.path.join(f, "en_US")))
        except OSError:
            n = 0
        
        total_n += n

        name = f[len(folder):].lstrip("/")
        error_name = os.path.join(name, "error.html")
        w('<tr><td>')
        w('<a href="' + os.path.join(name, "index.html") + '">' + name + '</a>')
        w('</td><td>')
        w(str(n))
        w('</td><td>')
        if os.path.exists(error_log):
            
            text = open(error_log).read()
            error_kind = "warnings"
            if "<INTERNAL ERROR>" in text:
                error_kind = "internal error"
            elif "<WML ERROR>" in text:
                error_kind = "wml error"
            elif "<PARSE ERROR>" in text:
                error_kind = "parse error"
                
            source = []
            
            def postprocess(line):
                if line == "WMLError:": return ""
                if line == "?": return ""
                if line == "Preprocessor error:": return ""
                if line.startswith("Automatically found a possible data directory"): return ""
                if line.startswith("Overriding data directory with"): return ""
                if line == "'SKIP_CORE' defined.": return ""
                if re.match("added .* defines.", line): return ""
                if line.startswith("skipped 'data/core'"): return ""
                if line.startswith("preprocessing specified resource:"): return ""

                mo = re.match(r"0 /tmp/wmlparser_.*?/(.*)\.plain", line)
                if mo:
                    source.append("/tmp/" + mo.group(1))
                    return ""

                mo = re.match(".*--preprocess-defines(.*)", line)
                if mo: return "Defines: " + mo.group(1) + "<br />"
                
                if source:
                    line = line.replace(source[-1], "WML")
                
                line = line.replace("included from WML:1", "")
                rows = line.replace("included from", "\n&nbsp;included from").splitlines()
                out = ""
                for row in rows:
                    row = row.strip()
                    out += row + "<br />"
                return out
            
            htmlerr = open(error_html, "w")
            htmlerr.write("<html><body>")
            for line in text.splitlines():
                line = line.strip()
                if line in ["<INTERNAL ERROR>", "<WML ERROR>", "<PARSE ERROR>"]:
                    htmlerr.write("<p>")
                elif line in ["</INTERNAL ERROR>", "</WML ERROR>", "</PARSE ERROR>"]:
                    htmlerr.write("</p>")
                else:
                    htmlerr.write(postprocess(line))
            htmlerr.write("</body></html>")
            
            total_error_logs += 1
            w('<a class="error" href="%s">%s</a>' % (error_name, error_kind))
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
