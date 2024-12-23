/*
	Copyright (C) 2014 - 2024
	by Chris Beck <render787@gmail.com>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#include "game_board.hpp"
#include "config.hpp"
#include "log.hpp"
#include "map/map.hpp"
#include "preferences/preferences.hpp"
#include "recall_list_manager.hpp"
#include "units/unit.hpp"
#include "units/animation_component.hpp"
#include "utils/general.hpp"

#include <set>
#include <vector>

static lg::log_domain log_engine("enginerefac");
#define DBG_RG LOG_STREAM(debug, log_engine)
#define LOG_RG LOG_STREAM(info, log_engine)
#define WRN_RG LOG_STREAM(warn, log_engine)
#define ERR_RG LOG_STREAM(err, log_engine)

static lg::log_domain log_engine_enemies("engine/enemies");
#define DBG_EE LOG_STREAM(debug, log_engine_enemies)

game_board::game_board(const config& level)
	: teams_()
	, map_(std::make_unique<gamemap>(level["map_data"]))
	, unit_id_manager_(level["next_underlying_unit_id"].to_size_t())
	, units_()
{
}

game_board::game_board(const game_board& other)
	: teams_(other.teams_)
	, labels_(other.labels_)
	, map_(new gamemap(*(other.map_)))
	, unit_id_manager_(other.unit_id_manager_)
	, units_(other.units_)
{
}

game_board::~game_board()
{
}

// TODO: Fix this so that we swap pointers to maps
// However, then anytime gameboard is overwritten, resources::gamemap must be updated. So might want to
// just get rid of resources::gamemap and replace with resources::gameboard->map() at that point.
void swap(game_board& one, game_board& other)
{
	std::swap(one.teams_, other.teams_);
	std::swap(one.units_, other.units_);
	std::swap(one.unit_id_manager_, other.unit_id_manager_);
	one.map_.swap(other.map_);
}

void game_board::new_turn(int player_num)
{
	for(unit& i : units_) {
		if(i.side() == player_num) {
			i.new_turn();
		}
	}
}

void game_board::end_turn(int player_num)
{
	for(unit& i : units_) {
		if(i.side() == player_num) {
			i.end_turn();
		}
	}
}

void game_board::set_all_units_user_end_turn()
{
	for(unit& i : units_) {
		i.set_user_end_turn(true);
	}
}

void game_board::heal_all_survivors()
{
	for(auto& u : units_) {
		if(get_team(u.side()).persistent()) {
			u.new_turn();
			u.new_scenario();
		}
	}

	for(auto& t : teams_) {
		if(t.persistent()) {
			for(auto& up : t.recall_list()) {
				up->new_scenario();
				up->new_turn();
			}
		}
	}
}

void game_board::check_victory(bool& continue_level,
	bool& found_player,
	bool& found_network_player,
	bool& cleared_villages,
	std::set<unsigned>& not_defeated,
	bool remove_from_carryover_on_defeat)
{
	continue_level = true;
	found_player = false;
	found_network_player = false;
	cleared_villages = false;

	not_defeated = std::set<unsigned>();

	for(const unit& i : units()) {
		DBG_EE << "Found a unit: " << i.id() << " on side " << i.side();
		const team& tm = get_team(i.side());
		DBG_EE << "That team's defeat condition is: " << defeat_condition::get_string(tm.defeat_cond());
		if(i.can_recruit() && tm.defeat_cond() == defeat_condition::type::no_leader_left) {
			not_defeated.insert(i.side());
		} else if(tm.defeat_cond() == defeat_condition::type::no_units_left) {
			not_defeated.insert(i.side());
		}
	}

	for(team& tm : teams_) {
		if(tm.defeat_cond() == defeat_condition::type::never) {
			not_defeated.insert(tm.side());
		}

		// Clear villages for teams that have no leader and
		// mark side as lost if it should be removed from carryover.
		if(not_defeated.find(tm.side()) == not_defeated.end()) {
			tm.clear_villages();
			// invalidate_all() is overkill and expensive but this code is
			// run rarely so do it the expensive way.
			cleared_villages = true;

			if(remove_from_carryover_on_defeat) {
				tm.set_lost(true);
			}
		} else if(remove_from_carryover_on_defeat) {
			tm.set_lost(false);
		}
	}

	for(std::set<unsigned>::iterator n = not_defeated.begin(); n != not_defeated.end(); ++n) {
		std::size_t side = *n - 1;
		DBG_EE << "Side " << (side + 1) << " is a not-defeated team";

		std::set<unsigned>::iterator m(n);
		for(++m; m != not_defeated.end(); ++m) {
			if(teams()[side].is_enemy(*m)) {
				return;
			}

			DBG_EE << "Side " << (side + 1) << " and " << *m << " are not enemies.";
		}

		if(teams()[side].is_local_human()) {
			found_player = true;
		}

		if(teams()[side].is_network_human()) {
			found_network_player = true;
		}
	}

	continue_level = false;
}

unit_map::iterator game_board::find_visible_unit(const map_location& loc, const team& current_team, bool see_all)
{
	if(!map_->on_board(loc)) {
		return units_.end();
	}

	unit_map::iterator u = units_.find(loc);
	if(!u.valid() || !u->is_visible_to_team(current_team, see_all)) {
		return units_.end();
	}

	return u;
}

bool game_board::has_visible_unit(const map_location& loc, const team& current_team, bool see_all) const
{
	if(!map_->on_board(loc)) {
		return false;
	}

	unit_map::const_iterator u = units_.find(loc);
	if(!u.valid() || !u->is_visible_to_team(current_team, see_all)) {
		return false;
	}

	return true;
}

void game_board::side_drop_to(int side_num, side_controller::type ctrl, side_proxy_controller::type proxy)
{
	team& tm = get_team(side_num);

	tm.change_controller(ctrl);
	tm.change_proxy(proxy);
	tm.set_local(true);

	tm.set_current_player(side_controller::get_string(ctrl) + std::to_string(side_num));

	unit_map::iterator leader = units_.find_leader(side_num);
	if(leader.valid()) {
		leader->rename(side_controller::get_string(ctrl) + std::to_string(side_num));
	}
}

void game_board::side_change_controller(
	int side_num, bool is_local, const std::string& pname, const std::string& controller_type)
{
	team& tm = get_team(side_num);

	tm.set_local(is_local);

	// only changing the type of controller
	if(controller_type == side_controller::ai && !tm.is_ai()) {
		tm.make_ai();
		return;
	} else if(controller_type == side_controller::human && !tm.is_human()) {
		tm.make_human();
		return;
	}

	if(pname.empty() || !tm.is_human()) {
		return;
	}

	tm.set_current_player(pname);

	unit_map::iterator leader = units_.find_leader(side_num);
	if(leader.valid()) {
		leader->rename(pname);
	}
}

bool game_board::team_is_defeated(const team& t) const
{
	switch(t.defeat_cond()) {
	case defeat_condition::type::always:
		return true;
	case defeat_condition::type::no_leader_left:
		return !units_.find_leader(t.side()).valid();
	case defeat_condition::type::no_units_left:
		for(const unit& u : units_) {
			if(u.side() == t.side())
				return false;
		}
		return true;
	case defeat_condition::type::never:
	default:
		return false;
	}
}

bool game_board::try_add_unit_to_recall_list(const map_location&, const unit_ptr& u)
{
	get_team(u->side()).recall_list().add(u);
	return true;
}

utils::optional<std::string> game_board::replace_map(const gamemap& newmap)
{
	utils::optional<std::string> ret;

	/* Remember the locations where a village is owned by a side. */
	std::map<map_location, int> villages;
	for(const auto& village : map_->villages()) {
		const int owner = village_owner(village);
		if(owner != 0) {
			villages[village] = owner;
		}
	}

	for(unit_map::iterator itor = units_.begin(); itor != units_.end();) {
		if(!newmap.on_board(itor->get_location())) {
			if(!try_add_unit_to_recall_list(itor->get_location(), itor.get_shared_ptr())) {
				ret = std::string("replace_map: Cannot add a unit that would become off-map to the recall list\n");
			}
			units_.erase(itor++);
		} else {
			++itor;
		}
	}

	/* Disown villages that are no longer villages. */
	for(const auto& village : villages) {
		if(!newmap.is_village(village.first)) {
			get_team(village.second).lose_village(village.first);
		}
	}

	*map_ = newmap;
	return ret;
}

bool game_board::change_terrain(
	const map_location& loc, const std::string& t_str, const std::string& mode_str, bool replace_if_failed)
{
	// Code internalized from the implementation in lua.cpp
	t_translation::terrain_code terrain = t_translation::read_terrain_code(t_str);
	if(terrain == t_translation::NONE_TERRAIN) {
		return false;
	}

	terrain_type_data::merge_mode mode = terrain_type_data::BOTH;

	if(mode_str == "base") {
		mode = terrain_type_data::BASE;
	} else if(mode_str == "overlay") {
		mode = terrain_type_data::OVERLAY;
	}

	return change_terrain(loc, terrain, mode, replace_if_failed);
}

bool game_board::change_terrain(const map_location &loc, const t_translation::terrain_code &terrain, terrain_type_data::merge_mode& mode, bool replace_if_failed) {
	/*
	 * When a hex changes from a village terrain to a non-village terrain, and
	 * a team owned that village it loses that village. When a hex changes from
	 * a non-village terrain to a village terrain and there is a unit on that
	 * hex it does not automatically capture the village. The reason for not
	 * capturing villages it that there are too many choices to make; should a
	 * unit loose its movement points, should capture events be fired. It is
	 * easier to do this as wanted by the author in WML.
	 */
	t_translation::terrain_code old_t = map_->get_terrain(loc);
	t_translation::terrain_code new_t = map_->tdata()->merge_terrains(old_t, terrain, mode, replace_if_failed);

	if(new_t == t_translation::NONE_TERRAIN) {
		return false;
	}

	prefs::get().encountered_terrains().insert(new_t);

	if(map_->tdata()->is_village(old_t) && !map_->tdata()->is_village(new_t)) {
		int owner = village_owner(loc);
		if(owner != 0)
			get_team(owner).lose_village(loc);
	}

	map_->set_terrain(loc, new_t);

	for(const t_translation::terrain_code& ut : map_->underlying_union_terrain(loc)) {
		prefs::get().encountered_terrains().insert(ut);
	}

	return true;
}

void game_board::write_config(config& cfg) const
{
	cfg["next_underlying_unit_id"] = unit_id_manager_.get_save_id();

	for(const team& t : teams_) {
		config& side = cfg.add_child("side");
		t.write(side);
		side["no_leader"] = true;
		side["side"] = std::to_string(t.side());

		// current units
		for(const unit& i : units_) {
			if(i.side() == t.side()) {
				config& u = side.add_child("unit");
				i.get_location().write(u);
				i.write(u, false);
			}
		}

		// recall list
		for(const unit_const_ptr j : t.recall_list()) {
			config& u = side.add_child("unit");
			j->write(u);
		}
	}

	// write the map
	cfg["map_data"] = map_->write();
}

temporary_unit_placer::temporary_unit_placer(unit_map& m, const map_location& loc, unit& u)
	: m_(m)
	, loc_(loc)
	, temp_(m_.extract(loc))
{
	u.mark_clone(true);
	m_.add(loc, u);
}

temporary_unit_placer::temporary_unit_placer(game_board& b, const map_location& loc, unit& u)
	: m_(b.units_)
	, loc_(loc)
	, temp_(m_.extract(loc))
{
	u.mark_clone(true);
	m_.add(loc, u);
}

temporary_unit_placer::~temporary_unit_placer()
{
	try {
		m_.erase(loc_);
		if(temp_) {
			m_.insert(temp_);
		}
	} catch(...) {
		DBG_RG << "Caught exception in temporary_unit_placer destructor: " << utils::get_unknown_exception_type();
	}
}

temporary_unit_remover::temporary_unit_remover(unit_map& m, const map_location& loc)
	: m_(m)
	, loc_(loc)
	, temp_(m_.extract(loc))
{
}

temporary_unit_remover::temporary_unit_remover(game_board& b, const map_location& loc)
	: m_(b.units_)
	, loc_(loc)
	, temp_(m_.extract(loc))
{
}

temporary_unit_remover::~temporary_unit_remover()
{
	try {
		if(temp_) {
			m_.insert(temp_);
		}
	} catch(...) {
		DBG_RG << "Caught exception in temporary_unit_remover destructor: " << utils::get_unknown_exception_type();
	}
}

/**
 * Constructor
 * This version will change the unit's current movement to @a new_moves while
 * the unit is moved (and restored to its previous value upon this object's
 * destruction).
 */
temporary_unit_mover::temporary_unit_mover(unit_map& m, const map_location& src, const map_location& dst, int new_moves, bool stand)
	: m_(m)
	, src_(src)
	, dst_(dst)
	, old_moves_(-1)
	, temp_(src == dst ? unit_ptr() : m_.extract(dst))
	, stand_(stand)
{
	auto [iter, success] = m_.move(src_, dst_);

	// Set the movement.
	if(success) {
		old_moves_ = iter->movement_left(true);
		iter->set_movement(new_moves);
		if(stand_) {
			m_.find_unit_ptr(dst_)->anim_comp().set_standing();
		}
	}
}

temporary_unit_mover::~temporary_unit_mover()
{
	try {
		auto [iter, success] = m_.move(dst_, src_);

		// Restore the movement?
		if(success && old_moves_ >= 0) {
			iter->set_movement(old_moves_);
			if(stand_) {
				m_.find_unit_ptr(src_)->anim_comp().set_standing();
			}
		}

		// Restore the extracted unit?
		if(temp_) {
			m_.insert(temp_);
		}
	} catch(...) {
		DBG_RG << "Caught exception in temporary_unit_mover destructor: " << utils::get_unknown_exception_type();
	}
}
