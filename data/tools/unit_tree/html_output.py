# encoding: utf-8

import os, gettext, time, copy, sys, re
import traceback
import unit_tree.helpers as helpers
import wesnoth.wmlparser3 as wmlparser3


pics_location = "../../pics"

html_entity_horizontal_bar = '&#8213;'

html_header = '''
<!DOCTYPE html>

<html class="no-js wmlunits %(classes)s" lang="en">
<head>
	<meta charset="utf-8" />
	<meta name="viewport" content="width=device-width,initial-scale=1" />

	<link rel="stylesheet" href="https://fonts.googleapis.com/css?family=Montaga%%7COpen+Sans:400,400i,700,700i" type="text/css" />
	<link rel="icon" type="image/png" href="https://www.wesnoth.org/wesmere/img/favicon-32.png" sizes="32x32" />
	<link rel="icon" type="image/png" href="https://www.wesnoth.org/wesmere/img/favicon-16.png" sizes="16x16" />
	<link rel="stylesheet" type="text/css" href="http://wesmere.localhost/wesmere/css/wesmere-1.1.0.css" />
	<link rel="stylesheet" type="text/css" href="http://wesmere.localhost/wesmere/css/wmlunits-1.1.0.css" />
	<script src="https://www.wesnoth.org/wesmere/js/modernizr.js"></script>
    <script type="text/javascript" src="%(path)s/menu.js"></script>
    <title>%(title)s - Wesnoth Units Database</title>
</head>

<body>

<div id="main">'''.strip()

top_bar = '''
<div id="nav" role="banner">
<div class="centerbox">

	<div id="logo">
		<a href="https://www.wesnoth.org/" aria-label="Wesnoth logo"></a>
	</div>

	<ul id="navlinks">
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

<div id="content" role="main">'''.strip()

html_footer = '''
</div> <!-- end content -->

<div class="centerbox"><div id="lastmod">%(generation_note)s</div></div>

</div> <!-- end main -->

<div id="footer-sep"></div>

<div id="footer"><div id="footer-content"><div>
	<a href="https://wiki.wesnoth.org/StartingPoints">Site Map</a> &#8226; <a href="http://status.wesnoth.org/">Site Status</a><br />
	Copyright &copy; 2003&ndash;2017 by <a rel="author" href="https://wiki.wesnoth.org/Project">The Battle for Wesnoth Project</a>.<br />
	Site design Copyright &copy; 2017 by Ignacio R. Morelle.
</div></div></div>

</body></html>
'''.strip()

html_clear_floats = '<div class="reset"></div>'

all_written_html_files = []

error_only_once = {}
def error_message(message):
    if message in error_only_once: return
    error_only_once[message] = 1
    write_error(message)

helpers.error_message = error_message

def reset_errors():
    error_only_once = {}

class MyFile:
    """
    Python 2 is a bit weird with encodings, really should switch this to
    Python 3.
    """
    def __init__(self, filename, mode):
        self.filename = filename
        self.f = open(filename, mode + "b")
    def write(self, x):
        self.f.write(x.encode("utf8"))
    def close(self):
        self.f.close()


class Translation:
    def __init__(self, localedir, langcode):
        self.catalog = {}
        self.localedir = localedir
        self.langcode = langcode
        class Dummy:
            def gettext(self, x):
                if not x: return ""
                caret = x.find("^")
                if caret < 0: return x
                return x[caret + 1:]
        self.dummy = Dummy()

    def translate(self, string, textdomain):
        if textdomain not in self.catalog:
            try:
                self.catalog[textdomain] = gettext.translation(
                    textdomain, self.localedir, [self.langcode])
                self.catalog[textdomain].add_fallback(self.dummy)
            except IOError:
                self.catalog[textdomain] = self.dummy
            except AttributeError:
                self.catalog[textdomain] = self.dummy
            except IndexError:
                # not sure why, but this happens within the
                # gettext.translation call sometimes
                self.catalog[textdomain] = self.dummy
            except ValueError:
                self.catalog[textdomain] = self.dummy

        r = self.catalog[textdomain].gettext(string)

        return r


class GroupByRace:
    def __init__(self, wesnoth, campaign):
        self.wesnoth = wesnoth
        self.campaign = campaign

    def unitfilter(self, unit):
        if not self.campaign: return True
        return unit.campaigns and self.campaign == unit.campaigns[0]

    def groups(self, unit):
        return [T(unit.race, "plural_name")]

    def group_name(self, group):
        if not group: return "None"
        return group

class GroupByNothing:
    def __init__(self):
        pass

    def unitfilter(self, unit):
        return True

    def groups(self, unit):
        return ["units"]

    def group_name(self, group):
        return "units"

class GroupByFaction:
    def __init__(self, wesnoth, era):
        self.wesnoth = wesnoth
        self.era = era

    def unitfilter(self, unit):
        return self.era in unit.eras

    def groups(self, unit):
        return [x for x in unit.factions if x[0] == self.era]

    def group_name(self, group):
        era = self.wesnoth.era_lookup[group[0]]
        if group[1]:
            faction = era.faction_lookup[group[1]]
            name = T(faction, "name")
            name = name[name.rfind("=") + 1:]
        else:
            name = "factionless"
        return name

global_htmlout = None
def T(tag, att):
    if not tag: return "none"
    return tag.get_text_val(att, translation = global_htmlout.translate)


class HTMLOutput:
    def __init__(self, isocode, output, addon, campaign, is_era, wesnoth, verbose = False):
        global global_htmlout
        self.output = output
        self.addon = addon
        self.campaign = campaign
        self.is_era = is_era
        self.verbose = verbose
        self.target = "index.html"
        self.wesnoth = wesnoth
        self.forest = None
        self.translation = Translation(options.transdir, isocode)
        self.isocode = isocode
        global_htmlout = self

    def translate(self, string, domain):
        return self.translation.translate(string, domain)

    def analyze_units(self, grouper, add_parents):
        """
        This takes all units belonging to a campaign, then groups them either
        by race or faction, and creates an advancements tree out of it.
        """

        # Build an advancement tree forest of all units.
        forest = self.forest = helpers.UnitForest()
        units_added = {}
        for uid, u in list(self.wesnoth.unit_lookup.items()):
            if u.hidden: continue
            if grouper.unitfilter(u):
                forest.add_node(helpers.UnitNode(u))
                units_added[uid] = u

        #print("    %d/%d units" % (len(units_added), len(self.wesnoth.unit_lookup)))

        # Always add any child units, even if they have been filtered out..
        while units_added:
            new_units_added = {}
            for uid, u in list(units_added.items()):
                for auid in u.advance:
                    if not auid in forest.lookup:
                        try:
                            au = self.wesnoth.unit_lookup[auid]
                        except KeyError:
                            error_message(
                                "Warning: Unit %s not found as advancement of %s\n" %
                                (auid, repr(uid)))
                            continue
                        forest.add_node(helpers.UnitNode(au))
                        new_units_added[auid] = au
            units_added = new_units_added

        if add_parents:
            # Also add parent units
            added = True
            while added:
                added = False
                for uid, u in list(self.wesnoth.unit_lookup.items()):
                    if uid in forest.lookup: continue
                    for auid in u.advance:
                        if auid in forest.lookup:
                            forest.add_node(helpers.UnitNode(u))
                            added = True
                            break

        forest.update()

        # Partition trees by race/faction of first unit.
        groups = {}
        breadth = 0

        for tree in list(forest.trees.values()):
            u = tree.unit
            ugroups = grouper.groups(u)

            for group in ugroups:
                groups[group] = groups.get(group, []) + [tree]
                breadth += tree.breadth

        thelist = list(groups.keys())
        thelist.sort(key = lambda x: grouper.group_name(x))

        rows_count = breadth + len(thelist)
        # Create empty grid.
        rows = []
        for j in range(rows_count):
            column = []
            for i in range(6):
                column.append((1, 1, None))
            rows.append(column)

        # Sort advancement trees by name of first unit and place into the grid.
        def by_name(t):
            x = T(t.unit, "name")
            if x is None: return ""
            return x

        def grid_place(nodes, x):
            nodes.sort(key = by_name)
            for node in nodes:
                level = node.unit.level
                if level < 0: level = 0
                if level > 5: level = 5
                rows[x][level] = (1, node.breadth, node)
                for i in range(1, node.breadth):
                    rows[x + i][level] = (0, 0, node)
                grid_place(node.children, x)
                x += node.breadth
            return x

        x = 0
        for group in thelist:
            node = helpers.GroupNode(group)
            node.name = grouper.group_name(group)

            rows[x][0] = (6, 1, node)
            for i in range(1, 6):
                rows[x][i] = (0, 0, None)
            nodes = groups[group]
            x += 1
            x = grid_place(nodes, x)

        self.unitgrid = rows
        
        return len(forest.lookup)

    def write_navbar(self, report_type):
        def write(x): self.output.write(x)
        
        all_written_html_files.append((self.isocode, self.output.filename))

        languages = self.wesnoth.languages_found
        langlist = list(languages.keys())
        langlist.sort()
        
        write(top_bar % {"path" : "../../"})

        write("""
        

        <div class="navbar">
        """)

        write('<ul class="navbar" role="menu">')

        def abbrev(name):
            abbrev = name[0]
            word_seperators = [" ", "_", "+", "(", ")"]
            for i in range(1, len(name)):
                if name[i] in ["+", "(", ")"] or name[i - 1] in word_seperators and name[i] not in word_seperators:
                    abbrev += name[i]
            return abbrev

        def add_menu(id, name, class2=""):
            write("""<li class="popuptrigger" role="menuitem" aria-haspopup="true"
                onclick="toggle_menu(this, '""" + id + """', 2)"
                onmouseover="toggle_menu(this, '""" + id + """', 1)"
                onmouseout="toggle_menu(this, '""" + id + """', 0)">""")
            write('<a class="' + class2 + '">' + name + "</a>")
            write('<ul class="popupmenu" id="' + id + '" role="menu" aria-label="' + name + '">')
            write('<li>' + name + '</li>')

        # FIXME: This is legacy code needed for the Language menu, since it's
        #        a table and we can't make it otherwise since CSS column
        #        support is still hit-or-miss for some browsers still in use.
        def add_menu2(id, name, class2=""):
            write("""<li class="popuptrigger" role="menuitem" aria-haspopup="true"
                onclick="toggle_menu(this, '""" + id + """', 2)"
                onmouseover="toggle_menu(this, '""" + id + """', 1)"
                onmouseout="toggle_menu(this, '""" + id + """', 0)">""")
            write('<a class="' + class2 + '">' + name + "</a>")
            write('<div class="popupmenu" id="' + id + '" role="menu" aria-label="' + name + '">')
            write('<div>' + name + '</div>')

        def end_menu():
            write('</ul></li>\n')

        # We may not have all the required info yet so defer writing the
        # campaigns/eras navigation.
        
        # Campaigns
        x = self.translate("addon_type^Campaign", "wesnoth")
        add_menu("campaigns_menu", x)
        write("PLACE CAMPAIGNS HERE\n")
        end_menu()
        
        # Eras
        x = self.translate("Era", "wesnoth")
        add_menu("eras_menu", x)
        write("PLACE ERAS HERE\n")
        end_menu()

        # Races / Factions

        target = self.target
        if self.campaign == "units":
            target = "mainline.html"

        if not self.is_era:
            
            x = self.translate("Race", "wesnoth-lib")
            add_menu("races_menu", x)

            write('<li><a href="mainline.html" role="menuitem">%s</a></li>\n' % (
                self.translate("all", "wesnoth-editor")))

            r = {}, {}
            for u in list(self.wesnoth.unit_lookup.values()):

                race = u.race
                racename = T(race, "plural_name")

                m = 1
                if u:
                    m = 0

                rname = race.get_text_val("id") if race else "none"
                if not rname:
                    rname = "none"
                r[m][racename] = rname
            racenames = sorted(r[0].items())
            if list(r[1].items()):
                racenames += [("-", "-")] + sorted(r[1].items())

            for racename, rid in racenames:
                if racename == "-":
                    write('<li>%s</li>' % html_entity_horizontal_bar)
                else:
                    write('<li><a href="%s#%s" role="menuitem">%s</a></li>' % (
                        target, racename, racename))

            end_menu()
        else:
            x = self.translate("Factions", "wesnoth-help")
            add_menu("races_menu", x)

            for row in self.unitgrid:
                for column in range(6):
                    hspan, vspan, un = row[column]
                    if not un: continue
                    if isinstance(un, helpers.GroupNode):
                        html = "../%s/%s.html" % (
                            self.isocode, self.campaign)
                        write('<li><a href="%s#%s" role="menuitem">%s</a></li>' % (
                            html, un.name, un.name))

            end_menu()

        # Add entries for the races also to the navbar itself.
        if not self.is_era:
            class Entry: pass
            races = {}

            for uid, u in list(self.wesnoth.unit_lookup.items()):
                if self.campaign != "units":
                    if self.campaign not in u.campaigns: continue
                if u.race:
                    racename = T(u.race, "plural_name")
                else:
                    racename = "none"
                runits = races.get(racename, [])
                runits.append(uid)
                races[racename] = runits

            racelist = sorted(races.keys())
            got_menu = False
            menuid = 0
            for r in racelist:
                if not r:
                    continue
                if got_menu:
                    end_menu()
                add_menu("units_menu" + str(menuid), r, "unitmenu")
                menuid += 1
                got_menu = True
                c = self.campaign
                if c == "units": c = "mainline"
                write('<li><a href="%s#%s" role="menuitem">%s</a></li>' % (
                    target, r, r))
                for uid in races[r]:
                    un = self.wesnoth.unit_lookup[uid]
                    if un.hidden: continue
                    if "mainline" in un.campaigns: addon = "mainline"
                    else: addon = self.addon
                    link = "../../%s/%s/%s.html" % (addon, self.isocode, uid)
                    name = self.wesnoth.get_unit_value(un,
                        "name", translation=self.translation.translate)
                    if not name:
                        error_message("Warning: Unit uid=" + uid + " has no name.\n")
                        name = uid
                    write('<li><a href="' + link + '" role="menuitem">' + name + '</a></li>')
            if got_menu:
                end_menu()

        # Languages
        x = self.translate("Language", "wesnoth")
        add_menu2("languages_menu", x)
        col = 0
        maxcol = len(langlist) - 1
        write("<table>")
        write("<tr>")
        for lang in langlist:
            col += 1
            write("<td>")
            labb = lang
            #underscore = labb.find("_")
            #if underscore > 0: labb = labb[:underscore]
            if self.addon == "mainline":
                write('<a title="%s" href="../%s/%s" role="menuitem">%s</a>\n' % (
                    languages[lang], lang, self.target,
                        labb))
            else:
                write('<a title="%s" href="../%s/%s">%s</a>\n' % (
                    languages[lang], lang, "mainline.html",
                        labb))    
            write("</td>")
            if col % 5 == 0:
                if col < maxcol: write("</tr><tr>")

        write("</tr>")
        write("</table>")
        write("</div></li>\n")
        
        write('<li class="overviewlink"><a href="../../overview.html">Build Report</a></li>')

        write("</ul>\n")

        write("</div>\n")

    def pic(self, u, x, recursion = 0):
        if recursion >= 4:
            error_message(
                "Warning: Cannot find image for unit %s(%s).\n" % (
                u.get_text_val("id"), x.name.decode("utf8")))
            return None, None
        image = self.wesnoth.get_unit_value(x, "image")
        portrait = x.get_all(tag="portrait")
        if not portrait:
            bu = self.wesnoth.get_base_unit(u)
            if bu:
                portrait = bu.get_all(tag="portrait")
        if portrait:
            portrait = portrait[0].get_text_val("image")
        if not image:
            if x.name == b"female":
                baseunit = self.wesnoth.get_base_unit(u)
                if baseunit:
                    female = baseunit.get_all(tag="female")
                    return self.pic(u, female[0], recursion = recursion + 1)
                else:
                    return self.pic(u, u, recursion = recursion + 1)
            error_message(
                "Warning: Missing image for unit %s(%s).\n" % (
                u.get_text_val("id"), x.name.decode("utf8")))
            return None, None
        icpic = image_collector.add_image_check(self.addon, image)
        if not icpic.ipath:
            error_message("Warning: No picture %s for unit %s.\n" %
                (image, u.get_text_val("id")))
        picname = icpic.id_name
        image = os.path.join(pics_location, picname)
        if portrait:
            picname = image_collector.add_image(self.addon, portrait,
                no_tc=True)
            portrait = os.path.join(pics_location, picname)
        return image, portrait

    def get_abilities(self, u):
        anames = []
        already = {}
        for abilities in u.get_all(tag="abilities"):
            try: c = abilities.get_all()
            except AttributeError: c = []
            for ability in c:
                try:
                    id = ability.get_text_val("id")
                except AttributeError as e:
                    error_message("Error: Ignoring ability " + ability.debug())
                    continue
                if id in already: continue
                already[id] = True
                name = T(ability, "name")
                if not name: name = id
                if not name: name = ability.name.decode("utf8")
                anames.append(name)
        return anames

    def get_recursive_attacks(self, this_unit):

        def copy_attributes(copy_from, copy_to):
            for c in copy_from.data:
                if isinstance(c, wmlparser3.AttributeNode):
                    copy_to.data.append(c)

        # Use attacks of base_units as base, if we have one.
        base_unit = self.wesnoth.get_base_unit(this_unit)
        attacks = []
        if base_unit:
            attacks = copy.deepcopy(self.get_recursive_attacks(base_unit))

        base_attacks_count = len(attacks)
        for i, attack in enumerate(this_unit.get_all(tag="attack")):
            # Attack merging is order based.
            if i < base_attacks_count:
                copy_attributes(attack, attacks[i])
            else:
                attacks.append(attack)

        return attacks

    def write_units(self):
        def write(x): self.output.write(x)
        def _(x, c="wesnoth"): return self.translate(x, c)
        rows = self.unitgrid
        write("<table class=\"units\">\n")
        write("<colgroup>")
        for i in range(6):
            write("<col class=\"col%d\" />" % i)
        write("</colgroup>")

        pic = image_collector.add_image("general",
            "../../../images/misc/leader-crown.png", no_tc=True)
        crownimage = os.path.join(pics_location, pic)
        ms = None
        for row in range(len(rows)):
            write("<tr>\n")
            for column in range(6):
                hspan, vspan, un = rows[row][column]
                if vspan:
                    attributes = ""
                    if hspan == 1 and vspan == 1:
                        pass
                    elif hspan == 1:
                        attributes += " rowspan=\"%d\"" % vspan
                    elif vspan == 1:
                        attributes += " colspan=\"%d\"" % hspan

                    if un and isinstance(un, helpers.GroupNode):
                        # Find the current multiplayer side so we can show the
                        # little crowns..
                        ms = None
                        if self.is_era:
                            try:
                                eid, fid = un.data
                                era = self.wesnoth.era_lookup[eid]
                                if fid:
                                    ms = era.faction_lookup[fid]
                            except TypeError:
                                pass
                        racename = un.name
                        # TODO: we need to use a unique race id instead of a potentially duplicate
                        #       name for the header id and link target!
                        attributes += " id=\"%s\" class=\"raceheader\"" % racename
                        write("<th" + attributes + ">")
                        write("<a href=\"#%s\">%s</a>" % (racename, racename))
                        write("</th>\n")
                    elif un:
                        u = un.unit
                        attributes += " class=\"unitcell\""
                        write("<td%s>" % attributes)

                        uid = u.get_text_val("id")
                        def uval(name):
                            return self.wesnoth.get_unit_value(u, name,
                                translation=self.translation.translate)
                        name = uval("name")
                        cost = uval("cost")
                        hp = uval("hitpoints")
                        mp = uval("movement")
                        xp = uval("experience")
                        level = uval("level")

                        crown = ""
                        if ms:
                            if un.id in ms.units:
                                crown = " ♟"
                            if un.id in ms.is_leader:
                                crown = " ♚"

                        uaddon = "mainline"
                        if "mainline" not in u.campaigns: uaddon = self.addon
                        link = "../../%s/%s/%s.html" % (uaddon, self.isocode, uid)
                        write("<div class=\"l\">L%s%s</div>" % (level, crown))
                        write("<a href=\"%s\" title=\"Id: %s\">%s</a><br/>" % (link, uid, name))

                        write('<div class="pic">')
                        image, portrait = self.pic(u, u)

                        write('<a href=\"%s\">' % link)

                        if crown == " ♚":
                            write('<div class="spritebg" style="background-image:url(\'%s\')">' % image)
                            write('<img src="%s" alt="(image)" />' % crownimage)
                            write("</div>")
                        else:
                            write('<img src="%s" alt="(image)" />' % image)

                        write('</a>\n</div>\n')
                        write("<div class=\"attributes\">")

                        attributes = (
                            (_("Cost: ", "wesnoth-help"), cost),
                            (_("HP: "), hp),
                            (_("XP: "), xp),
                            (_("MP: "), mp),
                        )

                        for attr_label, attr_value in attributes:
                            write('<span class="attribute-label">%s</span> <span class="attribute-value">%s</span><br />' % (attr_label.strip(), attr_value))

                        # Write info about abilities.
                        anames = self.get_abilities(u)
                        if anames:
                            write("\n<div class=\"abilities\">")
                            write(", ".join(anames))
                            write("</div>")

                        # Write info about attacks.
                        write("\n<div class=\"attacks\">")
                        attacks = self.get_recursive_attacks(u)
                        for attack in attacks:

                            n = T(attack, "number")
                            x = T(attack, "damage")
                            x = "%s - %s" % (x, n)
                            write("%s " % x)

                            r = T(attack, "range")
                            t = T(attack, "type")
                            write("%s (%s)" % (_(r), _(t)))

                            s = []
                            specials = attack.get_all(tag="specials")
                            if specials:
                                for special in specials[0].get_all(tag=""):
                                    sname = T(special, "name")
                                    if sname:
                                        s.append(sname)
                                s = ", ".join(s)
                                if s: write(" (%s)" % s)
                            write("<br />")
                        write("</div>")

                        write("</div>")
                        write("</td>\n")
                    else:
                        write("<td class=\"empty\"></td>")
            write("</tr>\n")
        write("</table>\n")

    def write_units_tree(self, grouper, title, add_parents):
        self.output.write(html_header % {"path": "../../",
            "title": title, "classes": "wmlunits-tree"})

        n = self.analyze_units(grouper, add_parents)
        self.write_navbar("units_tree")

        self.output.write("<div class=\"main\">")

        self.output.write("<h1>%s</h1>" % title)

        self.write_units()

        self.output.write(html_clear_floats)
        self.output.write("</div>")

        self.output.write(html_footer % {
            "generation_note": "Last updated on " + time.ctime() + "."})
        
        return n

    def write_unit_report(self, output, unit):
        def write(x): self.output.write(x)
        def _(x, c="wesnoth"): return self.translate(x, c)

        def find_attr(what, key):
            if unit.movetype:
                mtx = unit.movetype.get_all(tag=what)
                mty = None
                if mtx:
                    mty = mtx[0].get_text_val(key)
            x = unit.get_all(tag=what)
            y = None
            if x:
                y = x[0].get_text_val(key,
                translation=self.translation.translate)
            if y:
                return True, y
            if unit.movetype and mty != None:
                return False, mty
            return False, "-"

        def uval(name):
            return self.wesnoth.get_unit_value(unit, name,
                translation=self.translation.translate)

        # Write unit name, picture and description.
        uid = unit.get_text_val("id")
        uname = uval("name")
        display_name = uname

        self.output = output
        write(html_header % {"path": "../../",
            "title": display_name, "classes": "wmlunits-unit"})
        self.write_navbar("unit_report")

        self.output.write("<div class=\"main\">")

        female = unit.get_all(tag="female")
        if female:
            fname = T(female[0], "name")
            if fname and fname != uname:
                display_name += "<br/>" + fname

        write('<div class="unit-columns">')

        write('<div class="unit-column-left">')

        write("<h1>%s</h1>\n" % display_name)

        write('<div class="pic">')
        if female:
            mimage, portrait = self.pic(unit, unit)
            fimage, fportrait = self.pic(unit, female[0])
            if not fimage: fimage = mimage
            if not fportrait: fportrait = portrait
            write('<img src="%s" alt="(image)" />\n' % mimage)
            write('<img src="%s" alt="(image)" />\n' % fimage)
            image = mimage
        else:
            image, portrait = self.pic(unit, unit)
            write('<img src="%s" alt="(image)" />\n' % image)
        write('</div>\n')

        description = uval("description")

        # TODO: what is unit_description?
        if not description: description = uval("unit_description")
        if not description: description = "-"
        write("<p>%s</p>\n" % re.sub("\n", "\n<br />", description))

        # Base info.
        hp = uval("hitpoints")
        mp = uval("movement")
        xp = uval("experience")
        vision = uval("vision")
        jamming = uval("jamming")
        level = uval("level")
        alignment = uval("alignment")

        write("<h2>Information</h2>\n")
        write("<table class=\"unitinfo\">\n")
        write("<tr>\n")
        write("<th>%s" % _("Advances from: ", "wesnoth-help"))
        write("</th><td>\n")
        for pid in self.forest.get_parents(uid):
            punit = self.wesnoth.unit_lookup[pid]
            if "mainline" in unit.campaigns and "mainline" not in punit.campaigns:
                continue
            
            if "mainline" in unit.campaigns: addon = "mainline"
            else: addon = self.addon
            link = "../../%s/%s/%s.html" % (addon, self.isocode, pid)
            name = self.wesnoth.get_unit_value(punit, "name",
                translation=self.translation.translate)
            write("\n<a href=\"%s\">%s</a>" % (link, name))
        write("</td>\n")
        write("</tr><tr>\n")
        write("<th>%s" % _("Advances to: ", "wesnoth-help"))
        write("</th><td>\n")
        for cid in self.forest.get_children(uid):
            try:
                cunit = self.wesnoth.unit_lookup[cid]
                if "mainline" in cunit.campaigns: addon = "mainline"
                else: addon = self.addon
                link = "../../%s/%s/%s.html" % (addon, self.isocode, cid)
                if "mainline" in unit.campaigns and "mainline" not in cunit.campaigns:
                    continue
                name = self.wesnoth.get_unit_value(cunit, "name",
                    translation=self.translation.translate)
            except KeyError:
                error_message("Warning: Unit %s not found.\n" % cid)
                name = cid
                if "mainline" in unit.campaigns: continue
                link = self.target
            write("\n<a href=\"%s\">%s</a>" % (link, name))
        write("</td>\n")
        write("</tr>\n")

        for val, text in [
            ("cost", _("Cost: ", "wesnoth-help")),
            ("hitpoints", _("HP: ")),
            ("movement", _("Movement", "wesnoth-help") + ": "),
            ("vision", _("Vision", "wesnoth-help") + ": "),
            ("jamming", _("Jamming", "wesnoth-help") + ":"),
            ("experience", _("XP: ")),
            ("level", _("Level") + ": "),
            ("alignment", _("Alignment: ")),
            ("id", "Id: ")]:
            x = uval(val)
            if not x and val in ("jamming", "vision"): continue
            if val == "alignment": x = _(x)
            write("<tr>\n")
            write("<th>%s</th>" % text)
            write("<td class=\"val\">%s</td>" % x)
            write("</tr>\n")

        # Write info about abilities.
        anames = self.get_abilities(unit)

        write("<tr>\n")
        write("<th>%s</th>" % _("Abilities: ", "wesnoth-help"))
        write("<td class=\"val\">" + (", ".join(anames)) + "</td>")
        write("</tr>\n")

        write("</table>\n")

        # Write info about attacks.
        write("<h2>" + _("unit help^Attacks", "wesnoth-help") + " <small>(damage - count)</small></h2> \n")
        write("<table class=\"unitinfo attacks\">\n")
        write('<colgroup><col class="col0" /><col class="col1" /><col class="col2" /><col class="col3" /><col class="col4" /></colgroup>')
        attacks = self.get_recursive_attacks(unit)
        for attack in attacks:
            write("<tr>")

            aid = attack.get_text_val("name")
            aname = T(attack, "description")

            icon = attack.get_text_val("icon")
            if not icon:
                icon = "attacks/%s.png" % aid

            image_add = image_collector.add_image_check(self.addon,
                icon, no_tc = True)
            if not image_add.ipath:
                error_message("Error: No attack icon '%s' found for '%s'.\n" % (
                    icon, uid))
                icon = os.path.join(pics_location, "unit$elves-wood$shaman.png")
            else:
                icon = os.path.join(pics_location, image_add.id_name)
            write("<td><img src=\"%s\" alt=\"(image)\"/></td>" % icon)

            write("<td><b>%s</b>" % aname)

            r = T(attack, "range")
            write("<br/>%s</td>" % _(r))

            n = attack.get_text_val("number")
            x = attack.get_text_val("damage")
            x = "%s - %s" % (x, n)
            write("<td><i>%s</i>" % x)

            t = T(attack, "type")
            write("<br/>%s</td>" % _(t))

            s = []
            specials = attack.get_all(tag="specials")

            if specials:
                for special in specials[0].get_all(tag=""):
                    sname = T(special, "name")
                    if sname:
                        s.append(sname)
                    else:
                        error_message(
                            "Warning: Weapon special %s has no name for %s.\n" % (
                            special.name.decode("utf8"), uid))
            s = "<br/>".join(s)
            write("<td>%s</td>" % s)


            write("</tr>")
        write("</table>\n")

        # Write info about resistances.
        resistances = [
            ("blade", "attacks/sword-human.png"),
            ("pierce", "attacks/spear.png"),
            ("impact", "attacks/club.png"),
            ("fire", "attacks/fireball.png"),
            ("cold", "attacks/iceball.png"),
            ("arcane", "attacks/faerie-fire.png")]

        write("<h2>%s</h2>\n" % _("Resistances: ").strip(" :"))
        write("<table class=\"unitinfo resistances\">\n")
        write('<colgroup><col class="col0" /><col class="col1" /><col class="col2" /><col class="col3" /><col class="col4" /><col class="col5" /><col class="col6" /><col class="col7" /></colgroup>')
        write("<tr>\n")

        write("</tr>\n")
        row = 0
        for rid, ricon in resistances:
            special, r = find_attr("resistance", rid)
            if r == "-": r = 100
            try: r = "<i>%d%%</i>" % (100 - int(r))
            except ValueError:
                error_message("Warning: Invalid resistance %s for %s.\n" % (
                    r, uid))
            rcell = "td"
            if special: rcell += ' class="special"'
            if row % 2 == 0: write("<tr>\n")
            else: write("<td></td>")
            picname = image_collector.add_image(self.addon, ricon,
                no_tc = True)
            icon = os.path.join(pics_location, picname)
            write("<td><img src=\"%s\" alt=\"(icon)\" /></td>\n" % (icon, ))
            write("<th>%s</th><td class=\"num\">%s</td>\n" % (_(rid), r))
            if row % 2 == 1: write("</tr>\n")
            row += 1
        write("</table>\n")

        # end left column
        write('</div>')
        write('<div class="unit-column-right">')

        for si in range(2):
            if si and not female: break
            if si:
                sportrait = fportrait
                simage = fimage
            else:
                simage = image
                sportrait = portrait

            style = "background-image: url(%s);" % simage

            write('<div class="portrait">')
            write('<div style="%s">&nbsp;</div>' % style)
            if portrait:
                write('<img src="%s" alt="(portrait)" />\n' % sportrait)
            write('</div>')

        # Write info about movement costs and terrain defense.
        write("<h2>" + _("Terrain", "wesnoth-help") + "</h2>\n")
        write("<table class=\"unitinfo terrain\">\n")
        write('<colgroup><col class="col0" /><col class="col1" /><col class="col2" /><col class="col3" /><col class="col4" /></colgroup>')

        write("<tr><th colspan=\"2\"></th><th colspan=\"2\">%s</th></tr>\n" % (
            _("Movement Cost", "wesnoth-help")))
        write("<tr><th colspan=\"2\">%s</th><th></th><th class=\"numheader\">%s</th></tr>\n" % (
            _("Terrain", "wesnoth-help"), _("Defense", "wesnoth-help")))

        terrains = self.wesnoth.terrain_lookup
        terrainlist = []
        already = {}
        for tstring, t in list(terrains.items()):
            tid = t.get_text_val("id")
            if tid in ["off_map", "off_map2", "fog", "shroud", "impassable",
                "void", "rails"]: continue
            if t.get_all(att="aliasof"): continue
            if tid in already: continue
            already[tid] = 1
            name = T(t, "name")
            ticon = t.get_text_val("symbol_image")
            if not ticon:
                ticon = t.get_text_val("icon_image")

            # Use nice images for known mainline terrain types
            if tid == "fungus": ticon = "forest/mushrooms-tile"
            elif tid == "cave": ticon = "cave/floor6"
            elif tid == "sand": ticon = "sand/beach"
            elif tid == "reef": ticon = "water/reef-tropical-tile"
            elif tid == "hills": ticon = "hills/regular"
            elif tid == "swamp_water": ticon = "swamp/water-tile"
            elif tid == "shallow_water": ticon = "water/coast-tile"
            elif tid == "castle": ticon = "castle/castle-tile"
            elif tid == "mountains": ticon = "mountains/snow-tile"
            elif tid == "deep_water": ticon = "water/ocean-tile"
            elif tid == "flat": ticon = "grass/green-symbol"
            elif tid == "forest": ticon = "forest/pine-tile"
            elif tid == "frozen": ticon = "frozen/ice"
            elif tid == "village": ticon = "village/human-tile"
            elif tid == "impassable": ticon = "void/void"
            elif tid == "unwalkable": ticon = "unwalkable/lava"
            elif tid == "rails": ticon = "misc/rails-ne-sw"
            
            if ticon:
                terrainlist.append((name, tid, ticon))
            else:
                error_message("Terrain " + tid + " has no symbol_image\n")
        terrainlist.sort()

        for tname, tid, ticon in terrainlist:
            not_from_race, c = find_attr("movement_costs", tid)

            ccell = "td"
            if c == "99": ccell += ' class="grayed"'
            dcell = "td"
            not_from_race, d = find_attr("defense", tid)

            if d == "-": d = 100

            try:
                d = int(d)
                # negative defense has something to do with best defense if
                # there's multiple terrain types
                if d < 0: d = -d
                d = "%d%%" % (100 - d)
            except ValueError:
                error_message("Warning: Invalid defense %s for %s.\n" % (
                    d, uid))

            write("<tr>\n")
            picname = image_collector.add_image(self.addon,
                "terrain/" + ticon + ".png", no_tc=True)
            icon = os.path.join(pics_location, picname)
            write("<td><img src=\"%s\"  alt=\"(icon)\" /></td>\n" % (icon, ))
            write("<td>%s</td><%s><i>%s</i></td><%s class=\"num\"><i>%s</i></td>\n" % (
                tname, ccell, c, dcell, d))
            write("</tr>\n")
        write("</table>\n")

        write('</div>') # right column

        write('</div>') # columns parent

        self.output.write(html_clear_floats)
        write('</div>') # main

        self.output.write(html_footer % {
            "generation_note": "Last updated on " + time.ctime() + "."})


def generate_campaign_report(addon, isocode, campaign, wesnoth):
    if campaign:
        cid = campaign.get_text_val("id")
    else:
        cid = "mainline"
    if not cid: cid = addon + "_" + campaign.get_text_val("define")

    print(("campaign " + addon + " " + cid + " " + isocode))

    path = os.path.join(options.output, addon, isocode)
    if not os.path.isdir(path): os.mkdir(path)
    output = MyFile(os.path.join(path, "%s.html" % cid), "w")
    html = HTMLOutput(isocode, output, addon, cid, False, wesnoth)
    html.target = "%s.html" % cid
    grouper = GroupByRace(wesnoth, cid)

    if campaign:
        title = campaign.get_text_val("name", translation = html.translate)
    else:
        title = html.translate("Units", "wesnoth-help")
    if not title:
        title = cid

    n = html.write_units_tree(grouper, title, True)
    
    output.close()
    
    return n

def generate_era_report(addon, isocode, era, wesnoth):
    eid = era.get_text_val("id")
    
    print(("era " + addon + " " + eid + " " + isocode))

    path = os.path.join(options.output, addon, isocode)
    if not os.path.isdir(path): os.mkdir(path)
    
    output = MyFile(os.path.join(path, "%s.html" % eid), "w")
    html = HTMLOutput(isocode, output, addon, eid, True, wesnoth)
    html.target = "%s.html" % eid
    
    grouper = GroupByFaction(wesnoth, eid)

    ename = era.get_text_val("name", translation = html.translate)
    n = html.write_units_tree(grouper, ename, False)
    
    output.close()
    
    return n

def generate_single_unit_reports(addon, isocode, wesnoth):
    
    path = os.path.join(options.output, addon, isocode)
    if not os.path.isdir(path): os.mkdir(path)

    html = HTMLOutput(isocode, None, addon, "units", False, wesnoth)
    grouper = GroupByNothing()
    html.analyze_units(grouper, True)

    for uid, unit in list(wesnoth.unit_lookup.items()):
        if unit.hidden: continue
        if "mainline" in unit.campaigns and addon != "mainline": continue
        
        try:
            htmlname = "%s.html" % uid
            filename = os.path.join(path, htmlname)

            # We probably can come up with something better.
            if os.path.exists(filename):
                age = time.time() - os.path.getmtime(filename)
                # was modified in the last 12 hours - we should be ok
                if age < 3600 * 12: continue
        except (UnicodeDecodeError, UnicodeEncodeError) as e:
            traceback.print_exc()
            error_message("Unicode problem: " + repr(path) + " + " + repr(uid) + "\n")
            error_message(str(e) + "\n")
            continue
        
        output = MyFile(filename, "w")
        html.target = "%s.html" % uid
        html.write_unit_report(output, unit)
        output.close()
        
def html_postprocess_file(filename, isocode, batchlist):

    print(("postprocessing " + repr(filename)))
    
    chtml = ""
    ehtml = ""
    
    cids = [[], []]
    for addon in batchlist:
        for campaign in addon.get("campaigns", []):
            if campaign["units"] == "?": continue
            if campaign["units"] <= 0: continue
            if addon["name"] == "mainline": lang = isocode
            else: lang = "en_US"
            c = addon["name"], campaign["id"], campaign["translations"].get(
                lang, campaign["name"]), lang
            if addon["name"] == "mainline":
                cids[0].append(c)
            else:
                cids[1].append(c)

    for i in range(2):
        
        campaigns = cids[i]
        campaigns.sort(key = lambda x: "A" if x[1] == "mainline" else "B" + x[2])

        for campaign in campaigns:
            addon, cname, campname, lang = campaign

            chtml += '<li><a title="%s" href="../../%s/%s/%s.html" role="menuitem">%s</a></li>\n' % (
                campname, addon, lang, cname, campname)
        if i == 0 and cids[1]:
            chtml += '<li>%s</li>\n' % html_entity_horizontal_bar

    eids = [[], []]
    for addon in batchlist:
        for era in addon.get("eras", []):
            if era["units"] == "?": continue
            if era["units"] <= 0: continue
            if addon["name"] == "mainline": lang = isocode
            else: lang = "en_US"
            e = addon["name"], era["id"], era["translations"].get(
                lang, era["name"]), lang
            if addon["name"] == "mainline":
                eids[0].append(e)
            else:
                eids[1].append(e)
            
    for i in range(2):
        eras = eids[i]
        eras.sort(key = lambda x: x[2])

        for era in eras:
            addon, eid, eraname, lang = era

            ehtml += '<li><a title="%s" href="../../%s/%s/%s.html" role="menuitem">%s</a></li>\n' % (
                eraname, addon, lang, eid, eraname)
        if i == 0 and eids[1]:
            ehtml += '<li>%s</li>\n' % html_entity_horizontal_bar
            
    f = open(filename, "r+b")
    html = f.read().decode("utf8")
    html = html.replace("PLACE CAMPAIGNS HERE\n", chtml)
    html = html.replace("PLACE ERAS HERE\n", ehtml)
    f.seek(0)
    f.write(html.encode("utf8"))
    f.close()

def html_postprocess_all(batchlist):
    for isocode, filename in all_written_html_files:
        html_postprocess_file(filename, isocode, batchlist)

def write_index(out_path):
    output = MyFile(os.path.join(out_path, "index.html"), "w")
    output.write("""
    <html><head>
    <meta http-equiv="refresh" content="0;url=mainline/en_US/mainline.html">
    </head>
    <body>
    <a href="mainline/en_US/mainline.html">Redirecting to Wesnoth units database...</a>
    </body>
    </html>
    """)
