#!WPY
import ai as wesnoth

class AI:
    def __init__(self):
        self.do()

    def do(self):
        # loop over all enemy units
        for enemy_loc, ed in wesnoth.get_enemy_destinations_by_unit().iteritems():
            target_unit = wesnoth.get_units()[enemy_loc]
            # see if unit is the leader of player's side
            if target_unit.side == 1 and target_unit.can_recruit == 1:
                # if so, get adjacent locations
                for unit_loc, destinations in wesnoth.get_destinations_by_unit().iteritems():
                    attacked_flag = False
                    for destination in destinations:
                        if destination.adjacent_to(enemy_loc):
                            wesnoth.move_unit(unit_loc, destination)
                            wesnoth.attack_unit(destination, enemy_loc)
                            attacked_flag = True
                            break
                    if (not attacked_flag):
                        new_loc = self.go_to(unit_loc, enemy_loc, False)
                        if new_loc.adjacent_to(enemy_loc):
                            wesnoth.attack_unit(new_loc, enemy_loc)
                            
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
                path = unit.find_path(location, target, 1000)
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
