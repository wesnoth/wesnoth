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
    g.p = wmlparser2.Parser(options.wesnoth, options.config_dir,
        options.data_dir, no_preprocess = False)
    try:
        g.p.parse_text(wml, d)
    except wmlparser2.WMLError, e:
        for i, line in enumerate(str(e).splitlines()):
            print(str(1 + i) + ": " + line)
            for mo in re.findall("~add-ons/(.*?)[:/]", line):
                print("    " + mo)
                all.append(mo)

    return all

def get_userdir():
    if options.config_dir: return options.config_dir
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

    campaigns = {}

    udir = get_userdir() + "/data/add-ons"
    g.p = wmlparser2.Parser(options.wesnoth, options.config_dir,
        options.data_dir, no_preprocess = False)

    def bash(name):
        return "'" + name.replace("'", "'\\''") + "'"

    def move(f, t, name):

        if os.path.exists(f + "/" + name + ".cfg"):
            com = "mv " + f + "/" + bash(name + ".cfg") + " " + t + "/"
            shell(com)

        com = "mv " + f + "/" + bash(name) + " " + t + "/"
        shell(com)

    total = []
    passed = []
    for f in glob.glob(options.runaway + "/*"):
        name = os.path.basename(f)
        if f.endswith(".cfg"): continue

        print("__________\nTesting " + name)
        move(options.runaway, udir, name)


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
        except wmlparser2.WMLError, e:
            print("Parsing failed!")
            print("***")
            print(e)
            print("***")
            print("")
            ok = False

        if ok:
            campaigns[name] = g.p.get_all(tag = "campaign")

        move(udir, options.runaway, name)

        if ok:
            passed.append(name)
        total.append(name)

    print("\n%d/%d addons passed runaway test. Trying to parse them." % (
        len(passed), len(total)))

    parsed = []
    for name in passed:
        print("__________\nParsing " + name)
        move(options.runaway, udir, name)

        ok = True
        campaign = campaigns[name]
        if campaign and campaign[0].get_text_val("define", None):
            errors = parse_test("{~add-ons}",
                campaign[0].get_text_val("define", ""))
        else:
            errors = parse_test("{multiplayer}{~add-ons}", "MULTIPLAYER")
        if errors:
            print(errors)
            ok = False
            print("Parsing failed!")

        move(udir, options.runaway, name)

        if ok:
            parsed.append(name)


    print("\n%d/%d addons could be parsed. Moving them to the add-ons folder.\n" % (
        len(parsed), len(total)))
    for name in parsed:
        move(options.runaway, udir, name)

    print("\nSome addons may have failed simply because of unmet "
        "dependencies, as this test considers each one in isolation. "
        "TODO: Someone should fix this or tell me how to fix it.")

def main():
    global options
    p = optparse.OptionParser()
    p.add_option("-C", "--config-dir",
        help = "Specify the user configuration dir (wesnoth --config-path).")
    p.add_option("-D", "--data-dir",
        help = "Specify the wesnoth data dir (wesnoth --path).")
    p.add_option("-d", "--directory",
        help = "First move all add-ons into the wesnoth add-ons folder and "
        "this script will then move broken ones to the specified directory.")
    p.add_option("-w", "--wesnoth")
    p.add_option("-r", "--runaway",
        help = "First move all add-ons into the given folder and "
        "this script will then move them into the add-ons folder "
        "making sure there are no run-away units.")
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
