/* $Id$ */
/*
   Copyright (C) 2003 - 2007 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#ifndef GAME_EVENTS_H_INCLUDED
#define GAME_EVENTS_H_INCLUDED

class config;
class game_data;
class game_display;
class game_state;
class gamestatus;
class team;
class t_string;
class unit;

#include "map.hpp"
#include "soundsource.hpp"
#include "variable.hpp"
#include "unit_map.hpp"

#include <vector>
#include <map>


//! @file game_events.hpp
//! Define the game's events mechanism.
//
// Events might be units moving or fighting, or when victory or defeat occurs.
// A scenario's configuration file will define actions to take when certain events occur.
// This module is responsible for making sure that when the events occur, the actions take place.
//
// Note that game events have nothing to do with SDL events,
// like mouse movement, keyboard events, etc.
// See events.hpp for how they are handled.

namespace game_events
{
// The game event manager loads the scenario configuration object,
// and ensures that events are handled according to the
// scenario configuration for its lifetime.
//
// Thus, a manager object should be created when a scenario is played,
// and destroyed at the end of the scenario.
struct manager {
	// Note that references will be maintained,
	// and must remain valid for the life of the object.
	manager(const config& scenario_cfg, game_display& disp, gamemap& map,
			soundsource::manager& sndsources, unit_map& units, std::vector<team>& teams,
			game_state& state_of_game, gamestatus& status, const game_data& data);
	~manager();

	variable::manager variable_manager;
};

struct entity_location : public gamemap::location {
	entity_location(gamemap::location loc, const std::string& id="");
	explicit entity_location(unit_map::iterator itor);
	bool requires_unit() const;
	bool matches_unit(const unit& u) const;
private:
	std::string id_;
};

game_state* get_state_of_game();
void write_events(config& cfg);
void add_events(const config::child_list& cfgs,const std::string& id);

bool unit_matches_filter(unit_map::const_iterator itor, const vconfig filter);

//! Function to fire an event.
// Events may have up to two arguments, both of which must be locations.
bool fire(const std::string& event,
          const entity_location& loc1=gamemap::location::null_location,
          const entity_location& loc2=gamemap::location::null_location,
		  const config& data=config());

void raise(const std::string& event,
          const entity_location& loc1=gamemap::location::null_location,
          const entity_location& loc2=gamemap::location::null_location,
		  const config& data=config());

bool conditional_passed(const unit_map* units,
                        const vconfig cond, bool backwards_compat=true);
bool pump();

// The count of game event mutations to the unit_map
Uint32 mutations();

} // end namespace game_events

#endif
