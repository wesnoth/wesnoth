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

#include "teambuilder.hpp"

#include "actions/create.hpp"
#include "game_board.hpp"
#include "game_errors.hpp"
#include "log.hpp"
#include "map/map.hpp"
#include "serialization/string_utils.hpp"
#include "team.hpp"
#include "units/type_error.hpp"
#include "units/unit.hpp"

#include <vector>

static lg::log_domain log_engine_tc("engine/team_construction");
#define ERR_NG_TC LOG_STREAM(err, log_engine_tc)
#define WRN_NG_TC LOG_STREAM(warn, log_engine_tc)
#define LOG_NG_TC LOG_STREAM(info, log_engine_tc)
#define DBG_NG_TC LOG_STREAM(debug, log_engine_tc)

team_builder::team_builder(const config& side_cfg, team& to_build, const config& level, game_board& board, int num)
	: leader_configs_()
	, level_(level)
	, board_(board)
	, seen_ids_()
	, side_(num)
	, side_cfg_(side_cfg)
	, team_(to_build)
	, unit_configs_()
{
}

void team_builder::build_team_stage_one()
{
	// initialize the context variables and flags, find relevant tags, set up everything
	init();

	// builds the team for the given side
	new_team();

	// set team objectives if necessary
	objectives();

	// If the game state specifies additional units that can be recruited by the player, add them.
	previous_recruits();

}

void team_builder::build_team_stage_two()
{
	// place leader
	leader();

	// prepare units, populate obvious recall lists elements
	prepare_units();
}

void team_builder::build_team_stage_three()
{
	// place units
	// this is separate stage because we need to place units only after every other team is constructed
	place_units();
}

void team_builder::log_step(const char* s) const
{
	LOG_NG_TC << "team " << side_ << " construction: " << s;
}

void team_builder::init()
{
	if(side_cfg_["side"].to_int(side_) != side_) {
		ERR_NG_TC << "found invalid side=" << side_cfg_["side"].to_int(side_) << " in definition of side number " << side_;
	}

	log_step("init");

	if(board_.map().empty()) {
		throw game::load_game_failed("Map not found");
	}

	unit_configs_.clear();
	seen_ids_.clear();
}


void team_builder::new_team()
{
	log_step("new team");
	team_.build(side_cfg_, board_.map());
}

void team_builder::objectives()
{
	log_step("objectives");
	// If this team has no objectives, set its objectives
	// to the level-global "objectives"
	// this is only used by the default mp 'Defeat enemy leader' objectives
	if(team_.objectives().empty()) {
		team_.set_objectives(level_["objectives"], false);
	}
}

void team_builder::previous_recruits()
{
	log_step("previous recruits");

	if(const config::attribute_value* v = side_cfg_.get("previous_recruits")) {
		for(const std::string& rec : utils::split(*v)) {
			DBG_NG_TC << "adding previous recruit: " << rec;
			team_.add_recruit(rec);
		}
	}
}

void team_builder::handle_unit(const config& u, const char* origin)
{
	DBG_NG_TC
		<< "unit from " << origin << ": "
		<< "type=[" << u["type"] << "] "
		<< "id=[" << u["id"] << "] "
		<< "placement=[" << u["placement"] << "] "
		<< "x=[" << u["x"] << "] "
		<< "y=[" << u["y"] << "]";

	if(u["type"].empty()) {
		WRN_NG_TC
			<< "when building level, skipping a unit (id=[" << u["id"] << "]) from " << origin
			<< " with no type information,\n"
			<< "for side:\n"
			<< side_cfg_.debug();

		return;
	}

	const std::string& id = u["id"];
	if(!id.empty()) {
		if(seen_ids_.find(id) != seen_ids_.end()) {
			// seen before
			config u_tmp = u;
			u_tmp["side"] = std::to_string(side_);
			team_.recall_list().add(unit::create(u_tmp, true));
		} else {
			// not seen before
			unit_configs_.push_back(&u);
			seen_ids_.insert(id);
		}
	} else {
		unit_configs_.push_back(&u);
	}
}

void team_builder::handle_leader(const config& leader)
{
	// Make a persistent copy of the config.
	leader_configs_.push_back(leader);
	config& stored = leader_configs_.back();

	// Provide some default values, if not specified.
	config::attribute_value& a1 = stored["canrecruit"];
	if(a1.blank()) {
		a1 = true;
	}

	config::attribute_value& a2 = stored["placement"];
	if(a2.blank()) {
		a2 = "map,leader";
	}

	// Add the leader to the list of units to create.
	handle_unit(stored, "leader_cfg");
}

void team_builder::leader()
{
	log_step("leader");
	for(const config& l : side_cfg_.child_range("leader")) {
		handle_leader(l);
	}
}

void team_builder::prepare_units()
{
	// if this is a start-of-scenario save then  playcampaign.cpp merged
	// units in [replay_start][side] merged with [side] already
	// units that are in '[scenario][side]' are 'first'

	// for create-or-recall semantics to work: for each unit with non-empty
	// id, unconditionally put OTHER, later, units with same id directly to
	// recall list, not including them in unit_configs_
	for(const config& su : side_cfg_.child_range("unit")) {
		handle_unit(su, "side_cfg");
	}
}

void team_builder::place_units()
{
	log_step("place units");
	unit_creator uc(team_, board_.map().starting_position(side_), &board_);
	uc.allow_add_to_recall(true)
		.allow_discover(true)
		.allow_get_village(false)
		.allow_invalidate(false)
		.allow_rename_side(true)
		.allow_show(false);

	for(const config* u : unit_configs_) {
		try {
			uc.add_unit(*u);
		} catch(const unit_type_error& e) {
			ERR_NG_TC << e.what();
		}
	}
}
