/* $Id$ */
/*
   Copyright (C) 2009 by Yurii Chernyi <terraninfo@terraninfo.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * Gather statistics important for AI testing and output them
 * @file ai/testing.cpp
 */
#include "ai_manager.hpp"
#include "testing.hpp"
#include "../log.hpp"

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
	ai_game_info& i = ai_manager::get_ai_info();
	assert(side>=1);
	team& current_team = i.teams[side-1];

	size_t _turn_number = i.state.turn();
	int _units = side_units(i.units,side);
	int _units_cost = side_units_cost(i.units,side);
	int _gold = current_team.gold();
	int _villages = current_team.villages().size();
	int _income = current_team.total_income();

	LOG_AI_TESTING << msg <<                  side << ": " << _turn_number << std::endl;
	LOG_AI_TESTING << msg << "_UNITS"      << side << ": " << _units << std::endl;
	LOG_AI_TESTING << msg << "_UNITS_COST" << side << ": " << _units_cost << std::endl;
	LOG_AI_TESTING << msg << "_GOLD"       << side << ": " << _gold << std::endl;
	LOG_AI_TESTING << msg << "_VILLAGES"   << side << ": " << _villages << std::endl;
	LOG_AI_TESTING << msg << "_INCOME"     << side << ": " << _income << std::endl;
}

void ai_testing::log_draw()
{
	LOG_AI_TESTING << "DRAW:" << std::endl;
}

void ai_testing::log_victory(std::vector<unsigned int> winners)
{
	for(std::vector<unsigned int>::const_iterator w = winners.begin(); w != winners.end(); ++w) {
		LOG_AI_TESTING << "WINNER: "<< *w <<std::endl;
	}
}

void ai_testing::log_game_start()
{
	ai_game_info& i = ai_manager::get_ai_info();
	for (std::vector<team>::const_iterator tm = i.teams.begin(); tm != i.teams.end(); ++tm) {
		int side = tm-i.teams.begin()+1;
		LOG_AI_TESTING << "AI_IDENTIFIER"<<side<<": " << tm->ai_algorithm_identifier() <<std::endl;
		LOG_AI_TESTING << "FACTION"<<side<<": " << tm->name() << std::endl;
	}
	LOG_AI_TESTING << "VERSION: " << game_config::revision << std::endl;
}

void ai_testing::log_game_end()
{
	LOG_AI_TESTING << "GAME_END_TURN: "<< ai_manager::get_ai_info().state.turn() <<std::endl;
}
