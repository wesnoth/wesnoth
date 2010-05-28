#!/usr/bin/env python
#encoding: utf8

"""
Run with -h for help.

This is a script which tries to parse each add-on and removes it if
there are parser errors.
"""

import optparse, subprocess, sys, re, glob, os
import wmlparser2

class G: pass
g = G()

def parse_test(wml, d):
    all = []
    g.p = wmlparser2.Parser(options.wesnoth)
    try:
        g.p.parse_text(wml, d)
    except wmlparser2.WMLError as e:
        for i, line in enumerate(str(e).splitlines()):
            print(str(1 + i) + ": " + line[:80])
            for mo in re.findall("~add-ons/(.*?)[:/]", line):
                print("    " + mo)
                all.append(mo)

    return all

def get_userdir():
    p = subprocess.Popen([options.wesnoth, "--config-path"],
        stdout = subprocess.PIPE, stderr = subprocess.PIPE)
    out, err = p.communicate()
    return out.strip()

def shell(com):
    print(com)
    p = subprocess.Popen(com, stdout = subprocess.PIPE,
        stderr = subprocess.PIPE, shell = True)
    out, err = p.communicate()
    if out: sys.stdout.write(out)
    if err: sys.stdout.write(err)
    return err

def blacklist_if_faulty(wml, d):
    faulty = parse_test(wml, d)
    if faulty:
        dir = get_userdir()
        for f in faulty:
            com = "mv " + dir + "/data/add-ons/" + f + "* " +\
                options.directory + "/"
            if not shell(com): break
        return True
    return False

def check_runaway():
    dir = get_userdir() + "/data/add-ons"
    g.p = wmlparser2.Parser(options.wesnoth)

    for f in glob.glob(options.runaway + "/*"):
        name = os.path.basename(f)
        if f.endswith(".cfg"): continue
        name = name.replace("'", "'\\''")
        name = "'" + name + "'"
        print("__________\nTesting " + name)
        com = "mv " + options.runaway + "/" + name + "* " + dir + "/"
        shell(com)
        
        ok = True
        try:
            g.p.parse_text("{~add-ons}")
            u = g.p.get_all(tag = "units")
            if u:
                print("Found runaway [units]!")
                ok = False
            pu = g.p.get_all(tag = "+units")
            if pu:
                print("Found runaway [+units]!")
                ok = False
                
            names = []
            for units in u + pu:
                for un in units.get_all(tag = "unit_type"):
                    id = un.get_text_val("id")
                    if id: names.append(id)
            if names: print("Leaked units: " + ", ".join(names))
        except wmlparser2.WMLError as e:
            print("Parsing failed!")
            print("***")
            print(e)
            print("***")
            print("")
            print("Ignoring parse error as we're only doing runaway checks.")
            print("")

        if not ok:
            com = "mv " + dir + "/" + name + "* " + options.runaway + "/"
            shell(com)
            

def main():
    global options
    p = optparse.OptionParser()
    p.add_option("-d", "--directory",
        help = "First move all add-ons into the wesnoth add-ons folder and "
        "this script will then move broken ones to the specified directory.")
    p.add_option("-w", "--wesnoth")
    p.add_option("-r", "--runaway", help = "First move all addons into the "
        "given folder and the ones without runaway units will be "
        "moved into the wesnoth add-ons folder by this script.")
    options, args = p.parse_args()
    
    if not options.wesnoth:
        sys.stderr.write("No Wesnoth executable given.\n")
        sys.exit(1)
    
    if options.runaway:
        check_runaway()
        sys.exit(0)

    # First let's see if we can even parse the list of addons.
    while 1:
        if not blacklist_if_faulty("{~add-ons}", ""): break
        print("FAILED: reading add-ons list. Retrying.")
    print("PASSED: reading add-ons list")

    # Now try to load eras.
    while 1:
        if not blacklist_if_faulty("{multiplayer}{~add-ons}", "MULTIPLAYER"): break
        print("FAILED: reading eras. Retrying.")
    print("PASSED: reading eras")

    # Next try to load campaigns, one by one.
    for c in g.p.get_all(tag = "campaign"):
        d = c.get_text_val("define")
        if blacklist_if_faulty("{~add-ons}", d):
            print("FAILED: " + d)
        else:
            print("PASSED: " + d)

if __name__ == "__main__":
    main()
