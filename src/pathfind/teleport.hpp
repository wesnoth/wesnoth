/* $Id: teleport.hpp $ */
/*
   Copyright (C) 2010 by Fabian Mueller <fabianmueller5@gmx.de>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef TELEPORT_H_INCLUDED
#define TELEPORT_H_INCLUDED

#include "map.hpp"

#include "config.hpp"
#include "terrain_filter.hpp"
#include "savegame_config.hpp"
#include "pathfind/pathfind.hpp"


namespace pathfind {

typedef std::pair<std::set<map_location>, std::set<map_location> >
		teleport_pair;

/*
 * Represents the tunnel wml tag.
 */
class teleport_group: public savegame::savegame_config {
public:
	/*
	 * Used to create the object from a saved file.
	 */
	teleport_group(const config cfg);
	/*
	 *
	 */
	teleport_group(vconfig cfg, bool way_back = false);

	/*
	 * Fills the argument loc_pair if the unit u matches the groups filter.
	 */
	void get_teleport_pair(teleport_pair& loc_pair, const unit& u, bool ignore_units);
	/*
	 * Returns the unique id of the teleport group.
	 * Can be set by the id attribute or is randomly chosen.
	 */
	const std::string& get_teleport_id() const;

	/*
	 * Returns whether the group should always be visible,
	 * even for enemy movement under shroud.
	 */
	bool always_visible() const;

	config to_config() const;

private:

	vconfig cfg_;
	bool reversed_;
	std::string id_;
};


class teleport_map {
public:
	teleport_map(std::vector<teleport_group> teleport_groups, const unit& u,
			const unit_map &units, const team &viewing_team, bool see_all,
			bool ignore_units);
	teleport_map() :
		teleport_map_(), sources_(), targets_() {
	}
	;

	void get_adjacents(std::set<map_location>& adjacents, map_location loc) const;
	void get_sources(std::set<map_location>& sources) const;
	void get_targets(std::set<map_location>& targets) const;

	bool empty() const {
		return sources_.empty();
	}

private:
	std::map<map_location, std::set<std::string> > teleport_map_;
	std::map<std::string, std::set<map_location> > sources_;
	std::map<std::string, std::set<map_location> > targets_;
};

//TODO clean up the interface
const teleport_map get_teleport_locations(const unit &u, const unit_map &units,
		const team &viewing_team, bool see_all = false, bool ignore_units =
				false);

class manager: public savegame::savegame_config {
public:
	manager(const config &cfg);

	void add(const teleport_group &group);
	void remove(const std::string &id);
	const std::vector<teleport_group>& get() const;

	config to_config() const;

	// For unique tunnel IDs
	std::string next_unique_id();
private:
	std::vector<teleport_group> tunnels_;
	int id_;
};

}

#endif /* TELEPORT_H_INCLUDED */
