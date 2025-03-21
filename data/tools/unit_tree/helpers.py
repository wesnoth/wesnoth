#!/usr/bin/env python3

"""
Various helpers for use by the wmlunits tool.
"""
import sys, os, re, glob, shutil, copy, subprocess, traceback

import wesnoth.wmlparser3 as wmlparser3
from unit_tree.team_colorizer import colorize
import wesnoth.base64url as base64url

class Image:
    def __init__(self, id_name, ipath, bases, no_tc):
        self.id_name = id_name
        self.ipath = ipath # none if it was not found
        self.bases = bases
        self.no_tc = no_tc
        self.addons = set()

class ImageCollector:
    """
    A class to collect all the images which need to be copied to the HTML
    output folder.
    """
    def __init__(self, wesnoth_exe, userdir, datadir, magick_exe="convert"):
        self.images_by_addon_name = {}
        self.images_by_ipath = {}
        self.binary_paths_per_addon = {}
        self.datadir = datadir
        self.userdir = userdir
        self.hide_paths = [
            os.path.join(self.userdir, "data", "add-ons"),
            os.path.join(self.userdir, "data"),
            os.path.join(self.userdir),
            os.path.join(self.datadir, "data"),
            os.path.join(self.datadir)
        ]
        self.magick = magick_exe

    def add_binary_paths_from_WML(self, addon, WML):
        for binpath in WML.get_all(tag="binary_path"):
            path = binpath.get_text_val("path")
            if addon not in self.binary_paths_per_addon:
                self.binary_paths_per_addon[addon] = []
            self.binary_paths_per_addon[addon].append(path)

    def _find_image(self, addon, name, check_transparent):
        tilde = name.find("~")
        if tilde >= 0:
            name = name[:tilde]
        bases = [
            os.path.join(self.datadir, "data", "core", "images"),
            os.path.join(self.datadir, "images")
        ]
        binpaths = self.binary_paths_per_addon.get(addon, [])[:]
        binpaths.reverse()
        for path in binpaths:
            for idir in ["images", "images/units"]:
                bases.append(os.path.join(self.datadir, path, idir))
                bases.append(os.path.join(self.userdir, path, idir))

        if not check_transparent:
            bases = [os.path.join("%s" % base, name) for base in bases]
        else:
            dirname, filename = os.path.split(name)
            new_bases = []
            for base in bases:
                new_bases.append(os.path.join("%s" % base, dirname, "transparent", filename))
                new_bases.append(os.path.join("%s" % base, name))
            bases = new_bases

        for ipath in bases:
            ipath = os.path.normpath(ipath)
            if os.path.exists(ipath):
                return ipath, list(map(os.path.normpath, bases))
        return None, list(map(os.path.normpath, bases))

    def add_image_check(self, addon, name, no_tc=False, check_transparent=False):
        if (addon, name) in self.images_by_addon_name:
            image = self.images_by_addon_name[(addon, name)]
            if addon not in image.addons:
                image.addons.add(addon)
            return image

        ipath, bases = self._find_image(addon, name, check_transparent)
        if ipath in self.images_by_ipath:
            image = self.images_by_ipath[ipath]
            if addon not in image.addons:
                image.addons.add(addon)
            return image

        def make_name(vpath, hide_paths, prefix_fallback=None):
            if prefix_fallback is not None:
                vpath = os.path.join(prefix_fallback, vpath)
            vpath = os.path.normpath(vpath.strip())
            for hidden_path in hide_paths:
                if vpath.startswith(hidden_path):
                #if os.path.commonpath(vpath, hidden_path) == hidden_path:
                    vpath = os.path.relpath(vpath, hidden_path)
                    break
            else:                
                # root_dir is prefix after normalization
                root_dir = vpath.split("/")[0].split("\\")[0]
                if prefix_fallback is None or (root_dir != "general" and root_dir != prefix_fallback):
                    print("Path \"%s\" outside valid folders." % vpath)
                    vpath = "403.png"

            encoded_dir_name = base64url.encode_str(os.path.dirname(vpath))
            sanitized_file_name = re.sub(r'[^a-zA-Z0-9_.-]' , "", ".".join(os.path.basename(vpath).split("(")[0].split(".")[-2:]))
            head, ext = os.path.splitext(sanitized_file_name)
            return '%s..%s%s' % (head, encoded_dir_name, ext)

        if ipath:
            id_name = make_name(ipath, self.hide_paths)
        else:
            id_name = make_name(name, self.hide_paths, prefix_fallback=addon)

        image = Image(id_name, ipath, bases, no_tc)
        image.addons.add(addon)

        self.images_by_addon_name[(addon, name)] = image
        if ipath:
            self.images_by_ipath[ipath] = image

        return image

    def add_image(self, addon, path, no_tc=False, check_transparent=False):
        image = self.add_image_check(addon, path, no_tc, check_transparent)
        return image.id_name

    def copy_and_color_images(self, target_path):
        for image in list(self.images_by_ipath.values()):
            opath = os.path.join(target_path, "pics", image.id_name)
            try:
                os.makedirs(os.path.dirname(opath))
            except FileExistsError:
                pass
            except OSError:
                traceback.print_exc()

            no_tc = image.no_tc

            ipath = os.path.normpath(image.ipath)
            default_addons_dir = os.path.join(options.config_dir, "data", "add-ons")
            if ipath.startswith(default_addons_dir):
                # Override with custom --addons
                ipath = os.path.join(options.addons, os.path.relpath(ipath, default_addons_dir))
            if ipath and os.path.exists(ipath) and not os.path.isdir(ipath):
                if no_tc:
                    shutil.copy2(ipath, opath)
                else:
                    colorize(None, ipath, opath, magick=self.magick)
            else:
                sys.stderr.write(
                    "Warning: Required image %s does not exist at %s (referenced by %s).\n" % (
                        image.id_name, ipath, ", ".join(image.addons)))
                if options.verbose:
                    if image.bases:
                        sys.stderr.write("Warning: Looked at the following locations:\n")
                        sys.stderr.write("\n".join(image.bases) + "\n")
                    else:
                        sys.stderr.write("nowhere\n")

blah = 1
class WesnothList:
    """
    Lists various Wesnoth stuff like units, campaigns, terrains, factions...
    """
    def __init__(self, wesnoth_exe, config_dir, data_dir, transdir):
        self.unit_lookup = {}
        self.race_lookup = {}
        self.terrain_lookup = {}
        self.movetype_lookup = {}
        self.era_lookup = {}
        self.campaign_lookup = {}
        self.parser = wmlparser3.Parser(wesnoth_exe, config_dir, data_dir)

    def add_terrains(self):
        """
        We need those for movement costs and defenses.
        """
        self.parser.parse_text("{core/terrain.cfg}\n")

        n = 0
        for terrain in self.parser.get_all(tag="terrain_type"):
            tstring = terrain.get_text_val("string")
            if tstring is None: continue
            self.terrain_lookup[tstring] = terrain
            n += 1
        return n

    def add_languages(self, languages):
        """
        Returns a dictionary mapping isocodes to languages.
        """
        self.languages_found = {}

        parser = wmlparser3.Parser(options.wesnoth,
                                   options.config_dir,
                                   options.data_dir)
        parser.parse_text("{languages}")

        for locale in parser.get_all(tag="locale"):
            isocode = locale.get_text_val("locale")
            name = locale.get_text_val("name")
            if isocode is None or name is None:
                continue
            if isocode == "ang_GB":
                continue
            self.languages_found[isocode] = name

    def add_era(self, era):
        """
        For an era, list all factions and units in it.
        """
        eid = era.get_text_val("id")
        if not eid:
            return
        self.era_lookup[eid] = era
        era.faction_lookup = {}
        for multiplayer_side in era.get_all(tag="multiplayer_side"):
            fid = multiplayer_side.get_text_val("id")
            if fid == "Random" or fid in era.faction_lookup:
                continue
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

    def add_binary_paths(self, addon, image_collector):
        image_collector.add_binary_paths_from_WML(addon, self.parser.root)

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
        for era in self.parser.get_all(tag="era"):
            self.add_era(era)
            n += 1
        return n

    def add_units(self, campaign):
        """
        We assume each unit, in mainline and all addons, has one unique id. So
        we reference them everywhere by this id, and here can add them all to
        one big collection.
        """
        addunits = self.parser.get_all(tag="units")
        addunits += self.parser.get_all(tag="+units")
        if not addunits:
            return 0

        def getall(oftype):
            res = []
            for units in addunits:
                res += units.get_all(tag=oftype)
            return res

        # Find all unit types.
        newunits = getall("unit_type") + getall("unit")
        for unit in newunits:
            uid = unit.get_text_val("id", "none")
            unit.id = uid

            if unit.get_text_val("do_not_list", "no") not in ["no", "false"] or \
               unit.get_text_val("hide_help", "no") not in ["no", "false"]:
                unit.hidden = True
            else:
                unit.hidden = False

            if uid in self.unit_lookup:
                unit = self.unit_lookup[uid]
                # TODO: We might want to compare the two units
                # with the same id and if one is different try
                # to do something clever like rename it...
            else:
                self.unit_lookup[uid] = unit
            if not hasattr(unit, "campaigns"):
                unit.campaigns = []
            unit.campaigns.append(campaign)

        # Find all races.
        newraces = getall("race")
        for race in newraces:
            rid = race.get_text_val("id")
            if rid is None:
                rid = race.get_text_val("name", "none")
            self.race_lookup[rid] = race

        # Find all movetypes.
        newmovetypes = getall("movetype")
        for movetype in newmovetypes:
            mtname = movetype.get_text_val("name")
            if mtname is None: continue
            self.movetype_lookup[mtname] = movetype

        # Store race/movetype/faction of each unit for easier access later.
        for unit in newunits:
            race = self.get_unit_value(unit, "race")
            try:
                unit.race = self.race_lookup[race]
                unit.rid = unit.race.get_text_val("id", "none")
            except KeyError:
                unit.race = None
                unit.rid = "none"
                error_message("Warning: No race \"%s\" found (%s).\n" % (
                    race, unit.get_text_val("id")))
            movetype = self.get_unit_value(unit, "movement_type")
            try:
                unit.movetype = self.movetype_lookup[movetype]
            except KeyError:
                unit.movetype = None

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

    def check_units(self):
        """
        Once all units have been added, do some checking.
        This function used to handle now-removed advancefrom tags
        """
        return

    def find_unit_factions(self):
        for unit in list(self.unit_lookup.values()):
            unit.factions = []
            unit.eras = []
        for eid, era in list(self.era_lookup.items()):
            for fid, multiplayer_side in list(era.faction_lookup.items()):
                for uid in multiplayer_side.units:
                    try:
                        unit = self.unit_lookup[uid]
                    except KeyError:
                        error_message(
                            ("Error: Era '%s' faction '%s' references " +
                             "non-existant unit id '%s'!\n") % (
                                 eid,
                                 fid,
                                 str(uid)))
                        continue
                    if not eid in unit.eras:
                        unit.eras.append(eid)
                    unit.factions.append((eid, fid))
        # as a special case, add units from this addon but with no faction
        for unit in list(self.unit_lookup.values()):
            if unit.campaigns[0] == self.cid:
                if not unit.factions:
                    if not eid in unit.eras:
                        unit.eras.append(eid)
                    unit.factions.append((eid, None))

    def get_base_unit(self, unit):
        b = unit.get_all(tag="base_unit")
        if b:
            b = b[0]
            buid = b.get_text_val("id")
            try:
                baseunit = self.unit_lookup[buid]
            except KeyError:
                error_message(
                    "Warning: No baseunit \"%s\" for \"%s\".\n" % (
                        buid, unit.get_text_val("id")))
                return None
            return baseunit
        return None

    def get_unit_value(self, unit, attribute, default=None, translation=None):
        value = unit.get_text_val(attribute, None, translation)
        if value is None:
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
        for uid, u in list(self.lookup.items()):
            for cid in u.child_ids:
                c = self.lookup.get(cid, None)
                if not c:
                    continue
                u.children.append(c)
                if not uid in c.parent_ids:
                    c.parent_ids.append(uid)

        # Put all roots into the forest
        for uid, u in list(self.lookup.items()):
            if not u.parent_ids:
                self.trees[uid] = u

        # Sanity check because some argGRRxxx addons have units who advance to
        # themselves.

        def recurse(u, already):
            already2 = already.copy()
            for c in u.children[:]:
                already2[c.id] = True
                if c.id in already:
                    error_message(
                        ("Warning: Unit %s advances to unit %s in a loop.\n" %
                         (u.id, c.id)) +
                        ("    Removing advancement %s.\n" % c.id))
                    u.children.remove(c)
            for c in u.children:
                recurse(c, already2)
        for u in list(self.trees.values()):
            already = {u.id : True}
            recurse(u, already)

    def update(self):
        self.create_network()
        self.breadth = sum([x.update_breadth() for x in list(self.trees.values())])
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
        self.id = unit.get_text_val("id", "none")
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
