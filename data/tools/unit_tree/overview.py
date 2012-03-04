#!/usr/bin/env python
import glob, os, sys, time
sys.path.append(os.path.join(os.path.dirname(__file__), ".."))
import html_output

def main(folder):
    out = open(os.path.join(folder, "overview.html"), "wb")
    def w(x): out.write(x.encode("utf8") + "\n")
    
    path = ""
    title = "Wesnoth Unit Database Overview"
    generation_note = "generated on " + time.ctime()

    w(html_output.html_header % locals())
    
    w("""
    <div class="header">
    <a href="http://www.wesnoth.org">
    <img src="wesnoth-logo.jpg" alt="Wesnoth logo"/>
    </a>
    </div>
    <div class="topnav">
    <a href="index.html">Wesnoth Units database</a>
    </div>""")
    
    w('<table class="overview">')
    w("<tr><th>")
    w("Addon")
    w("</th><th>")
    w("Output Files")
    w("</th><th>")
    w("Error Log")
    w("</th></tr>")
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

        name = f[len(folder) + 1:]
        w('<tr><td>')
        w(name)
        w('</td><td>')
        w(str(n))
        w('</td><td>')
        if os.path.exists(error_log):
            total_error_logs += 1
            w('<a href="%s">error.log</a>' % error_log)
        w("</td></tr>")
        
    w("<tr><td>")
    w("Total:")
    w("</td><td>")
    w(str(total_n))
    w("</td><td>")
    w(str(total_error_logs))
    w("</td></tr>")
        
    w("</table>")
        
    w(html_output.html_footer % locals())

if __name__ == "__main__":
    main(sys.argv[1])
