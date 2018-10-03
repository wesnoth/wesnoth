/*
   Copyright (C) 2003-2005 by David White <dave@whitevine.net>
   Copyright (C) 2005 - 2018 by Philippe Plantier <ayin@anathas.org>
   Part of the Battle for Wesnoth Project https://www.wesnoth.org

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/** @file */

#pragma once

#include "game_end_exceptions.hpp"

#include <memory>
#include <sstream>
#include <set>
#include <string>

class saved_game;
class terrain_type_data;
class team;
class playsingle_controller;
typedef std::shared_ptr<terrain_type_data> ter_data_cache;

class config;

class wesnothd_connection;
struct mp_campaign_info
{
	mp_campaign_info(wesnothd_connection& wdc)
		: connected_players()
		, is_host()
		, current_turn(0)
		, skip_replay(false)
		, skip_replay_blindfolded(false)
		, connection(wdc)
	{

	}
	/// players and observers
	std::set<std::string> connected_players;
	bool is_host;
	unsigned current_turn;
	bool skip_replay;
	bool skip_replay_blindfolded;
	wesnothd_connection& connection;
};

class campaign_controller
{
	saved_game& state_;
	const ter_data_cache & tdata_;
	const bool is_unit_test_;
	bool is_replay_;
	mp_campaign_info* mp_info_;
public:
	campaign_controller(saved_game& state, const ter_data_cache & tdata, bool is_unit_test = false)
		: state_(state)
		, tdata_(tdata)
		, is_unit_test_(is_unit_test)
		, is_replay_(false)
		, mp_info_(nullptr)
	{
	}
	LEVEL_RESULT play_game();
	LEVEL_RESULT play_replay()
	{
		is_replay_ = true;
		return play_game();
	}
	void set_mp_info(mp_campaign_info* mp_info) { mp_info_ = mp_info; }
private:
	LEVEL_RESULT playsingle_scenario(end_level_data &end_level);
	LEVEL_RESULT playmp_scenario(end_level_data &end_level);
	void show_carryover_message(playsingle_controller& playcontroller, const end_level_data& end_level, LEVEL_RESULT res);
	static void report_victory(std::ostringstream &report, team& t,	int finishing_bonus_per_turn, int turns_left, int finishing_bonus);
};
