/*
	Copyright (C) 2005 - 2024
	by Philippe Plantier <ayin@anathas.org>
	Copyright (C) 2003 - 2005 by David White <dave@whitevine.net>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#pragma once

#include "game_end_exceptions.hpp"

#include <set>
#include <string>

class saved_game;
class wesnothd_connection;

struct mp_game_metadata
{
	mp_game_metadata(wesnothd_connection& wdc)
		: connected_players()
		, is_host()
		, current_turn(0)
		, skip_replay(false)
		, skip_replay_blindfolded(false)
		, connection(wdc)
	{
	}

	/** players and observers */
	std::set<std::string> connected_players;
	bool is_host;
	unsigned current_turn;
	bool skip_replay;
	bool skip_replay_blindfolded;
	wesnothd_connection& connection;
};

class campaign_controller
{
public:
	campaign_controller(saved_game& state, bool is_unit_test = false)
		: state_(state)
		, is_unit_test_(is_unit_test)
		, is_replay_(false)
		, mp_info_(nullptr)
	{
	}

	~campaign_controller();

	level_result::type play_game();
	level_result::type play_replay()
	{
		is_replay_ = true;
		return play_game();
	}

	void set_mp_info(mp_game_metadata* mp_info)
	{
		mp_info_ = mp_info;
	}

private:
	level_result::type playsingle_scenario(end_level_data& end_level);
	level_result::type playmp_scenario(end_level_data& end_level);

	saved_game& state_;
	const bool is_unit_test_;
	bool is_replay_;
	mp_game_metadata* mp_info_;
};
