"""
Various helpers for use by the wmlunits tool.
"""
import sys, os, re, glob, shutil, copy, urllib2

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
        parser.do_preprocessor_logic = True
        parser.gettext = self.gettext
        
        # Parse core macros.
        parser.parse_text("{core/macros/}\n")
        parser.parse_top(None)
        self.core_macros = parser.macros
                
    def parse(self, text_to_parse, ignore_macros = None):
        # Create the real parser.
        parser = wmlparser.Parser(self.datadir, self.userdir)
        parser.do_preprocessor_logic = True
        parser.gettext = self.gettext
        parser.macros = copy.copy(self.core_macros)
        
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

    def add_image(self, campaign, path):
        if (campaign, path) in self.notfound: return self.notfound[(campaign, path)]
        ipath, error = self.find_image(path, campaign)
        if ipath in self.ipathes:
            return self.ipathes[ipath]
    
        name = "%05d_" % self.id
        name += os.path.basename(path)
        self.id += 1

        self.images[name] = ipath, path, campaign, error
        if ipath:
            self.ipathes[ipath] = name
        else:
            self.notfound[(campaign, path)] = name
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
                command = os.path.join("data",
                    "tools/unit_tree/TeamColorizer") + " '%s' '%s'" % (
                    ipath, opath)
                os.system(command)
            else:
                sys.stderr.write(
                    "Warning: Required image %s: \"%s\" does not exist.\n" % (c, i))
                sys.stderr.write("Warning: Looked at the following locations:\n")
                sys.stderr.write("\n".join(bases) + "\n")

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
        for terrain in WML.get_all("terrain"):
            tid = terrain.get_text_val("id")
            self.terrain_lookup[tid] = terrain
            n += 1
        return n

    def add_era(self, era):
        """
        For an era, list all factions and units in it.
        """
        eid = era.get_text_val("id")
        self.era_lookup[eid] = era
        era.faction_lookup = {}
        for multiplayer_side in era.get_all("multiplayer_side"):
            fid = multiplayer_side.get_text_val("id")
            if fid == "Random": continue
            era.faction_lookup[fid] = multiplayer_side
            recruit = multiplayer_side.get_text_val("recruit").strip()
            leader = multiplayer_side.get_text_val("leader").strip()
            units = recruit.split(",") + leader.split(",")
            multiplayer_side.units = {}
            for u in units:
                uid = u.strip()
                if uid:
                    multiplayer_side.units[uid] = True
        return eid

    def add_campaign(self, campaign):
        name = campaign.get_text_val("id")
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
        WML = self.parser.parse("""
            #define MULTIPLAYER\n#enddef
            #define RANDOM_SIDE\n#enddef
            {~campaigns}
            """)
        n = 0
        for campaign in WML.find_all("campaign"):
            cid = self.add_campaign(campaign)
            n += 1
            image_collector.add_binary_pathes_from_WML(cid, WML)
        
        for era in WML.find_all("era"):
            eid = self.add_era(era)
            image_collector.add_binary_pathes_from_WML(eid, WML)
            n += 1
            
        n = self.add_units(WML, "addons")
        
        image_collector.add_binary_pathes_from_WML("addons", WML)
        return n

    def add_units(self, WML, campaign):
        """
        We assume each unit, in mainline and all addons, has one unique id. So
        we reference them everywhere by this id, and here can add them all to
        one big collection.
        """
        addunits = WML.get_all("+units")
        if not addunits: return 0
        
        def getall(oftype):
            r = []
            for units in addunits:
                r += units.get_all(oftype)
            return r

        # Find all unit types.
        newunits = getall("unit_type") + getall("unit")
        for unit in newunits:
            if unit.get_text_val("do_not_list", "no") == "no":
                uid = unit.get_text_val("id")
                self.unit_lookup[uid] = unit
                unit.campaign = campaign

        # Find all races.
        newraces = getall("race")
        for race in newraces:
            rid = race.get_text_val("id")
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
                            "non-existant unit id '%s'!\n") % (eid, fid, uid))
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

        # First, we check if any of the new node's advancements already is in
        # the forest. If so, remove it and attach it to the new node.
        for cid in un.child_ids:
            if cid in self.trees:
                un.children.append(self.trees[cid])
                self.lookup[cid].parent_ids.append(un.id)
                del self.trees[cid]

        # Next, we check if the node is an advancement of an existing node. If
        # so, add it there.
        for rootnode in self.trees.values():
            if rootnode.try_add(un):
                return

        # Else, add a new tree with the new node as root.
        self.trees[un.id] = un

    def update_breadth(self):
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
        advanceto = unit.get_text_val("advanceto")
        if advanceto and advanceto != "null":
            for advance in advanceto.split(","):
                advance = advance.strip()
                self.child_ids.append(advance)

    def try_add(self, un):
        # A child of yours truly?
        if un.id in self.child_ids:
            self.children.append(un)
            un.parent_ids.append(self.id)
            return True
        # A recursive child?
        for child in self.children:
            if child.try_add(un): return True
        return False

    def update_breadth(self):
        if not self.children:
            self.breadth = 1
        else:
            self.breadth = sum([x.update_breadth() for x in self.children])
        return self.breadth   

class GroupNode:
    def __init__(self, data):
        self.data = data

