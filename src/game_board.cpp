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

#include "config.hpp"
#include "game_board.hpp"
#include "unit.hpp"

#include <boost/foreach.hpp>


void game_board::new_turn(int player_num) {
	BOOST_FOREACH (unit & i, units_) {
		if (i.side() == player_num) {
			i.new_turn();
		}
	}
}

void game_board::end_turn(int player_num) {
	BOOST_FOREACH (unit & i, units_) {
		if (i.side() == player_num) {
			i.end_turn();
		}
	}
}

void game_board::set_all_units_user_end_turn() {
	BOOST_FOREACH (unit & i, units_) {
		i.set_user_end_turn(true);
	}
}

unit_map::iterator game_board::find_visible_unit(const map_location &loc,
	const team& current_team, bool see_all)
{
	if (!map_.on_board(loc)) return units_.end();
	unit_map::iterator u = units_.find(loc);
	if (!u.valid() || !u->is_visible_to_team(current_team, see_all))
		return units_.end();
	return u;
}

unit* game_board::get_visible_unit(const map_location &loc,
	const team &current_team, bool see_all)
{
	unit_map::iterator ui = find_visible_unit(loc, current_team, see_all);
	if (ui == units_.end()) return NULL;
	return &*ui;
}

void game_board::write_config(config & cfg) const {
	for(std::vector<team>::const_iterator t = teams_.begin(); t != teams_.end(); ++t) {
		int side_num = t - teams_.begin() + 1;

		config& side = cfg.add_child("side");
		t->write(side);
		side["no_leader"] = true;
		side["side"] = str_cast(side_num);

		//current units
		{
			BOOST_FOREACH(const unit & i, units_) {
				if (i.side() == side_num) {
					config& u = side.add_child("unit");
					i.get_location().write(u);
					i.write(u);
				}
			}
		}
		//recall list
		{
			BOOST_FOREACH(const unit & j, t->recall_list()) {
				config& u = side.add_child("unit");
				j.write(u);
			}
		}
	}

	//write the map
	cfg["map_data"] = map_.write();
}
