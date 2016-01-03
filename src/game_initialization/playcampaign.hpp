/*
   Copyright (C) 2003-2005 by David White <dave@whitevine.net>
   Copyright (C) 2005 - 2016 by Philippe Plantier <ayin@anathas.org>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/** @file */

#ifndef PLAYCAMPAIGN_H_INCLUDED
#define PLAYCAMPAIGN_H_INCLUDED

#include "game_end_exceptions.hpp"

#include <boost/shared_ptr.hpp>
#include <sstream>
#include <set>
#include <string>

class display;
class game_display;
class saved_game;
class terrain_type_data;
class team;
class playsingle_controller;
typedef boost::shared_ptr<terrain_type_data> tdata_cache;

class config;

struct mp_campaign_info
{
	mp_campaign_info()
		: connected_players()
		, is_host()
		, skip_replay_until_turn(0)
		, skip_replay_blindfolded(false)
		, is_connected(true)
	{

	}
	/// players and observers
	std::set<std::string> connected_players;
	bool is_host;
	int skip_replay_until_turn;
	bool skip_replay_blindfolded;
	bool is_connected;
};

class campaign_controller
{
	game_display& disp_;
	saved_game& state_;
	const config& game_config_;
	const tdata_cache & tdata_;
	const bool is_unit_test_;
	bool is_replay_;
	mp_campaign_info* mp_info_;
public:
	campaign_controller(game_display& disp, saved_game& state, const config& game_config, const tdata_cache & tdata, bool is_unit_test = false)
		: disp_(disp)
		, state_(state)
		, game_config_(game_config)
		, tdata_(tdata)
		, is_unit_test_(is_unit_test)
		, is_replay_(false)
		, mp_info_(NULL)
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

#endif // PLAYCAMPAIGN_H_INCLUDED

