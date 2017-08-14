# encoding: utf-8

import copy
import gettext
import html
import os
import re
import time
import traceback
import urllib.parse
import unit_tree.helpers as helpers
import wesnoth.wmlparser3 as wmlparser3

PICS_LOCATION = "../../pics"

# Icons for mainline terrains used on the unit details page
TERRAIN_ICONS = {
    "fungus":        "forest/mushrooms-tile",
    "cave":          "cave/floor6",
    "sand":          "sand/beach",
    "reef":          "water/reef-tropical-tile",
    "hills":         "hills/regular",
    "swamp_water":   "swamp/water-tile",
    "shallow_water": "water/coast-tile",
    "castle":        "castle/castle-tile",
    "mountains":     "mountains/snow-tile",
    "deep_water":    "water/ocean-tile",
    "flat":          "grass/green-symbol",
    "forest":        "forest/pine-tile",
    "frozen":        "frozen/ice",
    "village":       "village/human-tile",
    "impassable":    "void/void",
    "unwalkable":    "unwalkable/lava",
    "rails":         "misc/rails-ne-sw",
}

# Omit these terrains from the terrain info report on the unit details page
HIDDEN_TERRAINS = [
    "off_map", "off_map2", "fog", "shroud", "impassable", "void", "rails"
]

# Damage tpye ids and associated icons used on the details page
RESISTANCES = [
    ("blade", "attacks/sword-human.png"),
    ("pierce", "attacks/spear.png"),
    ("impact", "attacks/club.png"),
    ("fire", "attacks/fireball.png"),
    ("cold", "attacks/iceball.png"),
    ("arcane", "attacks/faerie-fire.png")
]

WESMERE_CSS_VERSION = "1.1.0"
WESMERE_CSS_PREFIX = "https://www.wesnoth.org"

WESMERE_HEADER = '''\
<!DOCTYPE html>

<html class="no-js %(classes)s" lang="en">
<head>
	<meta charset="utf-8" />
	<meta name="viewport" content="width=device-width,initial-scale=1" />

	<link rel="stylesheet" href="https://fonts.googleapis.com/css?family=Montaga%%7COpen+Sans:400,400i,700,700i" type="text/css" />
	<link rel="icon" type="image/png" href="https://www.wesnoth.org/wesmere/img/favicon-32.png" sizes="32x32" />
	<link rel="icon" type="image/png" href="https://www.wesnoth.org/wesmere/img/favicon-16.png" sizes="16x16" />
	<link rel="stylesheet" type="text/css" href="%(cssprefix)s/wesmere/css/wesmere-%(cssver)s.css" />
	<link rel="stylesheet" type="text/css" href="%(cssprefix)s/wesmere/css/wmlunits-%(cssver)s.css" />
	<script src="https://www.wesnoth.org/wesmere/js/modernizr.js"></script>
	<script type="text/javascript" src="%(path)s/menu.js"></script>
	<title>%(title)s - Wesnoth Units Database</title>
</head>

<body>

<div id="main">

<div id="nav" role="banner">
<div class="centerbox">

	<div id="logo">
		<a href="https://www.wesnoth.org/" aria-label="Wesnoth logo"></a>
	</div>

	<ul id="navlinks">
		<li><a href="https://units.wesnoth.org/">Units</a></li>
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

<div id="content" role="main">'''

WESMERE_FOOTER = '''\
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
'''

HTML_CLEAR_FLOATS = '<div class="reset"></div>'

HTML_ENTITY_HORIZONTAL_BAR = '&#8213;'
HTML_ENTITY_MULTIPLICATION_SIGN = '&#215;'
HTML_ENTITY_FIGURE_DASH = '&#8210;'

PRE_PLACEHOLDER_CAMPAIGNS = "PLACE CAMPAIGNS HERE\n"
PRE_PLACEHOLDER_ERAS = "PLACE ERAS HERE\n"

def website_header(title='', path='../../', classes=[]):
    """Returns the website header with the specified parameters."""
    return WESMERE_HEADER % {
        "title":     title,
        "path":      path,
        "cssprefix": WESMERE_CSS_PREFIX,
        "cssver":    WESMERE_CSS_VERSION,
        "classes":   ' '.join(['wmlunits'] + classes)}

def website_footer(timestamp=''):
    """Returns the website footer, including the current timestamp."""
    if not timestamp:
        timestamp = "Last updated on " + time.ctime() + "."
    return WESMERE_FOOTER % { "generation_note": timestamp }


all_written_html_files = []

error_only_once = {}

def error_message(message):
    if message in error_only_once:
        return
    error_only_once[message] = 1
    write_error(message)


helpers.error_message = error_message

def reset_errors():
    error_only_once = {}

def int_fallback(str_value, int_fallback=0):
    try:
        return int(str_value)
    except TypeError:
        return int_fallback
    except ValueError:
        return int_fallback


def cleanurl(url):
    """
    Encode the given URL to ensure it only contains valid URL characters
    (also known as percent-encoding).
    """
    if url is None:
        return url
    return urllib.parse.quote(url, encoding='utf-8')

def cleantext(text, quote=True):
    """Escape any HTML special characters in the given string."""
    if text is None:
        return text
    return html.escape(text, quote)

def resistance_rating_color_class(resistance):
    """Return a color class adequate for the provided unit resistance value."""
    if resistance < 0:
        return 'red'
    elif resistance <= 20:
        return 'yellow'
    elif resistance <= 40:
        return 'olive'
    else:
        return 'green'

def defense_rating_color_class(defense):
    """Return a color class adequate for the provided terrain defense value."""
    if defense <= 10:
        return 'red'
    elif defense <= 30:
        return 'yellow'
    elif defense <= 50:
        return 'olive'
    else:
        return 'green'

def mvtcost_rating_color_class(str_mvtcost, str_moves):
    """Return a color class adequate for the provided movement cost value."""
    cost = int_fallback(str_mvtcost, 99)
    moves = int_fallback(str_moves)
    if cost >= moves:
        return 'gray'
    elif cost > moves/2:
        return 'red'
    elif cost > 1:
        return 'yellow'
    else:
        return 'green'


class MyFile:
    """
    Python 2 is a bit weird with encodings, really should switch this to
    Python 3.
    """
    def __init__(self, filename, mode):
        self.filename = filename
        self.fileobj = open(filename, mode + "b")
    def write(self, line):
        self.fileobj.write(line.encode("utf8"))
    def close(self):
        self.fileobj.close()


class Translation:
    def __init__(self, localedir, langcode):
        self.catalog = {}
        self.localedir = localedir
        self.langcode = langcode
        class Dummy:
            def gettext(self, msgid):
                if not msgid:
                    return ""
                caret = msgid.find("^")
                if caret < 0:
                    return msgid
                return msgid[caret + 1:]
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
        return self.catalog[textdomain].gettext(string)


class GroupByRace:
    def __init__(self, wesnoth, campaign):
        self.wesnoth = wesnoth
        self.campaign = campaign

    def unitfilter(self, unit):
        if not self.campaign:
            return True
        return unit.campaigns and self.campaign == unit.campaigns[0]

    def groups(self, unit):
        return [T(unit.race, "plural_name")]

    def group_name(self, group):
        if not group:
            return "None"
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
    if not tag:
        return "none"
    return tag.get_text_val(att, translation=global_htmlout.translate)


class HTMLOutput:
    def __init__(self, isocode, output, addon, campaign, is_era, wesnoth, verbose=False):
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
            if u.hidden:
                continue
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
                    if uid in forest.lookup:
                        continue
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
        thelist.sort(key=lambda x: grouper.group_name(x))

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
            return "" if x is None else x

        def grid_place(nodes, x):
            nodes.sort(key=by_name)
            for node in nodes:
                level = max(0, min(5, node.unit.level))
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
        def write(line):
            self.output.write(line)
        def _(msgid, textdomain="wesnoth"):
            return self.translate(msgid, textdomain)

        all_written_html_files.append((self.isocode, self.output.filename))

        languages = self.wesnoth.languages_found
        langlist = list(languages.keys())
        langlist.sort()

        write('<div class="navbar">')
        write('<ul class="navbar" role="menu">')

        def abbrev(name):
            abbrev = name[0]
            word_separators = [" ", "_", "+", "(", ")"]
            for i in range(1, len(name)):
                if name[i] in ["+", "(", ")"] or \
                    name[i - 1] in word_separators and \
                    name[i] not in word_separators:
                    abbrev += name[i]
            return abbrev

        def add_menu(menuid, name, classes='', is_table_container=False):
            """
            Writes the initial portion of a sidebar item, including the item's
            label, the start tag for the popup menu container, and the label
            for said menu.

            If is_table_container=True, the popup menu prolog is suitable for
            a table (currently used for the Language menu). This will be
            removed one day, hopefully. (TODO)
            """
            html_name = cleantext(name)
            html_classes = " ".join((cleantext(classes), "popuptrigger"))
            write('<li class="popupcontainer" role="menuitem" aria-haspopup="true">')
            write('<a class="' + html_classes + '" href="#">' + html_name + '</a>')
            write('<div id="%s" class="popupmenu" role="menu" aria-label="%s">' %
                  (menuid, html_name))
            write('<div class="popupheader">' + html_name + '</div>')
            if not is_table_container:
                write('<ul>')

        def add_menuitem_placeholder():
            """Writes a horizontal bar serving as a menu placeholder."""
            write('<li>' + HTML_ENTITY_HORIZONTAL_BAR + '</li>')

        def add_menuitem(url, label, standalone=False, title=''):
            """
            Writes a sidebar item.

            If standalone=True, the item will be provided without the list
            element tags so it can be used in pretty much any context. In
            reality, the option is only provided for use with add_menu() with
            is_table_container=True and it will be removed at some point in
            the hopefully not-so-far future (TODO).
            """
            if not standalone:
                write('<li>')
            extra_attr = (' title="%s"' % cleantext(title)) if title else ''
            write('<a href="%s" role="menuitem"%s>%s</a>' %
                  (cleantext(url), extra_attr, cleantext(label, quote=False)))
            if not standalone:
                write('</li>')

        def end_menu(is_table_container=False):
            """
            Writes the closing tags for a menu started with start_menu().
            The is_table_container value used here ought to be the same as the
            original.
            """
            if not is_table_container:
                write('</ul>')
            write('</div></li>')

        # We may not have all the required info yet so defer writing the
        # campaigns/eras navigation.

        # Campaigns
        add_menu("campaigns_menu", _("addon_type^Campaign"))
        write(PRE_PLACEHOLDER_CAMPAIGNS)
        end_menu()

        # Eras
        add_menu("eras_menu", _("Era"))
        write(PRE_PLACEHOLDER_ERAS)
        end_menu()

        # Races / Factions

        target = self.target
        if self.campaign == "units":
            target = "mainline.html"

        if not self.is_era:
            add_menu("races_menu", _("Race", "wesnoth-lib"))

            add_menuitem('mainline.html', _("all", "wesnoth-editor"))

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
                    add_menuitem_placeholder()
                else:
                    url = cleanurl(target) + '#' + cleanurl(racename)
                    add_menuitem(url, racename)
            end_menu()
        else:
            add_menu("races_menu", _("Factions", "wesnoth-help"))
            for row in self.unitgrid:
                for column in range(6):
                    hspan, vspan, un = row[column]
                    if not un:
                        continue
                    if isinstance(un, helpers.GroupNode):
                        url = cleanurl('../%s/%s.html' % (self.isocode, self.campaign))
                        url += '#' + cleanurl(un.name)
                        add_menuitem(url, un.name)
            end_menu()

        # Add entries for the races also to the navbar itself.
        if not self.is_era:
            races = {}

            for uid, u in list(self.wesnoth.unit_lookup.items()):
                if self.campaign != "units" and self.campaign not in u.campaigns:
                    continue
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
                if c == "units":
                    c = "mainline"
                add_menuitem('%s#%s' % (cleanurl(target), cleanurl(r)), r)
                for uid in races[r]:
                    un = self.wesnoth.unit_lookup[uid]
                    if un.hidden:
                        continue
                    if "mainline" in un.campaigns:
                        addon = "mainline"
                    else:
                        addon = self.addon
                    link = cleanurl("../../%s/%s/%s.html" % (addon, self.isocode, uid))
                    name = self.wesnoth.get_unit_value(un, "name",
                                                       translation=self.translation.translate)
                    if not name:
                        error_message("Warning: Unit uid=" + uid + " has no name.\n")
                        name = uid
                    add_menuitem(link, name)
            if got_menu:
                end_menu()

        # Languages
        add_menu("languages_menu", _("Language"), is_table_container=True)
        cell = 0
        col = 0
        colcount = 5
        lastcell = len(langlist) - 1
        write('<table>')
        write('<tr>')
        for lang in langlist:
            cell += 1
            col += 1
            write('<td>')
            filename = self.target if self.addon == 'mainline' else 'mainline.html'
            url = cleanurl('../%s/%s' % (lang, filename))
            # TODO: Maybe use the language name instead of its code for the label?
            add_menuitem(url, lang, title=languages[lang], standalone=True)
            write('</td>')
            if cell % colcount == 0:
                if cell < lastcell:
                    write('</tr><tr>')
                col = 0
        if col:
            for i in range(colcount - col + 1, colcount):
                write('<td></td>')
        write('</tr>')
        write('</table>')
        end_menu(is_table_container=True)

        write('<li class="overviewlink"><a href="../../overview.html">Build Report</a></li>')

        write('</ul></div>\n')

        write('<script>\nwmlunits_menu_setup();\n</script>');

    def pic(self, u, x, recursion=0):
        if recursion >= 4:
            error_message(
                "Warning: Cannot find image for unit %s(%s).\n" % (
                    u.get_text_val("id"), x.name.decode("utf8")))
            return None, None
        image = self.wesnoth.get_unit_value(x, "image")
        portrait = self.wesnoth.get_unit_value(x, "profile")
        if not portrait:
            bu = self.wesnoth.get_base_unit(u)
            if bu:
                portrait = self.wesnoth.get_unit_value(bu, "profile")
        if not image:
            if x.name == b"female":
                baseunit = self.wesnoth.get_base_unit(u)
                if baseunit:
                    female = baseunit.get_all(tag="female")
                    return self.pic(u, female[0], recursion=recursion + 1)
                else:
                    return self.pic(u, u, recursion=recursion + 1)
            error_message("Warning: Missing image for unit %s(%s).\n" %
                          (u.get_text_val("id"), x.name.decode("utf8")))
            return None, None
        icpic = image_collector.add_image_check(self.addon, image)
        if not icpic.ipath:
            error_message("Warning: No picture %s for unit %s.\n" %
                          (image, u.get_text_val("id")))
        picname = icpic.id_name
        image = os.path.join(PICS_LOCATION, picname)
        if portrait:
            picname = image_collector.add_image(self.addon, portrait, no_tc=True)
            portrait = os.path.join(PICS_LOCATION, picname)
        return image, portrait

    def get_abilities(self, u):
        anames = []
        already = {}
        for abilities in u.get_all(tag="abilities"):
            try:
                c = abilities.get_all()
            except AttributeError:
                c = []
            for ability in c:
                try:
                    id = ability.get_text_val("id")
                except AttributeError as e:
                    error_message("Error: Ignoring ability " + ability.debug())
                    continue
                if id in already:
                    continue
                already[id] = True
                name = T(ability, "name")
                if not name:
                    name = id
                if not name:
                    name = ability.name.decode("utf8")
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
        def write(line):
            self.output.write(line)
        def _(msgid, textdomain="wesnoth"):
            return self.translate(msgid, textdomain)

        rows = self.unitgrid
        write('<table class="units">\n<colgroup>')
        for i in range(6):
            write('<col class="col%d" />' % i)
        write('</colgroup>')

        pic = image_collector.add_image("general",
                                        "../../../images/misc/leader-crown.png",
                                        no_tc=True)
        crownimage = cleanurl(os.path.join(PICS_LOCATION, pic))
        ms = None
        for row in range(len(rows)):
            write('<tr>\n')
            for column in range(6):
                hspan, vspan, un = rows[row][column]
                if vspan:
                    attributes = ""
                    if hspan == 1 and vspan == 1:
                        pass
                    elif hspan == 1:
                        attributes += ' rowspan="%d"' % vspan
                    elif vspan == 1:
                        attributes += ' colspan="%d"' % hspan

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
                        attributes += ' id="%s" class="raceheader"' % cleantext(racename)
                        write('<th' + attributes + '>')
                        write('<a href="#%s">%s</a>' % (cleanurl(racename), cleantext(racename, quote=False)))
                        write('</th>\n')
                    elif un:
                        u = un.unit
                        attributes += ' class="unitcell"'
                        write('<td%s>' % attributes)

                        def uval(name):
                            return self.wesnoth.get_unit_value(u, name,
                                                               translation=self.translation.translate)

                        def clean_uval(name):
                            return cleantext(uval(name))

                        uid = cleantext(u.get_text_val("id"))
                        name = clean_uval("name")
                        cost = clean_uval("cost")
                        hp = clean_uval("hitpoints")
                        mp = clean_uval("movement")
                        xp = clean_uval("experience")
                        level = clean_uval("level")

                        crown = ""
                        if ms:
                            if un.id in ms.units:
                                crown = " ♟"
                            if un.id in ms.is_leader:
                                crown = " ♚"

                        uaddon = "mainline"
                        if "mainline" not in u.campaigns:
                            uaddon = self.addon
                        link = cleanurl("../../%s/%s/%s.html" % (uaddon, self.isocode, uid))
                        write('<div class="l">L%s%s</div>' % (level, crown))
                        write('<a href="%s" title="Id: %s">%s</a><br />' % (link, uid, name))

                        write('<div class="pic">')
                        image, portrait = self.pic(u, u)
                        image = cleanurl(image)

                        write('<a href="%s" title="Id: %s">' % (link, name))

                        if crown == " ♚":
                            write('<div class="spritebg" style="background-image:url(\'%s\')">' % image)
                            write('<img src="%s" alt="(image)" />' % crownimage)
                            write('</div>')
                        else:
                            write('<img src="%s" alt="(image)" />' % image)

                        write('</a>\n</div>\n')

                        write('<div class="attributes">')

                        write('<table><colgroup><col class="attribute-label"><col class="attribute-value">')
                        attributes = (
                            (_("Cost: ", "wesnoth-help"), cost),
                            (_("HP: "), hp),
                            (_("XP: "), xp),
                            (_("MP: "), mp),
                        )
                        for attr_label, attr_value in attributes:
                            write('<tr><th>%s</th><td>%s</td></tr>' % (cleantext(attr_label.strip(), quote=False), attr_value))
                        write('</table>')

                        # Write info about abilities.
                        anames = self.get_abilities(u)
                        if anames:
                            write('\n<div class="abilities">')
                            write(cleantext(", ".join(anames)))
                            write('</div>')

                        # Write info about attacks.
                        write('\n<div class="attacks">')
                        attacks = self.get_recursive_attacks(u)
                        for attack in attacks:
                            n = T(attack, "number")
                            x = T(attack, "damage")
                            x = "%s %s %s " % (cleantext(x, quote=False), HTML_ENTITY_MULTIPLICATION_SIGN, cleantext(n, quote=False))
                            write(x)

                            r = T(attack, "range")
                            t = T(attack, "type")
                            write(cleantext("%s (%s)" % (_(r), _(t)), quote=False))

                            s = []
                            specials = attack.get_all(tag="specials")
                            if specials:
                                for special in specials[0].get_all(tag=""):
                                    sname = T(special, "name")
                                    if sname:
                                        s.append(sname)
                                s = ", ".join(s)
                                if s:
                                    write(" (%s)" % cleantext(s, quote=False))
                            write('<br />')
                        write('</div>')

                        write('</div>')
                        write('</td>\n')
                    else:
                        write('<td class="empty"></td>')
            write('</tr>\n')
        write('</table>\n')

    def write_units_tree(self, grouper, title, add_parents):
        html_title = cleantext(title)

        self.output.write(website_header(title=html_title,
                                         classes=['wmlunits-tree']))

        n = self.analyze_units(grouper, add_parents)
        self.write_navbar("units_tree")

        self.output.write('<div class="main">')

        self.output.write('<h1>%s</h1>' % html_title)

        self.write_units()

        self.output.write(HTML_CLEAR_FLOATS)
        self.output.write('</div>')

        self.output.write(website_footer())

        return n

    def write_unit_report(self, output, unit):
        def write(line):
            self.output.write(line)
        def _(msgid, textdomain="wesnoth"):
            return self.translate(msgid, textdomain)

        def find_attr(what, key):
            if unit.movetype:
                mtx = unit.movetype.get_all(tag=what)
                mty = None
                if mtx:
                    mty = mtx[0].get_text_val(key)
            x = unit.get_all(tag=what)
            y = None
            if x:
                y = x[0].get_text_val(key, translation=self.translation.translate)
            if y:
                return True, y
            if unit.movetype and mty is not None:
                return False, mty
            return False, "-"

        def uval(name):
            return self.wesnoth.get_unit_value(unit, name, translation=self.translation.translate)

        def clean_uval(name):
            return cleantext(uval(name))

        # Write unit name, picture and description.
        uid = unit.get_text_val("id")
        uname = uval("name")
        display_name = cleantext(uname)

        self.output = output
        write(website_header(title=display_name,
                             classes=['wmlunits-unit']))
        self.write_navbar("unit_report")

        self.output.write('<div class="main">')

        female = unit.get_all(tag="female")
        if female:
            fname = T(female[0], "name")
            if fname and fname != uname:
                display_name += " / " + cleantext(fname)

        write('<div class="unit-columns">')

        write('<div class="unit-column-left">')

        write('<h1>%s</h1>\n' % display_name)

        write('<div class="pic">')
        if female:
            mimage, portrait = self.pic(unit, unit)
            fimage, fportrait = self.pic(unit, female[0])
            if not fimage:
                fimage = mimage
            if not fportrait:
                fportrait = portrait
            write('<img src="%s" alt="(image)" />\n' % cleanurl(mimage))
            write('<img src="%s" alt="(image)" />\n' % cleanurl(fimage))
            image = mimage
        else:
            image, portrait = self.pic(unit, unit)
            write('<img src="%s" alt="(image)" />\n' % cleanurl(image))
        write('</div>\n')

        description = clean_uval("description")

        # TODO: what is unit_description?
        if not description:
            description = clean_uval("unit_description")
        if not description:
            description = HTML_ENTITY_HORIZONTAL_BAR
        write('<p>%s</p>\n' % re.sub('\n', '\n<br />', description))

        write('<h2>Information</h2>\n')
        write('<table class="unitinfo">\n')

        # Advances-from list
        write('<tr><th>%s</th><td>' % cleantext(_("Advances from: ", "wesnoth-help"), quote=False))
        have_advances = False
        for pid in self.forest.get_parents(uid):
            punit = self.wesnoth.unit_lookup[pid]
            if "mainline" in unit.campaigns and "mainline" not in punit.campaigns:
                continue
            addon = "mainline" if "mainline" in unit.campaigns else self.addon
            link = cleanurl("../../%s/%s/%s.html" % (addon, self.isocode, pid))
            name = self.wesnoth.get_unit_value(punit, "name",
                                               translation=self.translation.translate)
            if have_advances:
                write(', ')
            write('<a href="%s">%s</a>' % (link, cleantext(name, quote=False)))
            have_advances = True
        if not have_advances:
            write(HTML_ENTITY_FIGURE_DASH)
        write('</td></tr>\n')

        # Advances-to list
        write('<tr><th>%s</th><td>' % cleantext(_("Advances to: ", "wesnoth-help"), quote=False))
        have_advances = False
        for cid in self.forest.get_children(uid):
            try:
                cunit = self.wesnoth.unit_lookup[cid]
                addon = "mainline" if "mainline" in cunit.campaigns else self.addon
                link = cleanurl("../../%s/%s/%s.html" % (addon, self.isocode, cid))
                if "mainline" in unit.campaigns and "mainline" not in cunit.campaigns:
                    continue
                name = self.wesnoth.get_unit_value(cunit, "name",
                                                   translation=self.translation.translate)
            except KeyError:
                error_message("Warning: Unit %s not found.\n" % cid)
                name = cid
                if "mainline" in unit.campaigns:
                    continue
                link = cleanurl(self.target)
            if have_advances:
                write(', ')
            write('<a href="%s">%s</a>' % (link, cleantext(name, quote=False)))
            have_advances = True
        if not have_advances:
            write(HTML_ENTITY_FIGURE_DASH)
        write('</td></tr>\n')

        attributes = [
            ("cost",       _("Cost: ", "wesnoth-help")),
            ("hitpoints",  _("HP: ")),
            ("movement",   _("Movement", "wesnoth-help") + ": "),
            ("vision",     _("Vision", "wesnoth-help") + ": "),
            ("jamming",    _("Jamming", "wesnoth-help") + ":"),
            ("experience", _("XP: ")),
            ("level",      _("Level") + ": "),
            ("alignment",  _("Alignment: ")),
            ("id",         "Id: ")
        ]

        for attr, label in attributes:
            value = uval(attr)
            if not value and attr in ("jamming", "vision"):
                continue
            if attr == "alignment":
                value = _(value)
            write('<tr><th>%s</th><td class="val">%s</td></tr>\n' % (cleantext(label, quote=False), cleantext(value, quote=False)))

        # Write info about abilities.
        anames = self.get_abilities(unit)

        write('<tr>\n')
        write('<th>%s</th>' % cleantext(_("Abilities: ", "wesnoth-help"), quote=False))
        if len(anames):
            write('<td class="val">' + cleantext(', '.join(anames), quote=False) + '</td>')
        else:
            write('<td class="val">' + HTML_ENTITY_FIGURE_DASH + '</td>')
        write('</tr>\n')

        write('</table>\n')

        # Write info about attacks.
        write('<h2>%s <small>(damage %s count)</small></h2>\n' %
              (cleantext(_("unit help^Attacks", "wesnoth-help"), quote=False),
               HTML_ENTITY_MULTIPLICATION_SIGN))
        write('<table class="unitinfo attacks">\n')
        write('<colgroup><col class="col0" /><col class="col1" /><col class="col2" /><col class="col3" /></colgroup>')
        attacks = self.get_recursive_attacks(unit)
        for attack in attacks:
            write('<tr>')

            aid = attack.get_text_val("name")
            aname = T(attack, "description")

            icon = attack.get_text_val("icon")
            if not icon:
                icon = "attacks/%s.png" % aid

            image_add = image_collector.add_image_check(self.addon, icon, no_tc=True)
            if not image_add.ipath:
                error_message("Error: No attack icon '%s' found for '%s'.\n" % (
                    icon, uid))
                icon = os.path.join(PICS_LOCATION, "unit$elves-wood$shaman.png")
            else:
                icon = os.path.join(PICS_LOCATION, image_add.id_name)
            write('<td><img src="%s" alt="(image)"/></td>' % cleanurl(icon))

            write('<td><b>%s</b>' % cleantext(aname, quote=False))

            r = T(attack, "range")
            write('<br/>%s</td>' % cleantext(_(r), quote=False))

            n = attack.get_text_val("number")
            x = attack.get_text_val("damage")
            x = '%s %s %s' % (cleantext(x, quote=False), HTML_ENTITY_MULTIPLICATION_SIGN, cleantext(n, quote=False))
            write('<td><i>%s</i>' % x)

            t = T(attack, "type")
            write('<br/>%s</td>' % cleantext(_(t), quote=False))

            s = []
            specials = attack.get_all(tag="specials")
            if specials:
                for special in specials[0].get_all(tag=""):
                    sname = T(special, "name")
                    if sname:
                        s.append(cleantext(sname, quote=False))
                    else:
                        error_message("Warning: Weapon special %s has no name for %s.\n" %
                                      (special.name.decode("utf8"), uid))
            write('<td>%s</td>' % '<br/>'.join(s))
            write('</tr>')
        write('</table>\n')

        # Write info about resistances.
        write('<h2>%s</h2>\n' % _("Resistances: ").strip(" :"))
        write('<table class="unitinfo resistances">\n')
        write('<colgroup><col class="col0" /><col class="col1" /><col class="col2" /><col class="col3" /><col class="col4" /><col class="col5" /><col class="col6" /></colgroup>')
        row = 0
        for rid, ricon in RESISTANCES:
            special, resist_str = find_attr("resistance", rid)
            r = 100 if resist_str == '-' else 100 - int(resist_str)
            resist_classes = ['num']
            resist_rating = resistance_rating_color_class(r)
            if resist_rating:
                resist_classes.append('rating-' + resist_rating)
            try:
                resist_str = '<i>%d%%</i>' % r
            except ValueError:
                error_message("Warning: Invalid resistance %s for %s.\n" % (
                    r, uid))
            rcell = "td"
            if special:
                rcell += ' class="special"'
            if row % 2 == 0:
                write('<tr>\n')
            else:
                write('<td></td>')
            picname = image_collector.add_image(self.addon, ricon, no_tc=True)
            icon = os.path.join(PICS_LOCATION, picname)
            write('<td><img src="%s" alt="(icon)" /></td>\n' % (icon, ))
            write('<th>%s</th><td class="%s">%s</td>\n' % (cleantext(_(rid), quote=False), ' '.join(resist_classes), resist_str))
            if row % 2 == 1:
                write('</tr>\n')
            row += 1
        write('</table>\n')

        # end left column
        write('</div>')
        write('<div class="unit-column-right">')

        for si in range(2):
            if si and not female:
                break
            if si:
                sportrait = fportrait
                simage = fimage
            else:
                simage = image
                sportrait = portrait

            style = "background-image:url('%s');" % cleanurl(simage)

            write('<div class="portrait">')
            write('<div style="%s">&nbsp;</div>' % style)
            if portrait:
                write('<img src="%s" alt="(portrait)" />\n' % cleanurl(sportrait))
            write('</div>')

        # Write info about movement costs and terrain defense.
        write('<h2>' + cleantext(_("Terrain", "wesnoth-help"), quote=False) + '</h2>\n')
        write('<table class="unitinfo terrain">\n')
        write('<colgroup><col class="col0" /><col class="col1" /><col class="col2" /><col class="col3" /></colgroup>')

        write('<thead>')
        write('<tr><th colspan="2"><span class="sr-label">%s</span></th><th class="mvtcost">%s</th><th class="numheader">%s</th></tr>\n' % (
            cleantext(_("Terrain", "wesnoth-help"), quote=False),
            cleantext(_("Movement Cost", "wesnoth-help"), quote=False),
            cleantext(_("Defense", "wesnoth-help"), quote=False)))
        write('</thead>')

        terrains = self.wesnoth.terrain_lookup
        terrainlist = []
        already = {}
        for tstring, t in list(terrains.items()):
            tid = t.get_text_val("id")
            if tid in HIDDEN_TERRAINS or t.get_all(att="aliasof") or tid in already:
                continue
            already[tid] = 1
            name = T(t, "name")
            ticon = t.get_text_val("symbol_image")
            if not ticon:
                ticon = t.get_text_val("icon_image")
            # Use nice images for known mainline terrain types
            if tid in TERRAIN_ICONS:
                ticon = TERRAIN_ICONS[tid]
            if ticon:
                terrainlist.append((name, tid, ticon))
            else:
                error_message("Terrain " + tid + " has no symbol_image\n")
        terrainlist.sort()

        for tname, tid, ticon in terrainlist:
            not_from_race, move_cost = find_attr("movement_costs", tid)
            classes_cost = ['mvtcost']
            cost_rating = ''

            not_from_race, defense = find_attr("defense", tid)
            classes_defense = ['num']
            defense_rating = ''

            if defense == '-':
                defense = 100

            cost_rating = mvtcost_rating_color_class(move_cost, uval('movement'))

            try:
                defense = int(defense)
                # negative defense has something to do with best defense if
                # there's multiple terrain types
                if defense < 0:
                    defense = -defense
                defense_rating = defense_rating_color_class(100 - defense)
                defense = "%d%%" % (100 - defense)
            except ValueError:
                error_message("Warning: Invalid defense %s for %s.\n" % (
                    defense, uid))

            if cost_rating:
                classes_cost.append('rating-' + cost_rating)
            if defense_rating:
                classes_defense.append('rating-' + defense_rating)
            if move_cost == '-':
                move_cost = HTML_ENTITY_FIGURE_DASH
            else:
                move_cost = cleantext(move_cost, quote=False)

            write('<tr>\n')
            picname = image_collector.add_image(self.addon,
                                                "terrain/" + ticon + ".png",
                                                no_tc=True)
            icon = os.path.join(PICS_LOCATION, picname)
            write('<td><img src="%s" alt="(icon)" /></td>\n' % cleanurl(icon))
            write('<td>%s</td><td class="%s"><i>%s</i></td><td class="%s"><i>%s</i></td>\n' %
                  (cleantext(tname, quote=False),
                   ' '.join(classes_cost), move_cost,
                   ' '.join(classes_defense), defense))
            write('</tr>\n')
        write('</table>\n')

        write('</div>') # right column

        write('</div>') # columns parent

        self.output.write(HTML_CLEAR_FLOATS)
        write('</div>') # main

        self.output.write(website_footer())


def generate_campaign_report(addon, isocode, campaign, wesnoth):
    if campaign:
        cid = campaign.get_text_val("id")
    else:
        cid = "mainline"
    if not cid:
        cid = addon + "_" + campaign.get_text_val("define")

    print(("campaign " + addon + " " + cid + " " + isocode))

    path = os.path.join(options.output, addon, isocode)
    if not os.path.isdir(path):
        os.mkdir(path)
    output = MyFile(os.path.join(path, "%s.html" % cid), "w")
    html = HTMLOutput(isocode, output, addon, cid, False, wesnoth)
    html.target = "%s.html" % cid
    grouper = GroupByRace(wesnoth, cid)

    if campaign:
        title = campaign.get_text_val("name", translation=html.translate)
    else:
        title = html.translate("Units", "wesnoth-help")
    if not title:
        title = cid

    n = html.write_units_tree(grouper, title, True)

    output.close()

    return n

def generate_era_report(addon, isocode, era, wesnoth):
    eid = era.get_text_val("id")

    print("era " + addon + " " + eid + " " + isocode)

    path = os.path.join(options.output, addon, isocode)
    if not os.path.isdir(path):
        os.mkdir(path)

    output = MyFile(os.path.join(path, "%s.html" % eid), "w")
    html = HTMLOutput(isocode, output, addon, eid, True, wesnoth)
    html.target = "%s.html" % eid

    grouper = GroupByFaction(wesnoth, eid)

    ename = era.get_text_val("name", translation=html.translate)
    n = html.write_units_tree(grouper, ename, False)

    output.close()

    return n

def generate_single_unit_reports(addon, isocode, wesnoth):

    path = os.path.join(options.output, addon, isocode)
    if not os.path.isdir(path):
        os.mkdir(path)

    html = HTMLOutput(isocode, None, addon, "units", False, wesnoth)
    grouper = GroupByNothing()
    html.analyze_units(grouper, True)

    for uid, unit in list(wesnoth.unit_lookup.items()):
        if unit.hidden:
            continue
        if "mainline" in unit.campaigns and addon != "mainline":
            continue

        try:
            htmlname = "%s.html" % uid
            filename = os.path.join(path, htmlname)

            # We probably can come up with something better.
            if os.path.exists(filename):
                age = time.time() - os.path.getmtime(filename)
                # was modified in the last 12 hours - we should be ok
                if age < 3600 * 12:
                    continue
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
    print("postprocessing " + repr(filename))
    chtml = ""
    ehtml = ""

    cids = [[], []]
    for addon in batchlist:
        for campaign in addon.get("campaigns", []):
            if campaign["units"] == "?" or campaign["units"] <= 0:
                continue
            lang = isocode if addon["name"] == "mainline" else "en_US"
            c = addon["name"], campaign["id"], campaign["translations"].get(
                lang, campaign["name"]), lang
            if addon["name"] == "mainline":
                cids[0].append(c)
            else:
                cids[1].append(c)
    for i in range(2):
        campaigns = cids[i]
        campaigns.sort(key=lambda x: "A" if x[1] == "mainline" else "B" + x[2])

        for campaign in campaigns:
            addon, cname, campname, lang = campaign
            url = cleanurl("../../%s/%s/%s.html" % (addon, lang, cname))
            chtml += '<li><a title="%s" href="%s" role="menuitem">%s</a></li>\n' % (
                cleantext(campname), url, cleantext(campname, quote=False))
        if i == 0 and cids[1]:
            chtml += '<li>%s</li>\n' % HTML_ENTITY_HORIZONTAL_BAR

    eids = [[], []]
    for addon in batchlist:
        for era in addon.get("eras", []):
            if era["units"] == "?" or era["units"] <= 0:
                continue
            lang = isocode if addon["name"] == "mainline" else "en_US"
            e = addon["name"], era["id"], era["translations"].get(
                lang, era["name"]), lang
            if addon["name"] == "mainline":
                eids[0].append(e)
            else:
                eids[1].append(e)
    for i in range(2):
        eras = eids[i]
        eras.sort(key=lambda x: x[2])

        for era in eras:
            addon, eid, eraname, lang = era
            url = cleanurl("../../%s/%s/%s.html" % (addon, lang, eid))
            ehtml += '<li><a title="%s" href="%s" role="menuitem">%s</a></li>\n' % (
                cleantext(eraname), url, cleantext(eraname, quote=False))
        if i == 0 and eids[1]:
            ehtml += '<li>%s</li>\n' % HTML_ENTITY_HORIZONTAL_BAR

    f = open(filename, "r+b")
    html = f.read().decode("utf8")
    html = html.replace(PRE_PLACEHOLDER_CAMPAIGNS, chtml)
    html = html.replace(PRE_PLACEHOLDER_ERAS, ehtml)
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
