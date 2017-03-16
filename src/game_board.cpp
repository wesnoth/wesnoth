/*
   Copyright (C) 2014 - 2016 by Chris Beck <render787@gmail.com>
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
#include "map/map.hpp"
#include "recall_list_manager.hpp"
#include "terrain/type_data.hpp"
#include "units/unit.hpp"

#include <set>
#include <vector>

static lg::log_domain log_engine("enginerefac");
#define DBG_RG LOG_STREAM(debug, log_engine)
#define LOG_RG LOG_STREAM(info, log_engine)
#define WRN_RG LOG_STREAM(warn, log_engine)
#define ERR_RG LOG_STREAM(err, log_engine)

static lg::log_domain log_engine_enemies("engine/enemies");
#define DBG_EE LOG_STREAM(debug, log_engine_enemies)

game_board::game_board(const ter_data_cache & tdata, const config & level)
	: teams_()
	, map_(new gamemap(tdata, level["map_data"]))
	, unit_id_manager_(level["next_underlying_unit_id"])
	, units_()
{
}

game_board::game_board(const game_board & other)
	: teams_(other.teams_)
	, map_(new gamemap(*(other.map_)))
	, labels_(other.labels_)
	, unit_id_manager_(other.unit_id_manager_)
	, units_(other.units_) {}

game_board::~game_board() {}


//TODO: Fix this so that we swap pointers to maps
// However, then anytime gameboard is overwritten, resources::gamemap must be updated. So might want to
// just get rid of resources::gamemap and replace with resources::gameboard->map() at that point.
void swap(game_board & one, game_board & other) {
	std::swap(one.teams_, other.teams_);
	std::swap(one.units_, other.units_);
	std::swap(one.unit_id_manager_, other.unit_id_manager_);
	one.map_.swap(other.map_);
}

game_board & game_board::operator= (game_board other)
{
	swap(*this, other);
	return(*this);
}

void game_board::new_turn(int player_num) {
	for (unit & i : units_) {
		if (i.side() == player_num) {
			i.new_turn();
		}
	}
}

void game_board::end_turn(int player_num) {
	for (unit & i : units_) {
		if (i.side() == player_num) {
			i.end_turn();
		}
	}
}

void game_board::set_all_units_user_end_turn() {
	for (unit & i : units_) {
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

void game_board::check_victory(bool & continue_level, bool & found_player, bool & found_network_player, bool & cleared_villages, std::set<unsigned> & not_defeated, bool remove_from_carryover_on_defeat)
{
	continue_level = true;
	found_player = false;
	found_network_player = false;
	cleared_villages = false;

	not_defeated = std::set<unsigned>();

	for (const unit & i : units())
	{
		DBG_EE << "Found a unit: " << i.id() << " on side " << i.side() << std::endl;
		const team& tm = teams()[i.side()-1];
		DBG_EE << "That team's defeat condition is: " << tm.defeat_condition() << std::endl;
		if (i.can_recruit() && tm.defeat_condition() == team::DEFEAT_CONDITION::NO_LEADER) {
			not_defeated.insert(i.side());
		} else if (tm.defeat_condition() == team::DEFEAT_CONDITION::NO_UNITS) {
			not_defeated.insert(i.side());
		}
	}

	for (team& tm : teams_)
	{
		if(tm.defeat_condition() == team::DEFEAT_CONDITION::NEVER)
		{
			not_defeated.insert(tm.side());
		}
		// Clear villages for teams that have no leader and
		// mark side as lost if it should be removed from carryover.
		if (not_defeated.find(tm.side()) == not_defeated.end())
		{
			tm.clear_villages();
			// invalidate_all() is overkill and expensive but this code is
			// run rarely so do it the expensive way.
			cleared_villages = true;

			if (remove_from_carryover_on_defeat)
			{
				tm.set_lost(true);
			}
		}
		else if(remove_from_carryover_on_defeat)
		{
			tm.set_lost(false);
		}
	}

	for (std::set<unsigned>::iterator n = not_defeated.begin(); n != not_defeated.end(); ++n) {
		size_t side = *n - 1;

		DBG_EE << "Side " << (side+1) << " is a not-defeated team" << std::endl;

		std::set<unsigned>::iterator m(n);
		for (++m; m != not_defeated.end(); ++m) {
			if (teams()[side].is_enemy(*m)) {
				return;
			}
			DBG_EE << "Side " << (side+1) << " and " << *m << " are not enemies." << std::endl;
		}

		if (teams()[side].is_local_human()) {
			found_player = true;
		}

		if (teams()[side].is_network_human()) {
			found_network_player = true;
		}
	}

	continue_level = false;
}

unit_map::iterator game_board::find_visible_unit(const map_location &loc,
	const team& current_team, bool see_all)
{
	if (!map_->on_board(loc)) return units_.end();
	unit_map::iterator u = units_.find(loc);
	if (!u.valid() || !u->is_visible_to_team(current_team, *this, see_all))
		return units_.end();
	return u;
}

bool game_board::has_visible_unit(const map_location & loc, const team& current_team, bool see_all) const
{
	if (!map_->on_board(loc)) return false;
	unit_map::const_iterator u = units_.find(loc);
	if (!u.valid() || !u->is_visible_to_team(current_team, *this, see_all))
		return false;
	return true;
}

unit* game_board::get_visible_unit(const map_location &loc,
	const team &current_team, bool see_all)
{
	unit_map::iterator ui = find_visible_unit(loc, current_team, see_all);
	if (ui == units_.end()) return nullptr;
	return &*ui;
}

void game_board::side_drop_to(int side_num, team::CONTROLLER ctrl, team::PROXY_CONTROLLER proxy) {
	team &tm = teams_[side_num-1];

	tm.change_controller(ctrl);
	tm.change_proxy(proxy);
	tm.set_local(true);

	tm.set_current_player(ctrl.to_string() + std::to_string(side_num));

	unit_map::iterator leader = units_.find_leader(side_num);
	if (leader.valid()) leader->rename(ctrl.to_string() + std::to_string(side_num));
}

void game_board::side_change_controller(int side_num, bool is_local, const std::string& pname) {
	team &tm = teams_[side_num-1];

	tm.set_local(is_local);

	if (pname.empty() || !tm.is_human()) {
		return;
	}

	tm.set_current_player(pname);

	unit_map::iterator leader = units_.find_leader(side_num);
	if (leader.valid()) {
		leader->rename(pname);
	}
}

bool game_board::team_is_defeated(const team& t) const
{
	switch(t.defeat_condition().v)
	{
	case team::DEFEAT_CONDITION::ALWAYS:
		return true;
	case team::DEFEAT_CONDITION::NO_LEADER:
		return !units_.find_leader(t.side()).valid();
	case team::DEFEAT_CONDITION::NO_UNITS:
		for (const unit& u : units_)
		{
			if(u.side() == t.side())
				return false;
		}
		return true;
	case team::DEFEAT_CONDITION::NEVER:
	default:
		return false;
	}
}

bool game_board::try_add_unit_to_recall_list(const map_location&, const unit_ptr u)
{
	teams_[u->side()-1].recall_list().add(u);
	return true;
}


boost::optional<std::string> game_board::replace_map(const gamemap & newmap) {
	boost::optional<std::string> ret = boost::optional<std::string> ();

	/* Remember the locations where a village is owned by a side. */
	std::map<map_location, int> villages;
	for(const auto& village : map_->villages()) {
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
	for(const auto& village : villages) {
		if(!newmap.is_village(village.first)) {
			teams_[village.second].lose_village(village.first);
		}
	}

	*map_ = newmap;
	return ret;
}



void game_board::overlay_map(const gamemap & mask_map, const config & cfg, map_location loc) {
	map_->overlay(mask_map, cfg, loc);
}

bool game_board::change_terrain(const map_location &loc, const std::string &t_str,
                    const std::string & mode_str, bool replace_if_failed)
{
	//Code internalized from the implementation in lua.cpp
	t_translation::terrain_code terrain = t_translation::read_terrain_code(t_str);
	if (terrain == t_translation::NONE_TERRAIN) return false;

	terrain_type_data::merge_mode mode = terrain_type_data::BOTH;

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

	t_translation::terrain_code
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

	for(const t_translation::terrain_code &ut : map_->underlying_union_terrain(loc)) {
		preferences::encountered_terrains().insert(ut);
	}
	return true;
}

void game_board::write_config(config & cfg) const
{
	cfg["next_underlying_unit_id"] = unit_id_manager_.get_save_id();
	for(std::vector<team>::const_iterator t = teams_.begin(); t != teams_.end(); ++t) {
		int side_num = t - teams_.begin() + 1;

		config& side = cfg.add_child("side");
		t->write(side);
		side["no_leader"] = true;
		side["side"] = std::to_string(side_num);

		//current units
		{
			for (const unit & i : units_) {
				if (i.side() == side_num) {
					config& u = side.add_child("unit");
					i.get_location().write(u);
					i.write(u);
				}
			}
		}
		//recall list
		{
			for (const unit_const_ptr & j : t->recall_list()) {
				config& u = side.add_child("unit");
				j->write(u);
			}
		}
	}

	//write the map
	cfg["map_data"] = map_->write();
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

