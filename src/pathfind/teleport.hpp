/*
   Copyright (C) 2010 - 2018 by Fabian Mueller <fabianmueller5@gmx.de>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#pragma once

#include "config.hpp"
#include "map/location.hpp"

class team;
class unit;
class vconfig;


namespace pathfind {

typedef std::pair<std::set<map_location>, std::set<map_location>>
		teleport_pair;

/*
 * Represents the tunnel wml tag.
 */
class teleport_group
{
public:
	/*
	 * Constructs the object from a saved file.
	 * @param cfg	the contents of a [tunnel] tag
	 */
	teleport_group(const config& cfg);

	/*
	 * Constructs the object from a config file.
	 * @param cfg		the contents of a [tunnel] tag
	 * @param way_back	inverts the direction of the teleport
	 */
	teleport_group(const vconfig& cfg, bool way_back = false);

	/*
	 * Fills the argument loc_pair if the unit u matches the groups filter.
	 * @param loc_pair		returned teleport_pair if the unit matches
	 * @param u				this unit must match the group's filter
	 * @param ignore_units	don't consider zoc and blocking when calculating the shorted path between
	 */
	void get_teleport_pair(
			  teleport_pair& loc_pair
			, const unit& u
			, const bool ignore_units) const;

	/*
	 * Can be set by the id attribute or is randomly chosen.
	 * @return unique id of the teleport group
	 */
	const std::string& get_teleport_id() const;

	/*
	 * Returns whether the group should always be visible,
	 * even for enemy movement under shroud.
	 * @return	visibility of the teleport group
	 */
	bool always_visible() const;

	/*
	 * Returns whether allied units on the exit hex can be passed.
	 */
	bool pass_allied_units() const;

	/*
	 * Returns whether vision through tunnels is possible.
	 */
	bool allow_vision() const;

	/** Inherited from savegame_config. */
	config to_config() const;

private:

	config cfg_; 		// unexpanded contents of a [tunnel] tag
	bool reversed_; 	// Whether the tunnel's direction is reversed
	std::string id_; 	// unique id of the group
};


class teleport_map {
public:
	/*
	 * @param teleport_groups
	 * @param u
	 * @param viewing_team
	 * @param see_all
	 * @param ignore_units
	 * @param check_vision
	 */
	teleport_map(
			  const std::vector<teleport_group>& teleport_groups
			, const unit& u
			, const team &viewing_team
			, const bool see_all
			, const bool ignore_units
			, const bool check_vision);

	/*
	 * Constructs an empty teleport map.
	 */
	teleport_map() :
		teleport_map_(), sources_(), targets_() {}

	/*
	 * @param adjacents		used to return the adjacent hexes
	 * @param loc			the map location for which we want to know the adjacent hexes
	 */
	void get_adjacents(std::set<map_location>& adjacents, map_location loc) const;
	/*
	 * @param sources	used to return the locations that are an entrance of the tunnel
	 */
	void get_sources(std::set<map_location>& sources) const;
	/*
	 * @param targets	used to return the locations that are an exit of the tunnel
	 */
	void get_targets(std::set<map_location>& targets) const;

	/*
	 * @returns whether the teleport_map does contain any defined tunnel
	 */
	bool empty() const {
		return sources_.empty();
	}

private:
	std::map<map_location, std::set<std::string>> teleport_map_;
	std::map<std::string, std::set<map_location>> sources_;
	std::map<std::string, std::set<map_location>> targets_;
};

/*
 * @param u					The unit that is processed by pathfinding
 * @param viewing_team		The team the player belongs to
 * @param see_all			Whether the teleport can be seen below shroud
 * @param ignore_units		Whether to ignore zoc and blocking by units
 * @param check_vision		Whether to check vision as opposed to movement range
 * @returns a teleport_map
 */
const teleport_map get_teleport_locations(const unit &u, const team &viewing_team,
		bool see_all = false, bool ignore_units = false, bool check_vision = false);

class manager
{
public:
	manager(const config &cfg);

	/*
	 * @param group		teleport_group to be added
	 */
	void add(const teleport_group &group);

	/*
	 * @param id		id of the teleport_group that is to be removed by the method
	 */
	void remove(const std::string &id);

	/*
	 * @return	all registered teleport groups on the game field
	 */
	const std::vector<teleport_group>& get() const;

	/** Inherited from savegame_config. */
	config to_config() const;

	/*
	 * @returns the next free unique id for a teleport group
	 */
	std::string next_unique_id();
private:
	std::vector<teleport_group> tunnels_;
	int id_;
};

}
