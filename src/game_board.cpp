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
#include "game_preferences.hpp"
#include "log.hpp"
#include "unit.hpp"

#include "utils/foreach.tpp"

#include <boost/foreach.hpp>

static lg::log_domain log_engine("enginerefac");
#define DBG_RG LOG_STREAM(debug, log_engine)
#define LOG_RG LOG_STREAM(info, log_engine)
#define WRN_RG LOG_STREAM(warn, log_engine)
#define ERR_RG LOG_STREAM(err, log_engine)


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

void game_board::side_drop_to(int side_num, team::CONTROLLER ctrl) {
	team &tm = teams_[side_num-1];

	tm.change_controller(ctrl);

	tm.set_current_player(lexical_cast<std::string> (ctrl) + lexical_cast<std::string> (side_num));

	unit_map::iterator leader = units_.find_leader(side_num);
	if (leader.valid()) leader->rename(lexical_cast<std::string> (ctrl) + lexical_cast<std::string> (side_num));
}

void game_board::side_change_controller(int side_num, team::CONTROLLER ctrl, const std::string pname) {
	team &tm = teams_[side_num-1];

	tm.change_controller(ctrl);

	if (pname.empty()) {
		return ;
	}

	tm.set_current_player(pname);

	unit_map::iterator leader = units_.find_leader(side_num);
	if (leader.valid()) {
		leader->rename(pname);
	}
}

bool game_board::try_add_unit_to_recall_list(const map_location& loc, const unit& u)
{
	if(teams_[u.side()-1].persistent()) {
		teams_[u.side()-1].recall_list().push_back(u);
		return true;
	} else {
		ERR_RG << "unit with id " << u.id() << ": location (" << loc.x << "," << loc.y <<") is not on the map, and player "
			<< u.side() << " has no recall list.\n";
		return false;
	}
}


boost::optional<std::string> game_board::replace_map(const gamemap & newmap) {
	boost::optional<std::string> ret = boost::optional<std::string> ();

	/* Remember the locations where a village is owned by a side. */
	std::map<map_location, int> villages;
	FOREACH(const AUTO& village, map_.villages()) {
		const int owner = village_owner(village);
		if(owner != -1) {
			villages[village] = owner;
		}
	}

	for (unit_map::iterator itor = units_.begin(); itor != units_.end(); ) {
		if (!newmap.on_board(itor->get_location())) {
			if (!try_add_unit_to_recall_list(itor->get_location(), *itor)) {
				*ret = std::string("replace_map: Cannot add a unit that would become off-map to the recall list\n");
			}
			units_.erase(itor++);
		} else {
			++itor;
		}
	}

	/* Disown villages that are no longer villages. */
	FOREACH(const AUTO& village, villages) {
		if(!newmap.is_village(village.first)) {
			teams_[village.second].lose_village(village.first);
		}
	}

	map_ = newmap;
	return ret;
}



void game_board::overlay_map(const gamemap & mask_map, const config & cfg, map_location loc, bool border) {
	map_.overlay(mask_map, cfg, loc.x, loc.y, border);
}

bool game_board::change_terrain(const map_location &loc, const t_translation::t_terrain &t,
                    gamemap::tmerge_mode mode, bool replace_if_failed)
{
	/*
	 * When a hex changes from a village terrain to a non-village terrain, and
	 * a team owned that village it loses that village. When a hex changes from
	 * a non-village terrain to a village terrain and there is a unit on that
	 * hex it does not automatically capture the village. The reason for not
	 * capturing villages it that there are too many choices to make; should a
	 * unit loose its movement points, should capture events be fired. It is
	 * easier to do this as wanted by the author in WML.
	 */

	t_translation::t_terrain
		old_t = map_.get_terrain(loc),
		new_t = map_.merge_terrains(old_t, t, mode, replace_if_failed);
	if (new_t == t_translation::NONE_TERRAIN) return false;
	preferences::encountered_terrains().insert(new_t);

	if (map_.is_village(old_t) && !map_.is_village(new_t)) {
		int owner = village_owner(loc);
		if (owner != -1)
			teams_[owner].lose_village(loc);
	}

	map_.set_terrain(loc, new_t);

	BOOST_FOREACH(const t_translation::t_terrain &ut, map_.underlying_union_terrain(loc)) {
		preferences::encountered_terrains().insert(ut);
	}
	return true;
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
