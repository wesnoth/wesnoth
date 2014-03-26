/*
   Copyright (C) 2009 - 2014 by Yurii Chernyi <terraninfo@terraninfo.net>
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
 * Gather statistics important for AI testing and output them
 * @file
 */
#include "manager.hpp"
#include "testing.hpp"
#include "../log.hpp"
#include "../replay.hpp"
#include "../util.hpp"
#include "../resources.hpp"
#include "../team.hpp"
#include "../tod_manager.hpp"

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
	team& current_team = (*resources::teams)[side-1];

	int _turn_number = resources::tod_manager->turn();
	int _units = side_units(side);
	int _units_cost = side_units_cost(side);
	int _gold = current_team.gold();
	int _villages = current_team.villages().size();
	int _income = current_team.total_income();

	DBG_AI_TESTING << msg <<                  side << ": " << _turn_number << std::endl;
	DBG_AI_TESTING << msg << "_UNITS"      << side << ": " << _units << std::endl;
	DBG_AI_TESTING << msg << "_UNITS_COST" << side << ": " << _units_cost << std::endl;
	DBG_AI_TESTING << msg << "_GOLD"       << side << ": " << _gold << std::endl;
	DBG_AI_TESTING << msg << "_VILLAGES"   << side << ": " << _villages << std::endl;
	DBG_AI_TESTING << msg << "_INCOME"     << side << ": " << _income << std::endl;

	config c;
	c["side"] = int(side);
	c["turn"] = _turn_number;
	c["event"] = msg;
	c["units"] = _units;
	c["units_cost"] = _units_cost;
	c["gold"] = _gold;
	c["villages"] = _villages;
	recorder.add_log_data("ai_log","turn_info",c);
}

void ai_testing::log_draw()
{
	LOG_AI_TESTING << "DRAW:" << std::endl;
	recorder.add_log_data("ai_log","result","draw");
}

void ai_testing::log_victory(std::set<unsigned int> winners)
{
	recorder.add_log_data("ai_log","result","victory");
	for(std::set<unsigned int>::const_iterator w = winners.begin(); w != winners.end(); ++w) {
		LOG_AI_TESTING << "WINNER: "<< *w <<std::endl;
		recorder.add_log_data("ai_log","winner",str_cast(*w));
	}
}

void ai_testing::log_game_start()
{
	for (std::vector<team>::const_iterator tm = resources::teams->begin(); tm != resources::teams->end(); ++tm) {
		int side = tm-resources::teams->begin()+1;
		LOG_AI_TESTING << "AI_IDENTIFIER"<<side<<": " << ai::manager::get_active_ai_identifier_for_side(side) <<std::endl;
		LOG_AI_TESTING << "TEAM"<<side<<": " << tm->name() << std::endl;
		recorder.add_log_data("ai_log","ai_id"+str_cast(side),ai::manager::get_active_ai_identifier_for_side(side));
		recorder.add_log_data("ai_log","faction"+str_cast(side),tm->name());
		///@todo 1.9: add information about ai_config
	}
	LOG_AI_TESTING << "VERSION: " << game_config::revision << std::endl;
	recorder.add_log_data("ai_log","version",game_config::revision);
}

void ai_testing::log_game_end()
{
	LOG_AI_TESTING << "GAME_END_TURN: "<< resources::tod_manager->turn() <<std::endl;
	recorder.add_log_data("ai_log","end_turn",
		str_cast(resources::tod_manager->turn()));
	for (std::vector<team>::const_iterator tm = resources::teams->begin(); tm != resources::teams->end(); ++tm) {
		int side = tm-resources::teams->begin()+1;
		recorder.add_log_data("ai_log","end_gold"+str_cast(side),str_cast(tm->gold()));
		recorder.add_log_data("ai_log","end_units"+str_cast(side),str_cast(side_units(side)));
	}
}
