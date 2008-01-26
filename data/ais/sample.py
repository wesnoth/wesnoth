#!WPY

"""This is a rather simple minded example of a python AI."""

import wesnoth, heapq, random

def pos(location):
    """Just a helper function for printing positions in debug messages."""
    return "(%d, %d)" % (1 + location.x, 1 + location.y)

def debug(string):
    pass

class AI:
    """A class representing our AI."""

    def __init__(self):
        """This class is constructed once for each turn of the AI. To get
        persistent variables across terms, which also are saved when the game is
        saved, use set_variable and get_variable."""

        self.team = wesnoth.get_current_team()
        self.village_radius = 25
        self.scout_villages = 3

        self.recruit()

        self.fight()

        self.conquer()

    def conquer(self):
        """Try to capture villages."""
        villages = self.find_villages()
        units =  wesnoth.get_destinations_by_unit().keys()

        # Construct a sorted list of (distance, unit, village) triples.
        queue = []
        for village in villages:
            for unit in units:
                d = self.get_distance(unit, village)
                if d != None: heapq.heappush(queue, (d, unit, village))

        # Now assign units to villages, and move them.
        while queue:
            d, unit, village = heapq.heappop(queue)
            if unit in units and village in villages:
                units.remove(unit)
                villages.remove(village)
                self.go_to(unit, village)

                if not units: break
                if not villages: break

    def cumulate_damage(self, cumulated, hitpoints, new_damage):
        cumulated2 = {}
        for already, ap in cumulated.iteritems():
            for hp, probability in new_damage.iteritems():
                damage = int(already + hitpoints - hp)
                cumulated2[damage] = cumulated2.get(damage, 0) + ap * probability
        return cumulated2

    def danger_estimate(self, unit, where, enemy):
        """Get some crude indication about how unsafe it is for unit to get
        attacked by enemy at where."""

        scores = []
        u = wesnoth.get_units()[unit]
        e = wesnoth.get_units()[enemy]
        u_defense = u.defense_modifier(wesnoth.get_map(), where)
        e_defense = e.defense_modifier(wesnoth.get_map(), enemy)

        u_bonus = 100 - (u.type().alignment - 1) * wesnoth.get_gamestatus().lawful_bonus
        e_bonus = 100 - (e.type().alignment - 1) * wesnoth.get_gamestatus().lawful_bonus

        for attack in e.attacks():
            score = attack.damage * attack.num_attacks * e_bonus / 100
            score *= u_defense
            score *= u.damage_against(attack) / 100

            back = []
            for retaliation in u.attacks():
                if attack.range == retaliation.range:
                    x = retaliation.damage * retaliation.num_attacks * u_bonus / 100
                    x *= e_defense
                    x *= e.damage_against(retaliation) / 100
                    back.append(x)

            if back:
                r = max(back)
                score -= r
            heapq.heappush(scores, score)

        return scores[0]

    def danger(self, unit, location):
        """Try to estimate danger of moving unit to location."""
        attackers = []
        for enemy, destinations in wesnoth.get_enemy_destinations_by_unit():
            for tile in wesnoth.get_adjacent_tiles(unit):
                if tile in destinations:
                    heuristic = danger_estimate(unitm, location, enemy)
                    if heuristic > 0:
                        heapq.heappush(attackers, (-heuristic, enemy, tile))
        result = 0
        already = {}
        while attackers:
            danger, enemy, tile = heapq.heappop(attackers)
            if not already[enemy] and not already[tile]:
                danger = -danger
                result += danger
                already[enemy] = 1
                already[tile] = 1
        return result

    def fight(self):
        """Attack enemies."""
        enemies =  wesnoth.get_enemy_destinations_by_unit().keys()
        units =  wesnoth.get_destinations_by_unit().keys()

        # Get a list of all units we can possibly kill and their chance to kill.
        # This is just a heuristic, ignoring ZoC and unit placement.
        kills = []
        for enemy in enemies:
            e = wesnoth.get_units()[enemy]
            k = {0: 1.0}
            for unit, destinations in wesnoth.get_destinations_by_unit().iteritems():
                u = wesnoth.get_units()[unit]
                for tile in wesnoth.get_adjacent_tiles(enemy):
                    if tile in destinations:
                        own_hp, enemy_hp = u.attack_statistics(tile, enemy)
                        k = self.cumulate_damage(k, e.hitpoints, enemy_hp)
            ctk = 0
            for damage, p in k.iteritems():
                if damage >= e.hitpoints:
                    ctk += p
            if ctk:
                heapq.heappush(kills, (-ctk, enemy))

        # Now find positions from where own units can attack the to be killed
        # enemies.
        attacks = []
        while kills:
            ctk, enemy = heapq.heappop(kills)
            e = wesnoth.get_units()[enemy]
            ctk = -ctk
            for tile in wesnoth.get_adjacent_tiles(enemy):
                for unit in wesnoth.get_units_by_destination().get(tile, []):
                    u = wesnoth.get_units()[unit]
                    own_hp, enemy_hp = u.attack_statistics(tile, enemy)
                    score = e.hitpoints - sum([x[0] * x[1] for x in enemy_hp.iteritems()])
                    score -= u.hitpoints - sum([x[0] * x[1] for x in own_hp.iteritems()])

                    # This is so if there are two equally good attack
                    # possibilities, we chose the one on better terrain.
                    score *= 50 / u.defense_modifier(tile)

                    heapq.heappush(attacks, (-score, unit, tile, enemy))
                    #print own_hp, enemy_hp
                    debug("Score for %s at %s: %s<->%s: %f [%s]" % (u.name,
                        pos(unit), pos(tile), pos(enemy), score, e.name))

        # Now assign units to enemies, and move and attack.
        while attacks:
            score, unit, tile, enemy = heapq.heappop(attacks)
            score = -score

            if unit in units and enemy in enemies:
                #try:
                loc = wesnoth.move_unit(unit, tile)
                #except ValueError:
                #    loc = None
                if loc == tile:
                    e = wesnoth.get_units()[enemy]
                    wesnoth.attack_unit(tile, enemy)
                    if not e.is_valid:
                        enemies.remove(enemy)
                    units.remove(unit)
                    if not units: break

    def recruit(self):
        """Recruit units."""

        # Check if there is any gold left first.
        cheapest = min([x.cost for x in self.team.recruits()])
        if self.team.gold < cheapest: return

        # Find all keeps in the map.
        keeps = self.find_keeps()

        # Find our leader.
        leader = None
        for location, unit in wesnoth.get_units().iteritems():
            if unit.side == self.team.side and unit.can_recruit:
                leader = location
                break

        # Get number of villages to capture near to the leader.
        villages = len([x for x in self.find_villages()
            if leader.distance_to(x) < self.village_radius])

        units_recruited = int(wesnoth.get_variable("units_recruited") or 0)

        def attack_score(u1, u2):
            """Some crude score of u1 attacking u2."""
            maxdeal = 0
            for attack in u1.attacks():
                deal = attack.damage * attack.num_attacks
                deal *= u2.damage_from(attack) / 100.0
                for defense in u2.attacks():
                    if attack.range == defense.range:
                        receive = defense.damage * defense.num_attacks
                        receive *= u1.damage_from(defense) / 100.0
                        deal -= receive
                if deal > maxdeal: maxdeal = deal
            return maxdeal

        def recruit_score(recruit, speed, defense, aggression, resistance):
            """Score for recruiting the given unit type."""
            need_for_speed = 3 * (villages / self.scout_villages -
                units_recruited)
            if need_for_speed < 0: need_for_speed = 0
            v = speed * need_for_speed + defense * 0.1 + aggression + resistance
            v += 1
            if v < 1: v = 1
            return v

        # Try to figure out which units are good in this map.
        map = wesnoth.get_map()
        recruits = self.team.recruits()
        recruits_list = []
        for recruit in recruits:
            speed = 0.0
            defense = 0.0
            n = map.x * map.y
            for y in range(map.y):
                for x in range(map.x):
                    location = wesnoth.get_location(x, y)
                    speed += recruit.movement_cost(location)
                    defense += 100 - recruit.defense_modifier(location)
            speed = recruit.movement * n / speed
            defense /= n

            aggression = 0.0
            resistance = 0.0
            enemies = wesnoth.get_enemy_destinations_by_unit().keys()
            n = len(enemies)
            for location in enemies:
                enemy = wesnoth.get_units()[location]
                aggression += attack_score(recruit, enemy)
                resistance -= attack_score(enemy, recruit)
            aggression /= n
            resistance /= n

            debug("%s: speed: %f, defense: %f, aggression: %f, resistance: %f" %
                (recruit.name, speed, defense, aggression, resistance)

            recruits_list.append((recruit, speed, defense, aggression, resistance))

        # Now recruit.
        for location, unit in wesnoth.get_units().iteritems():
            if unit.side == self.team.side and unit.can_recruit:

                keepsort = []
                for keep in keeps:
                    heapq.heappush(keepsort, (location.distance_to(keep), keep))

                keep = keepsort[0][1]

                self.go_to(location, keep)
                for i in range(6): # up to 6 units (TODO: can be more)
                    # Get a random, weighted unit type from the available.
                    heap = []
                    total_v = 0
                    for r in recruits_list:
                        v = recruit_score(*r)
                        v *= v * v
                        total_v += v
                        heapq.heappush(heap, (-v, r[0]))
                    r = random.uniform(0, total_v)
                    while 1:
                        v, recruit = heapq.heappop(heap)
                        debug("%d %d" % % (r, v))
                        r += v
                        if r <= 0: break

                    # Try to recruit it on the adjacent tiles
                    # TODO: actually, it should just use the nearest possible
                    # location
                    for position in wesnoth.get_adjacent_tiles(location):
                        if wesnoth.recruit_unit(recruit.name, position):
                            break
                    else:
                        # was not possible -> we're done
                        break
                    units_recruited += 1
                    wesnoth.set_variable("units_recruited", str(units_recruited))

    def find_villages(self):
        """Find all villages which are unowned or owned by enemies."""
        villages = []
        m = wesnoth.get_map()
        for x in range(m.x):
            for y in range(m.y):
                location = wesnoth.get_location(x, y)
                if wesnoth.get_map().is_village(location):
                    for team in wesnoth.get_teams():
                        # does it alreadey belong to use or an ally?
                        if team.owns_village(location) and not team.is_enemy:
                            break
                    else:
                        # no, either it belongs to an enemy or to nobody
                        villages.append(location)

        return villages

    def find_keeps(self):
        """Find keep locations."""
        keeps = []
        m = wesnoth.get_map()
        for x in range(m.x):
            for y in range(m.y):
                location = wesnoth.get_location(x, y)
                if wesnoth.get_map().is_keep(location):
                    keeps.append(location)
        return keeps

    def get_distance(self, location, target, must_reach = False):
        """Find out how many turns it takes the unit at location to reach target."""
        if location == target: return 0
        unit = wesnoth.get_units()[location]
        path = unit.find_path(location, target, 100)
        extra = 0
        if not path:
            extra = 1
            if must_reach: return None
            for adjacent in wesnoth.get_adjacent_tiles(target):
                # Consider 5 turns worth of movement of this unit.
                path = unit.find_path(location, adjacent,
                    unit.type().movement * 5)
                if path: break
            else:
                return None
        l = 0
        for location in path:
            l += unit.movement_cost(location)
        l -= unit.movement_left
        l /= unit.type().movement
        l += 1 + extra
        return l

    def attack(self, location, enemy):
        """Attack an enemy unit."""
        wesnoth.attack_unit(location, enemy)

    def go_to(self, location, target, must_reach = False):
        """Make a unit at the given location go to the given target.
        Returns the reached position.
        """
        if location == target: return location

        # If target is occupied, try to go near it
        unit_locations = wesnoth.get_units().keys()
        if target in unit_locations:
            if must_reach: return location
            adjacent = wesnoth.get_adjacent_tiles(target)
            targets = [x for x in adjacent if not x in unit_locations]
            if targets:
                target = targets[0]
            else:
                return location

        # find a path
        for l, unit in wesnoth.get_units().iteritems():
            if location ==  l:
                path = unit.find_path(location, target, unit.type().movement * 5)
                break
        else:
            return location

        if path:
            possible_destinations = wesnoth.get_destinations_by_unit().get(location, [])
            if must_reach:
                if not target in path: return location
                if not target in possible_destinations: return location

            # find first reachable position in reversed path
            path.reverse()

            for p in path:
                if p in possible_destinations and not p in unit_locations:
                    location = wesnoth.move_unit(location, p)
                    return location
        return location

AI()
