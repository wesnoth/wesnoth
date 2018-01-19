/*
   Copyright (C) 2003 - 2018 by Jörg Hinrichs
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

#pragma once

#include "config.hpp"
#include "gettext.hpp"
#include "utils/make_enum.hpp"
#include "version.hpp"

#include <boost/optional.hpp>

struct mp_game_settings
{
	mp_game_settings();
	mp_game_settings(const config& cfg);

	config to_config() const;

	// The items returned while configuring the game

	std::string name;
	std::string password;
	std::string hash;
	std::string mp_era;
	std::string mp_era_name;
	std::string mp_scenario;
	std::string mp_scenario_name;
	std::string mp_campaign;
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
	bool registered_users_only;
	bool shuffle_sides;

	bool saved_game;

	MAKE_ENUM(RANDOM_FACTION_MODE,
		(DEFAULT, N_("Independent"))
		(NO_MIRROR, N_("No Mirror"))
		(NO_ALLY_MIRROR, N_("No Ally Mirror"))
	)

	RANDOM_FACTION_MODE random_faction_mode;

	config options;

	struct addon_version_info {
		boost::optional<version_info> version;
		boost::optional<version_info> min_version;

		explicit addon_version_info(const config &);
		void write(config &) const;
	};

	std::map<std::string, addon_version_info> addons; // the key is the addon_id

	// Takes a config with addon metadata (id =, version =, min_version =), formatted similarly to how mp_game_settings is written that is,
	// and adds this as a requirement, updating the min_version if there was already an entry for this addon_id.
	void update_addon_requirements(const config & addon_cfg);
};
