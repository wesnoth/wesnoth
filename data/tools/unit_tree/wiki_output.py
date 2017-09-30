#!/usr/bin/env python3

import sys
from . import helpers
from .html_output import Translation

def main():
    wesnoth = helpers.WesnothList(
        options.wesnoth,
        options.config_dir,
        options.data_dir,
        options.transdir)
    
    translated = Translation(options.transdir, options.language)
    original = Translation(options.transdir, "en_US")
    
    punits = {}
    
    defines = "NORMAL,ENABLE_ARMAGEDDON_DRAKE,ENABLE_DWARVISH_ARCANISTER," +\
        "ENABLE_DWARVISH_RUNESMITH,ENABLE_ANCIENT_LICH,ENABLE_DEATH_KNIGHT," +\
        "ENABLE_TROLL_SHAMAN,ENABLE_WOLF_ADVANCEMENT"
    
    sys.stderr.write("Parsing core units...\n")
    wesnoth.parser.parse_text("{core/units.cfg}", defines)
    punits["mainline"] = wesnoth.parser.get_all(tag = "units")
    punits["mainline"] += wesnoth.parser.get_all(tag = "+units")

    all_campaigns = {}
    sys.stderr.write("Parsing campaigns...\n")
    wesnoth.parser.parse_text("{campaigns}", defines)
    campaigns = wesnoth.parser.get_all(tag = "campaign")
    for campaign in campaigns:
        define = campaign.get_text_val("define")
        ed = campaign.get_text_val("extra_defines")
        if ed: define += "," + ed
        name = campaign.get_text_val("name", translation = translated.translate)
        sys.stderr.write("Parsing " + name + "...\n")
        campaign.name = name
        all_campaigns[campaign.get_text_val("id")] = campaign
        wesnoth.parser.parse_text("{campaigns}", defines + "," + define)
        punits[name] = wesnoth.parser.get_all(tag = "units")
        punits[name] += wesnoth.parser.get_all(tag = "+units")
    
    # Get all defined races.
    races = {}
    for campaign, unitslists in list(punits.items()):
        for unitlist in unitslists:
            for race in unitlist.get_all(tag = "race"):
                races[race.get_text_val("id")] = race
    
    # Go through all units and put them into a dictionary.
    all_units = {}
    for campaign, unitslists in list(punits.items()):
        for unitlist in unitslists:
            for unit in unitlist.get_all(tag = "unit_type"):
                if unit.get_text_val("do_not_list") in ["yes", "true"]: continue
                if unit.get_text_val("hide_help") in ["yes", "true"]: continue
                unit.id = unit.get_text_val("id")
                unit.campaign = campaign
                all_units[unit.id] = unit
                unit.children = []
                unit.parents = []
    
    def base_val(unit, val, translation = None):
        x = unit.get_text_val(val, translation = translation)
        if x: return x
        for base_unit in unit.get_all(tag = "base_unit"):
            base = all_units[base_unit.get_text_val("id")]
            x = base_val(base, val, translation = translation)
            if x: return x
        return None
    
    # Handle unit attributes
    for unit in list(all_units.values()):
        
        unit.name = base_val(unit, "name", translation = translated.translate)               
        unit.orig_name = base_val(unit, "name", translation = original.translate)
        
        try: unit.level = int(base_val(unit, "level"))
        except TypeError: unit.level = 0
        
        r = base_val(unit, "race")
        try: unit.race = races[r].get_text_val("plural_name", translation = translated.translate)   
        except KeyError: unit.race = "-"

        a = unit.get_text_val("advances_to")
        if not a or a == "null": unit.advances_to = []
        else: unit.advances_to = [x.strip() for x in a.split(",")]
    
    # Find children and parents of all units.
    for unit in list(all_units.values()):
        for aid in unit.advances_to:
            unit.children.append(all_units[aid])
            all_units[aid].parents.append(unit)
        for af in unit.get_all(tag = "advancefrom"):
            afid = af.get_text_val("unit")
            all_units[afid].children.append(unit)
            unit.parents.append(all_units[afid])

    def race_key(unit):
        if unit.campaign == "mainline": return 0, unit.race
        else : return 1, unit.campaign

    # Group by race/campaign
    units_per_race = {}
    for unit in list(all_units.values()):
        x = race_key(unit)
        if x not in units_per_race: units_per_race[x] = set()
        units_per_race[x].add(unit)
    
    # Recursively add all related units of a units to the same race as well.
    for race in list(units_per_race.keys()):
        while True:
            add = []
            for unit in units_per_race[race]:
                for rel in unit.children + unit.parents:
                    if rel not in units_per_race[race]:
                        add.append(rel)
            if not add: break
            for x in add: units_per_race[race].add(x)

    races = sorted(units_per_race.keys())
    
    def w(x): sys.stdout.write(x.encode("utf8") + "\n")
    
    # Now output the units list per race/campaign.
    for race in races:
        
        units = units_per_race[race]
        
        w("=== " + race[1] + " ===")

        w("{|")
    
        # Find root units.
        roots = []
        for u in units:
            if not u.parents:
                 roots.append(u)
                 continue
            if not [x for x in u.parents if x.race == u.race]:
                roots.append(u)
             
        roots.sort(key = lambda u: u.name)
        
        # Get a grid position for each unit.
        def handle_children(y, unit):
            unit.y = y
            for cunit in unit.children:
                y = handle_children(y, cunit)
            if not unit.children: y += 1
            return y
        n = 0
        for root in roots:
            n = handle_children(n, root)

        # Create grid.
        grid = []
        for j in range(n + 1):
            grid.append([None] * 6)
        for unit in units:
            grid[unit.y][unit.level] = unit
        
        # Output it.
        for y in range(n + 1):
            for x in range(6):
                unit = grid[y][x]
                if unit:
                    w("|'''" + unit.name + "'''<br />" + unit.orig_name)
                else:
                    w("|")
            w("|-")
        w("|}")
