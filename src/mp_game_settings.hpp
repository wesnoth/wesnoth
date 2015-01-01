/*
   Copyright (C) 2003 - 2015 by Jörg Hinrichs
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/** @file */

#ifndef MP_GAME_SETTINGS_HPP_INCLUDED
#define MP_GAME_SETTINGS_HPP_INCLUDED

#include "config.hpp"
#include "savegame_config.hpp"

struct mp_game_settings : public savegame::savegame_config
{
	mp_game_settings();
	mp_game_settings(const config& cfg);
	mp_game_settings(const mp_game_settings& settings);

	void reset();

	void set_from_config(const config& game_cfg);
	config to_config() const;

	// The items returned while configuring the game

	std::string name;
	std::string password;
	std::string hash;
	std::string mp_era;
	std::string mp_scenario;
	std::string mp_scenario_name;
	std::string mp_campaign;
	std::string difficulty_define;
	std::vector<std::string> active_mods;
	std::map<std::string, std::string> side_users;

	int num_turns;
	int village_gold;
	int village_support;
	int xp_modifier;
	int mp_countdown_init_time;
	int mp_countdown_reservoir_time;
	int mp_countdown_turn_bonus;
	int mp_countdown_action_bonus;
	bool mp_countdown;
	bool use_map_settings;
	bool random_start_time;
	bool fog_game;
	bool shroud_game;
	bool allow_observers;
	bool shuffle_sides;
	bool share_view;
	bool share_maps;

	bool saved_game;

	config options;

	/**
	 * If the game is to be randomly generated, the map generator
	 * will create the scenario data in this variable
	 */
	config scenario_data;
};

#endif
