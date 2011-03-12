#!/usr/bin/env python
# encoding: UTF-8

import sys, urllib2, urllib, re, htmlentitydefs, httplib, time, optparse

wiki_url = "http://wesnoth.org/mw/index.php"
login_url = "http://wesnoth.org/mw/index.php?title=Special:Userlogin"
#wiki_url = "http://en.wikipedia.org/wiki"
#login_url = "http://en.wikipedia.org/w/index.php?title=Special:Userlogin"

class MediaWiki:
    cookies = {}

    def __init__(self):
        self.read_cookies()

    def read_cookies(self):
        try:
            f = file("cookie.txt")
        except IOError:
            self.login()

        try:
            f = file("cookie.txt")
            for line in f.read().split("\n"):
                kv = line.split("=", 1)
                if len(kv) == 2:
                    self.cookies[kv[0]] = kv[1]
        except IOError:
            sys.stderr.write("Cannot login.\n")

    def store_cookies(self):
        f = file("cookie.txt", "w")
        for c in self.cookies:
            f.write("%s=%s\n" % (c, self.cookies[c]))

    def request(self, url):
        request = urllib2.Request(url)
        #request.set_proxy("localhost:8080", "http")
        request.add_header("User-Agent", "GrabberBot")
        cookies = ""
        for c in self.cookies:
            cookies += "%s=%s; " % (c, self.cookies[c])
        if cookies:
            request.add_header("Cookie", cookies)
        return request

    def login(self):
        sys.stderr.write("logging in\n")

        request = self.request(login_url + "&action=submitlogin")
        data = {
            "wpName": options.username,
            "wpPassword": options.password,
            "wpLoginattempt": "Login",
            "wpRemember": "1"
            }
        data = urllib.urlencode(data, True)

        class redir(urllib2.HTTPRedirectHandler):
            def redirect_request(self2, req, fp, code, msg, hdrs, newurl):
                for h in hdrs.getheaders("set-cookie"):
                    s = h.split("=", 1)
                    key = s[0]
                    value = s[1].split(";", 1)[0]
                    self.cookies[key] = value
                r = self.request(newurl)
                self.store_cookies()
                return r

        opener = urllib2.build_opener(redir())
        site = opener.open(request, data)

        c = site.read()
        mob = re.compile("<p class='error'>(.*?)</p>", re.S).search(c)
        if mob:
            return (True, mob.group(1))

        return (False, "Ok")

    def fetch(self, page):
        page = urllib.quote(page)
        url = wiki_url + "?title=" + page + "&action=edit"

        request = self.request(url)
        while 1:
            try:
                site = urllib2.urlopen(request)
            except httplib.BadStatusLine:
                sys.stderr.write("BadStatusLine when fetching %s\n" % page)
                time.sleep(1)
            else:
                break
        contents = site.read()

        if contents.find("<title>Login required to edit") >= 0:
            err = self.login()
            if err[0]:
                sys.stderr.write(err[1] + "\n")
                raise "Login failed"
            request = self.request(wiki_url + "?title=" + page + "&action=edit")
            site = urllib2.urlopen(request)
            contents = site.read()

        try:
            mob = re.compile("""<input type='hidden' value="(.*?)" name="wpEditToken" />""").search(contents)
            self.token = mob.group(1)
            mob = re.compile("""<input type='hidden' value="(.*?)" name="wpEdittime" />""").search(contents)
            self.time = mob.group(1)
        except AttributeError:
            return None

        mob = re.compile("""<textarea [^>]*?name="wpTextbox1"[^>]*?>(.*?)</textarea>""", re.S).search(contents)
        if mob:
            html = mob.group(1)
            for name, char in htmlentitydefs.name2codepoint.items():
                #if name != "amp":
                html = html.replace("&%s;" % name, unichr(char).encode("utf8"))
            return html
        else:
            return ""

    def post(self, page, text, comment):
        page = urllib.quote(page)
        url = wiki_url + "?title=" + page + "&action=submit"

        request = self.request(url)
        data = {
            "wpSave": "Save page",
            "wpSection": "",
            "wpSummary": comment,
            "wpEdittime": self.time,
            "wpTextbox1": text,
            "wpMinoredit": "1",
            "wpEditToken": self.token}
        data = urllib.urlencode(data, True)
        class ok(Exception):
            pass
        class redir(urllib2.HTTPRedirectHandler):
            def redirect_request(self2, req, fp, code, msg, hdrs, newurl):
                raise ok
        opener = urllib2.build_opener(redir())
        try:
            while 1:
                try:
                    site = opener.open(request, data)
                except httplib.BadStatusLine:
                    sys.stderr.write("BadStatusLine when posting %s\n" % page)
                    time.sleep(1)
                else:
                    break
        except ok:
            return False
        return True

if __name__ == "__main__":
    global options
    p = optparse.OptionParser()
    p.add_option("-u", "--username")
    p.add_option("-p", "--password")
    p.add_option("-n", "--name", help = "Name of the page.")
    p.add_option("-s", "--send", action = "store_true",
        help = "Read text from stdin and replace the page with it.")
    p.add_option("-r", "--receive", action = "store_true",
        help = "Read the page and output the contents.")
    p.add_option("-c", "--comment", default = "autogenerated",
        help = "Optional comment which appears in the edit history.")
    options, args = p.parse_args()

    def error(message):
        sys.stderr.write(message + "\n")
        p.print_help()
        sys.exit(1)

    if not options.username: error("Username required.")
    if not options.password: error("Password required.")
    if not options.name: error("Page name required.")
    if not options.send and not options.receive:
        error("Either --send or --receive operation is required.")

    mw = MediaWiki()
    oldpage = mw.fetch(options.name)
    if options.receive:
        if oldpage:
            sys.stdout.write(oldpage)
        else:
            sys.stderr.write("Page %s does not exist.\n" % options.name)
    if options.send:
        newpage = sys.stdin.read()
        mw.post(
            options.name,
            newpage,
            options.comment)
