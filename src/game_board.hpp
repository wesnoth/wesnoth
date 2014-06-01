/*
   Copyright (C) 2014 by Chris Beck <render787@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GAME_BOARD_HPP_INCLUDED
#define GAME_BOARD_HPP_INCLUDED

#include "global.hpp"

#include "map.hpp"
#include "team.hpp"
#include "unit_map.hpp"

#include <vector>

class config;

namespace events {
	class mouse_handler;
}

class game_board {

	std::vector<team> teams_;

	gamemap map_;
	unit_map units_;

	//TODO: Remove these when we have refactored enough to make it possible.
	friend class play_controller;
	friend class replay_controller;
	friend class playsingle_controller;
	friend class playmp_controller;
	friend class events::mouse_handler;
	
	public:

	// Constructors and const accessors

	game_board(const config & game_config, const config & level) : teams_(), map_(game_config, level), units_() {}

	const std::vector<team> & teams() const { return teams_; }
	const gamemap & map() const { return map_; }
	const unit_map & units() const { return units_; }

	// Saving

	void write_config(config & cfg) const;

	// Manipulators from play_controller

	void new_turn(int pnum);
	void end_turn(int pnum);
	void set_all_units_user_end_turn();

	// Manipulator from playturn

	void side_drop_to (int side_num, team::CONTROLLER ctrl);
	void side_change_controller (int side_num, team::CONTROLLER ctrl, const std::string pname = "");

	// Global accessor from unit.hpp

	unit_map::iterator find_visible_unit(const map_location &loc, const team& current_team, bool see_all = false);
	unit* get_visible_unit(const map_location &loc, const team &current_team, bool see_all = false); //TODO: can this not return a pointer?
};

#endif
