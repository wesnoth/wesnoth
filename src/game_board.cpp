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
#include "map.hpp"
#include "recall_list_manager.hpp"
#include "terrain_type_data.hpp"
#include "unit.hpp"

#include "utils/foreach.tpp"

#include <boost/foreach.hpp>

static lg::log_domain log_engine("enginerefac");
#define DBG_RG LOG_STREAM(debug, log_engine)
#define LOG_RG LOG_STREAM(info, log_engine)
#define WRN_RG LOG_STREAM(warn, log_engine)
#define ERR_RG LOG_STREAM(err, log_engine)

game_board::game_board(const tdata_cache & tdata, const config & level) : teams_(), map_(new gamemap(tdata, level)), units_() {}

game_board::game_board(const game_board & other) : teams_(other.teams_), map_(new gamemap(*(other.map_))), units_(other.units_) {}

game_board::~game_board() {}


//TODO: Fix this so that we swap pointers to maps, and also fix replace_map to use scoped_ptr::reset.
// However, then anytime gameboard is overwritten, resources::gamemap must be updated. So might want to
// just get rid of resources::gamemap and replace with resources::gameboard->map() at that point.
void swap(game_board & one, game_board & other) {
	std::swap(one.teams_, other.teams_);
	std::swap(one.units_, other.units_);
	one.map_.swap(other.map_);
}

game_board & game_board::operator= (game_board other)
{
	swap(*this, other);
	return(*this);
}

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

void game_board::heal_all_survivors() {
	for (unit_map::iterator it = units_.begin(); it != units_.end(); it++) {
		unit_ptr un =  it.get_shared_ptr();
		if (teams_[un->side() - 1].persistent()) {
			un->new_turn();
			un->new_scenario();
		}
	}
}

unit_map::iterator game_board::find_visible_unit(const map_location &loc,
	const team& current_team, bool see_all)
{
	if (!map_->on_board(loc)) return units_.end();
	unit_map::iterator u = units_.find(loc);
	if (!u.valid() || !u->is_visible_to_team(current_team, *map_, see_all))
		return units_.end();
	return u;
}

bool game_board::has_visible_unit(const map_location & loc, const team& current_team, bool see_all)
{
	if (!map_->on_board(loc)) return false;
	unit_map::iterator u = units_.find(loc);
	if (!u.valid() || !u->is_visible_to_team(current_team, *map_, see_all))
		return false;
	return true;
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

bool game_board::try_add_unit_to_recall_list(const map_location& loc, const unit_ptr u)
{
	if(teams_[u->side()-1].persistent()) {
		teams_[u->side()-1].recall_list().add(u);
		return true;
	} else {
		ERR_RG << "unit with id " << u->id() << ": location (" << loc.x << "," << loc.y <<") is not on the map, and player "
			<< u->side() << " has no recall list.\n";
		return false;
	}
}


boost::optional<std::string> game_board::replace_map(const gamemap & newmap) {
	boost::optional<std::string> ret = boost::optional<std::string> ();

	/* Remember the locations where a village is owned by a side. */
	std::map<map_location, int> villages;
	FOREACH(const AUTO& village, map_->villages()) {
		const int owner = village_owner(village);
		if(owner != -1) {
			villages[village] = owner;
		}
	}

	for (unit_map::iterator itor = units_.begin(); itor != units_.end(); ) {
		if (!newmap.on_board(itor->get_location())) {
			if (!try_add_unit_to_recall_list(itor->get_location(), itor.get_shared_ptr())) {
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

	*map_ = newmap;
	return ret;
}



void game_board::overlay_map(const gamemap & mask_map, const config & cfg, map_location loc, bool border) {
	map_->overlay(mask_map, cfg, loc.x, loc.y, border);
}

bool game_board::change_terrain(const map_location &loc, const std::string &t_str,
                    const std::string & mode_str, bool replace_if_failed)
{
	//Code internalized from the implementation in lua.cpp
	t_translation::t_terrain terrain = t_translation::read_terrain_code(t_str);
	if (terrain == t_translation::NONE_TERRAIN) return false;

	terrain_type_data::tmerge_mode mode = terrain_type_data::BOTH;

	if (mode_str == "base") mode = terrain_type_data::BASE;
	else if (mode_str == "overlay") mode = terrain_type_data::OVERLAY;

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
		old_t = map_->get_terrain(loc),
		new_t = map_->tdata()->merge_terrains(old_t, terrain, mode, replace_if_failed);
	if (new_t == t_translation::NONE_TERRAIN) return false;
	preferences::encountered_terrains().insert(new_t);

	if (map_->tdata()->is_village(old_t) && !map_->tdata()->is_village(new_t)) {
		int owner = village_owner(loc);
		if (owner != -1)
			teams_[owner].lose_village(loc);
	}

	map_->set_terrain(loc, new_t);

	BOOST_FOREACH(const t_translation::t_terrain &ut, map_->underlying_union_terrain(loc)) {
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
			BOOST_FOREACH(const unit_const_ptr & j, t->recall_list()) {
				config& u = side.add_child("unit");
				j->write(u);
			}
		}
	}

	//write the map
	cfg["map_data"] = map_->write();

	//Used by the carryover calculations.
	cfg["map_villages_num"] = map_->villages().size();
}

temporary_unit_placer::temporary_unit_placer(unit_map& m, const map_location& loc, unit& u)
	: m_(m), loc_(loc), temp_(m_.extract(loc))
{
	u.clone();
	m_.add(loc, u);
}

temporary_unit_placer::temporary_unit_placer(game_board& b, const map_location& loc, unit& u)
	: m_(b.units_), loc_(loc), temp_(m_.extract(loc))
{
	u.clone();
	m_.add(loc, u);
}

temporary_unit_placer::~temporary_unit_placer()
{
	try {
	m_.erase(loc_);
	if(temp_) {
		m_.insert(temp_);
	}
	} catch (...) {}
}

temporary_unit_remover::temporary_unit_remover(unit_map& m, const map_location& loc)
	: m_(m), loc_(loc), temp_(m_.extract(loc))
{
}

temporary_unit_remover::temporary_unit_remover(game_board& b, const map_location& loc)
	: m_(b.units_), loc_(loc), temp_(m_.extract(loc))
{
}

temporary_unit_remover::~temporary_unit_remover()
{
	try {
	if(temp_) {
		m_.insert(temp_);
	}
	} catch (...) {}
}

/**
 * Constructor
 * This version will change the unit's current movement to @a new_moves while
 * the unit is moved (and restored to its previous value upon this object's
 * destruction).
 */
temporary_unit_mover::temporary_unit_mover(unit_map& m, const map_location& src,
                                           const map_location& dst, int new_moves)
	: m_(m), src_(src), dst_(dst), old_moves_(-1),
	  temp_(src == dst ? unit_ptr() : m_.extract(dst))
{
	std::pair<unit_map::iterator, bool> move_result = m_.move(src_, dst_);

	// Set the movement.
	if ( move_result.second )
	{
		old_moves_ = move_result.first->movement_left(true);
		move_result.first->set_movement(new_moves);
	}
}

temporary_unit_mover::temporary_unit_mover(game_board& b, const map_location& src,
                                           const map_location& dst, int new_moves)
	: m_(b.units_), src_(src), dst_(dst), old_moves_(-1),
	  temp_(src == dst ? unit_ptr() : m_.extract(dst))
{
	std::pair<unit_map::iterator, bool> move_result = m_.move(src_, dst_);

	// Set the movement.
	if ( move_result.second )
	{
		old_moves_ = move_result.first->movement_left(true);
		move_result.first->set_movement(new_moves);
	}
}

/**
 * Constructor
 * This version does not change (nor restore) the unit's movement.
 */
temporary_unit_mover::temporary_unit_mover(unit_map& m, const map_location& src,
                                           const map_location& dst)
	: m_(m), src_(src), dst_(dst), old_moves_(-1),
	  temp_(src == dst ? unit_ptr() : m_.extract(dst))
{
	m_.move(src_, dst_);
}

temporary_unit_mover::temporary_unit_mover(game_board& b, const map_location& src,
                                           const map_location& dst)
	: m_(b.units_), src_(src), dst_(dst), old_moves_(-1),
	  temp_(src == dst ? unit_ptr() : m_.extract(dst))
{
	m_.move(src_, dst_);
}

temporary_unit_mover::~temporary_unit_mover()
{
	try {
	std::pair<unit_map::iterator, bool> move_result = m_.move(dst_, src_);

	// Restore the movement?
	if ( move_result.second  &&  old_moves_ >= 0 )
		move_result.first->set_movement(old_moves_);

	// Restore the extracted unit?
	if(temp_) {
		m_.insert(temp_);
	}
	} catch (...) {}
}

