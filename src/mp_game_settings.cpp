/* $Id$ */
/*
   Copyright (C) 2006 - 2009 by Joerg Hinrichs <joerg.hinrichs@alice-dsl.de>
   wesnoth playlevel Copyright (C) 2003 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 *  @file mp_game_settings.cpp
 *  Container for multiplayer game-creation parameters.
 */

#include "mp_game_settings.hpp"

mp_game_settings::mp_game_settings() :
	name(),
	era(),
	password(),
	num_turns(0),
	village_gold(0),
	xp_modifier(0),
	mp_countdown_init_time(0),
	mp_countdown_reservoir_time(0),
	mp_countdown_turn_bonus(0),
	mp_countdown_action_bonus(0),
	mp_countdown(false),
	use_map_settings(false),
	random_start_time(false),
	fog_game(false),
	shroud_game(false),
	allow_observers(false),
	share_view(false),
	share_maps(false),
	saved_game(false),
	scenario_data()

{ reset(); };

mp_game_settings::mp_game_settings(const config& cfg)
{
	set_from_config(cfg);
}

mp_game_settings::mp_game_settings(const mp_game_settings& settings)
{
	name = settings.name;
	era = settings.era;
	password = settings.password;
	num_turns = settings.num_turns;
	village_gold = settings.village_gold;
	xp_modifier = settings.xp_modifier;
	mp_countdown = settings.mp_countdown;
	mp_countdown_init_time = settings.mp_countdown_init_time;
	mp_countdown_reservoir_time = settings.mp_countdown_reservoir_time;
	mp_countdown_turn_bonus = settings.mp_countdown_turn_bonus;
	mp_countdown_action_bonus = settings.mp_countdown_action_bonus;
	use_map_settings = settings.use_map_settings;
	random_start_time = settings.random_start_time;
	fog_game = settings.fog_game;
	shroud_game = settings.shroud_game;
	allow_observers = settings.allow_observers;
	share_view = settings.share_view;
	share_maps = settings.share_maps;
	saved_game = settings.saved_game;
	scenario_data = settings.scenario_data;
}

void mp_game_settings::set_from_config(const config& game_cfg)
{
	const config& cfg = game_cfg.child("multiplayer");
	name = cfg["scenario"];
	xp_modifier = lexical_cast_default<int>(cfg["experience_modifier"]);
	num_turns = lexical_cast_default<int>(cfg["turns"]);
	use_map_settings = utils::string_bool(cfg["mp_use_map_settings"]);
	fog_game = utils::string_bool(cfg["mp_fog"]);
	shroud_game = utils::string_bool(cfg["mp_shroud"]);
	mp_countdown = utils::string_bool(cfg["mp_countdown"]);
	mp_countdown_init_time = lexical_cast_default<int>(cfg["mp_countdown_init_time"]);
	mp_countdown_turn_bonus = lexical_cast_default<int>(cfg["mp_countdown_turn_bonus"]);
	mp_countdown_reservoir_time = lexical_cast_default<int>(cfg["mp_countdown_reservoir_time"]);
	mp_countdown_action_bonus = lexical_cast_default<int>(cfg["mp_countdown_action_bonus"]);
	village_gold = lexical_cast_default<int>(cfg["mp_village_gold"]);
	allow_observers = utils::string_bool(cfg["observer"]);
}

void mp_game_settings::reset() 
{
	name = "";
	era = "";
	password = "";
	num_turns = 0;
	village_gold = 0;
	xp_modifier = 0;
	mp_countdown_init_time=0;
	mp_countdown_reservoir_time=0;
	mp_countdown_turn_bonus=0;
	mp_countdown_action_bonus=0;
	mp_countdown=false;
	use_map_settings = random_start_time = fog_game = shroud_game = allow_observers = share_view = share_maps = false;

	scenario_data.clear();
}

config mp_game_settings::to_config() const
{
	config cfg;

	cfg["turns"] = lexical_cast_default<std::string>(num_turns, "20");
	cfg["mp_countdown"] = mp_countdown ? "yes" : "no";
	cfg["mp_countdown_init_time"] = lexical_cast_default<std::string>(mp_countdown_init_time, "270");
	cfg["mp_countdown_turn_bonus"] = lexical_cast_default<std::string>(mp_countdown_turn_bonus, "35");
	cfg["mp_countdown_reservoir_time"] = lexical_cast_default<std::string>(mp_countdown_reservoir_time, "330");
	cfg["mp_countdown_action_bonus"] = lexical_cast_default<std::string>(mp_countdown_action_bonus, "13");
	cfg["scenario"] = name;
	cfg["experience_modifier"] = lexical_cast<std::string>(xp_modifier);
	cfg["mp_village_gold"] = lexical_cast<std::string>(village_gold);
	cfg["mp_fog"] = fog_game ? "yes" : "no";
	cfg["mp_shroud"] = shroud_game ? "yes" : "no";
	cfg["mp_use_map_settings"] = use_map_settings ? "yes" : "no";
	cfg["observer"] = allow_observers ? "yes" : "no";

	return cfg;
}