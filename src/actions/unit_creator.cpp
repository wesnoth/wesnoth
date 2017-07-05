/*
   Copyright (C) 2003 - 2018 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * @file
 * Recruiting, recalling.
 */

#include "actions/unit_creator.hpp"

#include "actions/move.hpp" //for actions::get_village

#include "config.hpp"
#include "display.hpp"
#include "filter_context.hpp"
#include "game_board.hpp"
#include "game_events/pump.hpp"
#include "preferences/game.hpp"
#include "game_data.hpp" // for resources::gamedata conversion variable_set
#include "gettext.hpp"
#include "log.hpp"
#include "map/map.hpp"
#include "pathfind/pathfind.hpp"
#include "resources.hpp" // for resources::gamedata
#include "team.hpp" //for team
#include "units/unit.hpp" // for unit
#include "units/udisplay.hpp" // for unit_display
#include "variable.hpp" // for vconfig
#include "deprecation.hpp"

static lg::log_domain log_engine("engine");
#define DBG_NG LOG_STREAM(debug, log_engine)
#define LOG_NG LOG_STREAM(info, log_engine)
#define WARN_NG LOG_STREAM(warn, log_engine)
#define ERR_NG LOG_STREAM(err, log_engine)

unit_creator::unit_creator(team &tm, const map_location &start_pos, game_board* board)
  : add_to_recall_(false)
  , discover_(false)
  , get_village_(false)
  , invalidate_(false)
  , rename_side_(false)
  , show_(false)
  , start_pos_(start_pos)
  , team_(tm)
  , board_(board ? board : resources::gameboard)
{
}


unit_creator& unit_creator::allow_show(bool b)
{
	show_ = b;
	return *this;
}


unit_creator& unit_creator::allow_get_village(bool b)
{
	get_village_ = b;
	return *this;
}


unit_creator& unit_creator::allow_rename_side(bool b)
{
	rename_side_ = b;
	return *this;
}

unit_creator& unit_creator::allow_invalidate(bool b)
{
	invalidate_ = b;
	return *this;
}


unit_creator& unit_creator::allow_discover(bool b)
{
	discover_ = b;
	return *this;
}


unit_creator& unit_creator::allow_add_to_recall(bool b)
{
	add_to_recall_ = b;
	return *this;
}


map_location unit_creator::find_location(const config &cfg, const unit* pass_check)
{

	DBG_NG << "finding location for unit with id=["<<cfg["id"]<<"] placement=["<<cfg["placement"]<<"] x=["<<cfg["x"]<<"] y=["<<cfg["y"]<<"] for side " << team_.side() << "\n";

	std::vector<std::string> placements = utils::split(cfg["placement"]);

	placements.push_back("map");
	placements.push_back("recall");

	bool pass = cfg["passable"].to_bool(false);
	bool vacant = !cfg["overwrite"].to_bool(false);

	for (const std::string& place : placements)
	{
		map_location loc;

		if ( place == "recall" ) {
			return map_location::null_location();
		}

		else if ( place == "leader"  ||  place == "leader_passable" ) {
			unit_map::const_iterator leader = board_->units().find_leader(team_.side());
			//todo: take 'leader in recall list' possibility into account
			if (leader.valid()) {
				loc = leader->get_location();
			} else {
				loc = start_pos_;
			}
			if(place == "leader_passable") {
				deprecated_message("placement=leader_passable", DEP_LEVEL::PREEMPTIVE, {1, 15, 0}, "Please use placement=leader and passable=yes instead");
				pass = true;
			}
		}

		// "map", "map_passable", and "map_overwrite".
		else if(place == "map"  ||  place == "map_passable" || place == "map_overwrite") {
			if(cfg.has_attribute("location_id")) {
				loc = board_->map().special_location(cfg["location_id"]);
			}
			if(!loc.valid()) {
				loc = map_location(cfg, resources::gamedata);
			}
			if(place == "map_passable") {
				deprecated_message("placement=map_passable", DEP_LEVEL::PREEMPTIVE, {1, 15, 0}, "Please use placement=map and passable=yes instead");
				pass = true;
			} else if(place == "map_overwrite") {
				deprecated_message("placement=map_overwrite", DEP_LEVEL::PREEMPTIVE, {1, 15, 0}, "Please use placement=map and overwrite=yes instead");
				vacant = false;
			}
		}

		if(loc.valid() && board_->map().on_board(loc)) {
			if(vacant) {
				loc = find_vacant_tile(loc, pathfind::VACANT_ANY,
				                       pass ? pass_check : nullptr, nullptr, board_);
			}
			if(loc.valid() && board_->map().on_board(loc)) {
				return loc;
			}
		}
	}

	return map_location::null_location();

}


void unit_creator::add_unit(const config &cfg, const vconfig* vcfg)
{
	config temp_cfg(cfg);
	temp_cfg["side"] = team_.side();

	const std::string& id =(cfg)["id"];
	bool animate = temp_cfg["animate"].to_bool();
	bool fire_event = temp_cfg["fire_event"].to_bool(true);
	temp_cfg.remove_attribute("animate");

	unit_ptr recall_list_element = team_.recall_list().find_if_matches_id(id);

	if ( !recall_list_element ) {
		//make the new unit
		unit_ptr new_unit(new unit(temp_cfg, true, vcfg));
		map_location loc = find_location(temp_cfg, new_unit.get());
		if ( loc.valid() ) {
			//add the new unit to map
			board_->units().replace(loc, new_unit);
			LOG_NG << "inserting unit for side " << new_unit->side() << "\n";
			post_create(loc,*(board_->units().find(loc)),animate,fire_event);
		}
		else if ( add_to_recall_ ) {
			//add to recall list
			team_.recall_list().add(new_unit);
			DBG_NG << "inserting unit with id=["<<id<<"] on recall list for side " << new_unit->side() << "\n";
			preferences::encountered_units().insert(new_unit->type_id());
		}
	} else {
		//get unit from recall list
		map_location loc = find_location(temp_cfg, recall_list_element.get());
		if ( loc.valid() ) {
			board_->units().replace(loc, recall_list_element);
			LOG_NG << "inserting unit from recall list for side " << recall_list_element->side()<< " with id="<< id << "\n";
			post_create(loc,*(board_->units().find(loc)),animate,fire_event);
			//if id is not empty, delete units with this ID from recall list
			team_.recall_list().erase_if_matches_id( id);
		}
		else if ( add_to_recall_ ) {
			LOG_NG << "wanted to insert unit on recall list, but recall list for side " << (cfg)["side"] << "already contains id=" <<id<<"\n";
			return;
		}
	}
}


void unit_creator::post_create(const map_location &loc, const unit &new_unit, bool anim, bool fire_event)
{

	if (discover_) {
		preferences::encountered_units().insert(new_unit.type_id());
	}

	bool show = show_ && (display::get_singleton() !=nullptr) && !display::get_singleton()->fogged(loc);
	bool animate = show && anim;

	if (get_village_) {
		assert(resources::gameboard);
		if (board_->map().is_village(loc)) {
			actions::get_village(loc, new_unit.side());
		}
	}

	// Only fire the events if it's safe; it's not if we're in the middle of play_controller::reset_gamestate()
	if (fire_event && resources::lua_kernel != nullptr) {
		resources::game_events->pump().fire("unit_placed", loc);
	}

	if (animate) {
		unit_display::unit_recruited(loc);
	}
}
