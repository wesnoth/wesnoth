"""
Various helpers for use by the wmlunits tool.
"""
import sys, os, re, glob, shutil, copy, urllib2, subprocess

import wesnoth.wmlparser2 as wmlparser2
import wesnoth.wmltools as wmltools

def get_datadir(wesnoth_exe):
    p = subprocess.Popen([wesnoth_exe, "--path"],
        stdout = subprocess.PIPE, stderr = subprocess.PIPE)
    out, err = p.communicate()
    return out.strip()

def get_userdir(wesnoth_exe):
    p = subprocess.Popen([wesnoth_exe, "--config-path"],
        stdout = subprocess.PIPE, stderr = subprocess.PIPE)
    out, err = p.communicate()
    return out.strip()

class ImageCollector:
    """
    A class to collect all the images which need to be copied to the HTML
    output folder.
    """
    def __init__(self, wesnoth_exe):
        self.images = {}
        self.paths_per_campaign = {}
        self.ipaths = {}
        self.notfound = {}
        self.id = 0
        self.verbose = 0
        
        self.datadir = get_datadir(wesnoth_exe)
        self.userdir = get_userdir(wesnoth_exe)

    def add_binary_paths_from_WML(self, campaign, WML):
        self.paths_per_campaign[campaign] = self.paths_per_campaign.get(
            campaign, [])
        for binpath in WML.get_all(tag = "binary_path"):
            path = binpath.get_text_val("path")
            self.paths_per_campaign[campaign].append(path)

    def find_image(self, i, c):
        if c == "mainline":
            bases = [os.path.join(self.datadir, "data/core/images")]
        else:
            bases = [os.path.join(self.datadir, "data/core/images")]
            binpaths = self.paths_per_campaign.get(c, [])
            binpaths.reverse()
            for x in binpaths:
                if x.startswith("data/"):
                    for idir in ["images", "images/units"]:
                        bases.append(os.path.join(self.datadir, x, idir))
                        if self.userdir:
                            bases.append(os.path.join(self.userdir, x, idir))
        for base in bases:
            tilde = i.find("~")
            if tilde >= 0:
                i = i[:tilde]
            ipath = os.path.join("%s" % base, i)
            if os.path.exists(ipath): return ipath, None
        return None, bases

    def add_image_check(self, campaign, path):
        if (campaign, path) in self.notfound:
            return self.notfound[(campaign, path)], True
        ipath, error = self.find_image(path, campaign)
        if ipath in self.ipaths:
            return self.ipaths[ipath], False
    
        name = "%05d_" % self.id
        name += os.path.basename(path)
        self.id += 1

        self.images[name] = ipath, path, campaign, error
        if ipath:
            self.ipaths[ipath] = name
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
                p = subprocess.Popen([command, ipath, opath])
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
    def __init__(self, wesnoth_exe, transdir):
        self.unit_lookup = {}
        self.race_lookup = {}
        self.terrain_lookup = {}
        self.movetype_lookup = {}
        self.era_lookup = {}
        self.campaign_lookup = {}
        self.parser = wmlparser2.Parser(wesnoth_exe)
        
        self.parser = wmlparser2.Parser(wesnoth_exe)

    def add_terrains(self):
        """
        We need those for movement costs and defenses.
        """
        self.parser.parse_text("{core/terrain.cfg}\n")

        n = 0
        for terrain in self.parser.get_all(tag = "terrain_type"):
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
        for multiplayer_side in era.get_all(tag = "multiplayer_side"):
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
        self.parser.parse_text("{multiplayer/eras.cfg}")

        n = 0
        for era in self.parser.get_all(tag = "era"):
            self.add_era(era)
            n += 1
        return n

    def add_mainline_campaigns(self):
        """
        Find all mainline campaigns.
        """
        self.parser.parse_text("{campaigns}")
        n = 0
        for campaign in self.parser.get_all(tag = "campaign"):
            self.add_campaign(campaign)
            n += 1
        return n

    def add_addons(self, image_collector):
        """
        Find all addon eras and campaigns.
        """
        self.parser.parse_text("{multiplayer}{~add-ons}", "MULTIPLAYER")
        
        cn = 0
        for campaign in self.parser.get_all(tag = "campaign"):
            cid = self.add_campaign(campaign)
            cn += 1

        en = 0
        for era in self.parser.get_all(tag = "era"):
            eid = self.add_era(era)
            en += 1
            if not eid: continue
            image_collector.add_binary_paths_from_WML(eid,
                self.parser.root)

        un = self.add_units("addons")

        image_collector.add_binary_paths_from_WML("addons",
            self.parser.root)
        return cn, en, un
    
    def add_mainline_units(self):
        self.parser.parse_text("{core/units.cfg}")
        return self.add_units("mainline")

    def add_campaign_units(self, cname, image_collector):
        campaign = self.campaign_lookup[cname]
        define = campaign.get_text_val("define")
        self.parser.parse_text("{campaigns}", define)
        
        image_collector.add_binary_paths_from_WML(cname,
            self.parser.root)
        
        return self.add_units(cname)

    def add_addon_campaign_units(self, cname, image_collector):
        campaign = self.campaign_lookup[cname]
        define = campaign.get_text_val("define")
        self.parser.parse_text("{~add-ons}", define)
        
        image_collector.add_binary_paths_from_WML(cname,
            self.parser.root)

        return self.add_units(cname)

    def add_units(self, campaign):
        """
        We assume each unit, in mainline and all addons, has one unique id. So
        we reference them everywhere by this id, and here can add them all to
        one big collection.
        """
        addunits = self.parser.get_all(tag = "units")
        addunits += self.parser.get_all(tag = "+units")
        if not addunits: return 0

        def getall(oftype):
            r = []
            for units in addunits:
                r += units.get_all(tag = oftype)
            return r

        # Find all unit types.
        newunits = getall("unit_type") + getall("unit")
        for unit in newunits:
            if unit.get_text_val("do_not_list", "no") == "no" and\
               unit.get_text_val("hide_help", "no") in ["no", "false"]:
                uid = unit.get_text_val("id")
                if uid in self.unit_lookup:
                    pass
                    # TODO: We might want to compare the two units
                    # with the same id and if one is different try
                    # to do something clever like rename it...
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
        b = unit.get_all(tag = "base_unit")
        if b:
            b = b[0]
            buid = b.get_text_val("id")
            try: baseunit = self.unit_lookup[buid]
            except KeyError:
                sys.stderr.write(
                    "Warning: No baseunit \"%s\" for \"%s\".\n" % (
                    buid, unit.get_text_val("id")))
                return None
            return baseunit
        return None

    def get_unit_value(self, unit, attribute, default = None, translation = None):
        value = unit.get_text_val(attribute, None, translation)
        if value == None:
            baseunit = self.get_base_unit(unit)
            if baseunit:
                return self.get_unit_value(baseunit, attribute, default, translation)
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

