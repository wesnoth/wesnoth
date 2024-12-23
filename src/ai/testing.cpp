/*
	Copyright (C) 2009 - 2024
	by Yurii Chernyi <terraninfo@terraninfo.net>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

/**
 * Gather statistics important for AI testing and output them
 * @file
 */
#include "ai/manager.hpp"
#include "ai/testing.hpp"
#include "log.hpp"
#include "game_board.hpp"
#include "replay.hpp"
#include "resources.hpp"
#include "team.hpp"
#include "tod_manager.hpp"
#include "game_version.hpp"

static lg::log_domain log_ai_testing("ai/testing");
#define DBG_AI_TESTING LOG_STREAM(debug, log_ai_testing)
#define LOG_AI_TESTING LOG_STREAM(info, log_ai_testing)
#define ERR_AI_TESTING LOG_STREAM(err, log_ai_testing)

void ai_testing::log_turn_start(unsigned int side)
{
	log_turn("TURN_START",side);
}

void ai_testing::log_turn_end(unsigned int side)
{
	log_turn("TURN_END",side);
}

void ai_testing::log_turn(const char* msg, unsigned int side)
{
	assert(side>=1);
	team& current_team = resources::gameboard->get_team(side);

	int _turn_number = resources::tod_manager->turn();
	int _units = resources::gameboard->side_units(side);
	int _units_cost = resources::gameboard->side_units_cost(side);
	int _gold = current_team.gold();
	int _villages = current_team.villages().size();
	int _income = current_team.total_income();

	DBG_AI_TESTING << msg <<                   side << ": " << _turn_number;
	DBG_AI_TESTING << msg << "_UNITS "      << side << ": " << _units;
	DBG_AI_TESTING << msg << "_UNITS_COST " << side << ": " << _units_cost;
	DBG_AI_TESTING << msg << "_GOLD "       << side << ": " << _gold;
	DBG_AI_TESTING << msg << "_VILLAGES "   << side << ": " << _villages;
	DBG_AI_TESTING << msg << "_INCOME "     << side << ": " << _income;

	config c;
	c["side"] = static_cast<int>(side);
	c["turn"] = _turn_number;
	c["event"] = msg;
	c["units"] = _units;
	c["units_cost"] = _units_cost;
	c["gold"] = _gold;
	c["villages"] = _villages;
	resources::recorder->add_log_data("ai_log","turn_info",c);
}

void ai_testing::log_draw()
{
	LOG_AI_TESTING << "DRAW:";
	resources::recorder->add_log_data("ai_log","result","draw");
}

void ai_testing::log_victory(const std::set<unsigned int>& winners)
{
	resources::recorder->add_log_data("ai_log","result","victory");
	for(std::set<unsigned int>::const_iterator w = winners.begin(); w != winners.end(); ++w) {
		LOG_AI_TESTING << "WINNER: "<< *w;
		resources::recorder->add_log_data("ai_log","winner",std::to_string(*w));
	}
}

void ai_testing::log_game_start()
{
	for(const team& tm : resources::gameboard->teams()) {
		int side = tm.side();
		LOG_AI_TESTING << "AI_IDENTIFIER " << side << ": " << ai::manager::get_singleton().get_active_ai_identifier_for_side(side);
		resources::recorder->add_log_data("ai_log", "ai_id" + std::to_string(side), ai::manager::get_singleton().get_active_ai_identifier_for_side(side));
	}
	LOG_AI_TESTING << "VERSION: " << game_config::revision;
	resources::recorder->add_log_data("ai_log","version",game_config::revision);
}

void ai_testing::log_game_end()
{
	LOG_AI_TESTING << "GAME_END_TURN: "<< resources::tod_manager->turn();
	resources::recorder->add_log_data("ai_log","end_turn",
		std::to_string(resources::tod_manager->turn()));
	for(const team& tm : resources::gameboard->teams()) {
		int side = tm.side();
		resources::recorder->add_log_data("ai_log","end_gold"+std::to_string(side),std::to_string(tm.gold()));
		resources::recorder->add_log_data("ai_log","end_units"+std::to_string(side),std::to_string(resources::gameboard->side_units(side)));
	}
}
