#!/usr/bin/env python2
import glob, os, sys, time, re
sys.path.append(os.path.join(os.path.dirname(__file__), ".."))
from . import html_output

def write_addon_overview(folder, addon):
    out = open(os.path.join(folder, "index.html"), "w")
    def w(x):
        out.write(x + "\n")

    name = addon["name"]

    path = "../"
    title = html_output.cleantext("Build Report for " + name)
    generation_note = "Last updated on " + time.ctime() + "."

    w(html_output.HTML_HEADER % locals())

    w(html_output.TOP_BAR % locals())

    w('<div class="overview">')

    eras = addon.get("eras", [])

    w('<h2>' + html_output.cleantext(name) + '</h2>')

    if eras:
        w("<h3>Eras</h3><ul>")
        for era in eras:
            epath = html_output.cleanurl(os.path.join("en_US", era["id"] + ".html"))
            w('<li><a href="' + epath + '">' + html_output.cleantext(era["name"], quote=False) + '</a></li>')
        w("</ul>")

    campaigns = addon.get("campaigns", [])
    if campaigns:
        w('<h3>Campaigns</h3><ul>')
        for campaign in campaigns:
            cpath = os.path.join("en_US", campaign["id"] + ".html")
            w('<li><a href="' + cpath + '">' + html_output.cleantext(campaign["name"], quote=False) + '</a></li>')
        w("</ul>")

    w("<div>")
    if os.path.exists(os.path.join(folder, "error.log")):
        w('<p><b>Warnings or errors were found: <a href="error.html">log</a></b></p>')
    w('<p><a href="../overview.html">Back to the full report</a></p>')
    w("</div>")

    w('</div> <!-- overview -->')

    w(html_output.HTML_FOOTER % locals())


def main(folder):
    out = open(os.path.join(folder, "overview.html"), "w")
    def w(x):
        out.write(x + "\n")

    path = ""
    title = "Database Build Report"
    generation_note = "generated on " + time.ctime()
    classes = "wmlunits-report"

    w(html_output.HTML_HEADER % locals())

    w(html_output.TOP_BAR % locals())

    w('<h1>Database Build Report</h1>')

    w('<div class="overview">')
    w('<table class="overview">')
    w('<thead><tr><th>Addon</th><th>Output Files</th><th>Error Log</th></tr></thead>')
    w('<tbody>')
    count = 0
    total_n = 0
    total_error_logs = 0
    total_lines = 0
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
        w('<a href="' + html_output.cleanurl(os.path.join(name, "index.html")) + '">' + html_output.cleantext(name, quote=False) + '</a>')
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
            elif "<TIMEOUT ERROR>" in text:
                error_kind = "timeout"

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

                mo = re.match(r"\d+ /tmp(?:/wmlparser_.*?/|/)(.*\.cfg).*", line)
                if mo:
                    source.append("/tmp/" + mo.group(1))
                    return ""

                mo = re.match(".*--preprocess-defines(.*)", line)
                if mo: return "Defines: " + mo.group(1) + "<br />"

                for s in source:
                    line = line.replace(s, "WML")

                line = line.replace("included from WML:1", "")
                rows = line.replace("included from", "\n&nbsp;included from").splitlines()
                out = ""
                for row in rows:
                    row = row.strip()
                    out += row + "<br />"
                return out

            htmlerr = open(error_html, "w")
            htmlerr.write("<html><body>")
            lines_count = 0
            for line in text.splitlines():
                line = line.strip()
                if line in ["<INTERNAL ERROR>", "<WML ERROR>", "<PARSE ERROR>", "<TIMEOUT ERROR>"]:
                    htmlerr.write("<p>")
                elif line in ["</INTERNAL ERROR>", "</WML ERROR>", "</PARSE ERROR>", "</TIMEOUT ERROR>"]:
                    htmlerr.write("</p>")
                else:
                    err_html = postprocess(line)
                    lines_count += err_html.count("<br")
                    htmlerr.write(err_html)
            htmlerr.write("</body></html>")

            total_lines += lines_count

            total_error_logs += 1
            w('<a class="error" href="%s">%s (%d lines)</a>' % (html_output.cleanurl(error_name), error_kind, lines_count))
        w("</td></tr>")

        count += 1
    w("</tbody>")

    w("<tfoot><tr>")
    w('<th scope="row">Total (for %d addons):</th>' % count)
    w('<td>' + str(total_n) + '</td>')
    w('<td>' + str(total_error_logs) + ' (' + str(total_lines) + ' lines)</td>')
    w('</tr></tfoot>')

    w('</table>')

    w('</div> <!-- overview -->')

    w(html_output.HTML_FOOTER % locals())

if __name__ == "__main__":
    main(sys.argv[1])
