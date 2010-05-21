"""
Various helpers for use by the wmlunits tool.
"""
import sys, os, re, glob, shutil, copy, urllib2
from subprocess import Popen

import wesnoth.wmldata as wmldata
import wesnoth.wmlparser as wmlparser
import wesnoth.wmltools as wmltools

class ParserWithCoreMacros:
    """
    A wrapper around the WML parser to do some things like we want.
    """
    def __init__(self, isocode, datadir, userdir, transdir):
        self.datadir = datadir
        self.userdir = userdir
        # Handle translations.
        self.translations = wmltools.Translations(transdir)
        def gettext(textdomain, x):
            return self.translations.get(textdomain, isocode, x, x)
        self.gettext = gettext

        # Create a new parser for the macros.
        parser = wmlparser.Parser(datadir)
        parser.gettext = self.gettext
        
        # Parse core macros.
        parser.parse_text("{core/macros/}\n")
        parser.parse_top(None)
        self.core_macros = parser.macros
                
    def parse(self, text_to_parse, ignore_macros = None,
        ignore_fatal_errors = False, verbose = False):

        # Create the real parser.
        parser = wmlparser.Parser(self.datadir, self.userdir)
        parser.verbose = verbose
        parser.gettext = self.gettext
        parser.macros = copy.copy(self.core_macros)
        
        #parser.verbose = True
        
        # Suppress complaints about undefined terrain macros
        parser.set_macro_not_found_callback(lambda wmlparser, name, params:
        name.startswith("TERRAIN") or name == "DISABLE_TRANSITIONS")
        
        if ignore_macros:
            parser.macro_callback = ignore_macros

        # Create a WML root element and parse the given text into it.
        WML = wmldata.DataSub("WML")

        parser.parse_text(text_to_parse)

        parser.parse_top(WML)
 
        return WML

class ImageCollector:
    """
    A class to collect all the images which need to be copied to the HTML
    output folder.
    """
    def __init__(self, datadir, userdir):
        self.images = {}
        self.pathes_per_campaign = {}
        self.ipathes = {}
        self.notfound = {}
        self.datadir = datadir
        self.userdir = userdir
        self.id = 0
        self.verbose = 0

    def add_binary_pathes_from_WML(self, campaign, WML):
        self.pathes_per_campaign[campaign] = self.pathes_per_campaign.get(
            campaign, [])
        for binpath in WML.get_all("binary_path"):
            path = binpath.get_text_val("path")
            self.pathes_per_campaign[campaign].append(path)

    def find_image(self, i, c):
        if c == "mainline":
            bases = [os.path.join(self.datadir, "core/images")]
        else:
            bases = [os.path.join(self.datadir, "core/images")]
            binpaths = self.pathes_per_campaign.get(c, [])
            binpaths.reverse()
            for x in binpaths:
                if x.startswith("data/"):
                    for idir in ["images", "images/units"]:
                        bases.append(os.path.join(self.datadir, x[5:], idir))
                        if self.userdir:
                            bases.append(os.path.join(self.userdir, x[5:], idir))
        for base in bases:
            ipath = os.path.join("%s" % base, i)
            if os.path.exists(ipath): return ipath, None
        return None, bases

    def add_image_check(self, campaign, path):
        if (campaign, path) in self.notfound:
            return self.notfound[(campaign, path)], True
        ipath, error = self.find_image(path, campaign)
        if ipath in self.ipathes:
            return self.ipathes[ipath], False
    
        name = "%05d_" % self.id
        name += os.path.basename(path)
        self.id += 1

        self.images[name] = ipath, path, campaign, error
        if ipath:
            self.ipathes[ipath] = name
            return name, False
        else:
            self.notfound[(campaign, path)] = name
            return name, True
        
    def add_image(self, campaign, path):
        name, error = self.add_image_check(campaign, path)
        return name

    def copy_and_color_images(self, target_path):
        for iid in self.images.keys():
            opath = os.path.join(target_path, "pics", iid)
            try:
                os.makedirs(os.path.dirname(opath))
            except OSError:
                pass

            ipath, i, c, bases = self.images[iid]
            if ipath and os.path.exists(ipath):
                #shutil.copy2(ipath, opath)
                # We assume TeamColorizer is in the same directory as the
                # helpers.py currently executing.
                command = os.path.join(os.path.dirname(__file__),
                    "TeamColorizer")
                p = Popen([command, ipath, opath])
                p.wait()
            else:
                sys.stderr.write(
                    "Warning: Required image %s: \"%s\" does not exist.\n" % (
                        repr(c), repr(i)))
                if self.verbose:
                    sys.stderr.write("Warning: Looked at the following locations:\n")
                    sys.stderr.write("\n".join(bases) + "\n")

blah = 1
class WesnothList:
    """
    Lists various Wesnoth stuff like units, campaigns, terrains, factions...
    """
    def __init__(self, isocode, datadir, userdir, transdir):
        self.unit_lookup = {}
        self.race_lookup = {}
        self.terrain_lookup = {}
        self.movetype_lookup = {}
        self.era_lookup = {}
        self.campaign_lookup = {}
        self.parser = ParserWithCoreMacros(isocode, datadir, userdir, transdir)

    def add_terrains(self):
        """
        We need those for movement costs and defenses.
        """
        WML = self.parser.parse("{core/terrain.cfg}\n")

        n = 0
        for terrain in WML.get_all("terrain_type"):
            tstring = terrain.get_text_val("string")
            self.terrain_lookup[tstring] = terrain
            n += 1
        return n

    def add_era(self, era):
        """
        For an era, list all factions and units in it.
        """
        eid = era.get_text_val("id")
        if not eid: return
        self.era_lookup[eid] = era
        era.faction_lookup = {}
        for multiplayer_side in era.get_all("multiplayer_side"):
            fid = multiplayer_side.get_text_val("id")
            if fid == "Random": continue
            era.faction_lookup[fid] = multiplayer_side
            recruit = multiplayer_side.get_text_val("recruit", "").strip()
            leader = multiplayer_side.get_text_val("leader", "").strip()
            units = recruit.split(",")
            leaders = leader.split(",")
            multiplayer_side.units = {}
            multiplayer_side.is_leader = {}
            for u in units:
                uid = u.strip()
                if uid:
                    multiplayer_side.units[uid] = True
            for u in leaders:
                uid = u.strip()
                if uid:
                    multiplayer_side.units[uid] = True
                    multiplayer_side.is_leader[uid] = True
        return eid

    def add_campaign(self, campaign):
        name = campaign.get_text_val("id")
        if not name:
            global blah
            name = "noid%d" % blah
            blah += 1
        self.campaign_lookup[name] = campaign
        return name

    def add_mainline_eras(self):
        """
        Find all mainline eras.
        """
        WML = self.parser.parse("{multiplayer/eras.cfg}\n")

        n = 0
        for era in WML.get_all("era"):
            self.add_era(era)
            n += 1
        return n

    def add_mainline_campaigns(self):
        """
        Find all mainline campaigns.
        """
        WML = self.parser.parse("{campaigns}\n")
        n = 0
        for campaign in WML.find_all("campaign"):
            self.add_campaign(campaign)
            n += 1
        return n

    def add_addons(self, image_collector):
        """
        Find all addon eras and campaigns.
        """
        n = 0
        try:
            WML = self.parser.parse("""
                #define MULTIPLAYER\n#enddef
                #define RANDOM_SIDE\n#enddef
                {~add-ons}
                """)
        except wmlparser.Error, e:
            try:
                print(e)
            except UnicodeEncodeError:
                print(e.text.encode("utf8", "ignore"))
            return n
        for campaign in WML.find_all("campaign"):
            cid = self.add_campaign(campaign)

        for era in WML.find_all("era"):
            eid = self.add_era(era)
            if not eid: continue
            image_collector.add_binary_pathes_from_WML(eid, WML)
            
        n = self.add_units(WML, "addons")
        
        image_collector.add_binary_pathes_from_WML("addons", WML)
        return n

    def add_units(self, WML, campaign):
        """
        We assume each unit, in mainline and all addons, has one unique id. So
        we reference them everywhere by this id, and here can add them all to
        one big collection.
        """
        addunits = WML.get_all("units")
        if not addunits: return 0
        
        def getall(oftype):
            r = []
            for units in addunits:
                r += units.get_all(oftype)
            return r

        # Find all unit types.
        newunits = getall("unit_type") + getall("unit")
        for unit in newunits:
            if unit.get_text_val("do_not_list", "no") == "no" and\
               unit.get_text_val("hide_help", "no") in ["no", "false"]:
                uid = unit.get_text_val("id")
                if uid in self.unit_lookup:
                    sys.stderr.write(
                        ("Fatal: Unit id \"%s\" already exists - either it has" +
                        " to be renamed, or there's a bug in this script.\n") % 
                        uid)
                else:
                    self.unit_lookup[uid] = unit
                unit.campaign = campaign

        # Find all races.
        newraces = getall("race")
        for race in newraces:
            rid = race.get_text_val("id")
            if rid == None:
                rid = race.get_text_val("name")

            self.race_lookup[rid] = race

        # Find all movetypes.
        newmovetypes = getall("movetype")
        for movetype in newmovetypes:
            mtname = movetype.get_text_val("name")
            self.movetype_lookup[mtname] = movetype

        # Store race/movetype/faction of each unit for easier access later.
        for unit in newunits:
            uid = unit.get_text_val("id")
            race = self.get_unit_value(unit, "race")
            try: unit.race = self.race_lookup[race]
            except KeyError:
                unit.race = None
                sys.stderr.write("Warning: No race \"%s\" found (%s).\n" % (
                    race, unit.get_text_val("id")))
            movetype = self.get_unit_value(unit, "movement_type")
            try: unit.movetype = self.movetype_lookup[movetype]
            except KeyError: unit.movetype = None
            
            unit.advance = []
            advanceto = unit.get_text_val("advances_to")
            # Add backwards compatibility for 1.4
            if not advanceto:
                advanceto = unit.get_text_val("advanceto")
            if advanceto and advanceto != "null":
                for advance in advanceto.split(","):
                    auid = advance.strip()
                    if auid: unit.advance.append(auid)

            # level
            try:
                level = int(self.get_unit_value(unit, "level"))
            except TypeError:
                level = 0
            except ValueError:
                level = 0
            if level < 0: level = 0
            unit.level = level

        return len(newunits)

    def find_unit_factions(self):
        for unit in self.unit_lookup.values():
            unit.factions = []
            unit.eras = []

        for eid, era in self.era_lookup.items():
            for fid, multiplayer_side in era.faction_lookup.items():
                for uid in multiplayer_side.units:
                    try:
                        unit = self.unit_lookup[uid]
                    except KeyError:
                        sys.stderr.write(
                            ("Error: Era '%s' faction '%s' references " +
                            "non-existant unit id '%s'!\n") % (
                                eid,
                                fid,
                                repr(uid)))
                        continue
                    if not eid in unit.eras:
                        unit.eras.append(eid)
                    unit.factions.append((eid, fid))

    def get_base_unit(self, unit):
        b = unit.get_first("base_unit")
        if b:
            buid = b.get_text_val("id")
            try: baseunit = self.unit_lookup[buid]
            except KeyError:
                sys.stderr.write(
                    "Warning: No baseunit \"%s\" for \"%s\".\n" % (
                    buid, unit.get_text_val("id")))
                return None
            return baseunit
        return None

    def get_unit_value(self, unit, attribute, default = None):
        value = unit.get_text_val(attribute, None)
        if value == None:
            baseunit = self.get_base_unit(unit)
            if baseunit:
                return self.get_unit_value(baseunit, attribute, default)
            return default
        return value

class UnitForest:
    """
    Contains the forest of unit advancement trees.
    """
    def __init__(self):
        self.trees = {}
        self.lookup = {}

    def add_node(self, un):
        """
        Add a new unit to the forest.
        """
        self.lookup[un.id] = un

    def create_network(self):
        """
        Assuming that each node which has been added to the tree only has a
        valid list of children in unit.child_ids, also fill in unit.parent_ids
        and update the unit.children shortcut.
        """
        
        # Complete the network
        for uid, u in self.lookup.items():
            for cid in u.child_ids:
                c = self.lookup.get(cid, None)
                if not c: continue
                u.children.append(c)
                if not uid in c.parent_ids:
                    c.parent_ids.append(uid)
            

        # Put all roots into the forest
        for uid, u in self.lookup.items():
            if not u.parent_ids:
                self.trees[uid] = u
        
        # Sanity check because some argGRRxxx addons have units who advance to
        # themselves.

        def recurse(u, already):
            already2 = already.copy()
            for c in u.children[:]:
                already2[c.id] = True
                if c.id in already:
                    sys.stderr.write(
                        "Warning: Unit %s advances to unit %s in a loop.\n" %
                        (u.id, c.id))
                    sys.stderr.write("    Removing advancement %s.\n" % c.id)
                    u.children.remove(c)
            for c in u.children:
                recurse(c, already2)
        for u in self.trees.values():
            already = {u.id : True}
            recurse(u, already)

    def update(self):
        self.create_network()       
    
        self.breadth = sum([x.update_breadth() for x in self.trees.values()])
        return self.breadth

    def get_children(self, uid):
        un = self.lookup[uid]
        return un.child_ids
    
    def get_parents(self, uid):
        un = self.lookup[uid]
        return un.parent_ids

class UnitNode:
    """
    A node in the advancement trees forest.
    """
    def __init__(self, unit):
        self.unit = unit
        self.children = []
        self.id = unit.get_text_val("id")
        self.child_ids = []
        self.parent_ids = []
        self.child_ids.extend(unit.advance)

    def update_breadth(self):
        if not self.children:
            self.breadth = 1
        else:
            self.breadth = sum([x.update_breadth() for x in self.children])
        return self.breadth   

class GroupNode:
    def __init__(self, data):
        self.data = data

