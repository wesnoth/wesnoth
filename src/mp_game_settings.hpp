/*
	Copyright (C) 2006 - 2024
	by Joerg Hinrichs <joerg.hinrichs@alice-dsl.de>
	Copyright (C) 2003 by David White <dave@whitevine.net>
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

#include "config.hpp"
#include "game_initialization/random_faction_mode.hpp"
#include "game_initialization/saved_game_mode.hpp"
#include "game_version.hpp"

#include "utils/optional_fwd.hpp"

#include <chrono>

struct mp_game_settings
{
	mp_game_settings();
	mp_game_settings(const config& cfg);

	config to_config() const;

	// The items returned while configuring the game

	std::string name;
	std::string password;
	std::string hash;
	std::string mp_era_name;
	std::string mp_scenario;
	std::string mp_scenario_name;
	std::string mp_campaign;
	std::map<std::string, std::string> side_users;

	int num_turns;
	int village_gold;
	int village_support;
	int xp_modifier;
	std::chrono::seconds mp_countdown_init_time;
	std::chrono::seconds mp_countdown_reservoir_time;
	std::chrono::seconds mp_countdown_turn_bonus;
	std::chrono::seconds mp_countdown_action_bonus;
	bool mp_countdown;
	bool use_map_settings;
	bool random_start_time;
	bool fog_game;
	bool shroud_game;
	bool allow_observers;
	bool private_replay;
	bool shuffle_sides;

	saved_game_mode::type saved_game;

	random_faction_mode::type mode;

	config options;

	struct addon_content
	{
		std::string id;
		std::string name;
		std::string type;
	};

	struct addon_version_info
	{
		utils::optional<version_info> version;
		utils::optional<version_info> min_version;
		std::string name;
		bool required;
		std::vector<addon_content> content;

		explicit addon_version_info(const config &);
		void write(config &) const;
	};

	/** the key is the addon_id */
	std::map<std::string, addon_version_info> addons;

	/**
	 * Takes a config with addon metadata (id, name, version, min_version) and adds
	 * it as a requirement for this game. It also updates min_version if there was
	 * already an entry for this addon_id.
	 */
	void update_addon_requirements(const config& addon_cfg);
};
