/*
   Copyright (C) 2006 - 2017 by Joerg Hinrichs <joerg.hinrichs@alice-dsl.de>
   wesnoth playlevel Copyright (C) 2003 by David White <dave@whitevine.net>
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
 *  @file
 *  Container for multiplayer game-creation parameters.
 */

#include "log.hpp"
#include "mp_game_settings.hpp"
#include "formula/string_utils.hpp"

static lg::log_domain log_engine("engine");
#define ERR_NG LOG_STREAM(err, log_engine)
#define WRN_NG LOG_STREAM(warn, log_engine)
#define LOG_NG LOG_STREAM(info, log_engine)
#define DBG_NG LOG_STREAM(debug, log_engine)

mp_game_settings::mp_game_settings() :
	name(),
	password(),
	hash(),
	mp_era(),
	mp_era_addon_id(),
	mp_scenario(),
	mp_scenario_name(),
	mp_campaign(),
	active_mods(),
	side_users(),
	show_connect(true),
	num_turns(0),
	village_gold(0),
	village_support(1),
	xp_modifier(100),
	mp_countdown_init_time(0),
	mp_countdown_reservoir_time(0),
	mp_countdown_turn_bonus(0),
	mp_countdown_action_bonus(0),
	mp_countdown(false),
	use_map_settings(false),
	random_start_time(false),
	fog_game(false),
	shroud_game(false),
	allow_observers(true),
	registered_users_only(false),
	shuffle_sides(false),
	saved_game(false),
	random_faction_mode(RANDOM_FACTION_MODE::DEFAULT),
	options(),
	addons()
{}

mp_game_settings::mp_game_settings(const config& cfg)
	: name(cfg["scenario"].str())
	, password()
	, hash(cfg["hash"].str())
	, mp_era(cfg["mp_era"].str())
	, mp_era_addon_id(cfg["mp_era_addon_id"].str())
	, mp_scenario(cfg["mp_scenario"].str())
	, mp_scenario_name(cfg["mp_scenario_name"].str())
	, mp_campaign(cfg["mp_campaign"].str())
	, active_mods(utils::split(cfg["active_mods"], ','))
	, side_users(utils::map_split(cfg["side_users"]))
	, show_connect(cfg["show_connect"].to_bool(true))
	, num_turns(cfg["mp_num_turns"])
	, village_gold(cfg["mp_village_gold"])
	, village_support(cfg["mp_village_support"])
	, xp_modifier(cfg["experience_modifier"].to_int(100))
	, mp_countdown_init_time(cfg["mp_countdown_init_time"])
	, mp_countdown_reservoir_time(cfg["mp_countdown_reservoir_time"])
	, mp_countdown_turn_bonus(cfg["mp_countdown_turn_bonus"])
	, mp_countdown_action_bonus(cfg["mp_countdown_action_bonus"])
	, mp_countdown(cfg["mp_countdown"].to_bool())
	, use_map_settings(cfg["mp_use_map_settings"].to_bool())
	, random_start_time(cfg["mp_random_start_time"].to_bool())
	, fog_game(cfg["mp_fog"].to_bool())
	, shroud_game(cfg["mp_shroud"].to_bool())
	, allow_observers(cfg["observer"].to_bool())
	, registered_users_only(cfg["registered_users_only"].to_bool())
	, shuffle_sides(cfg["shuffle_sides"].to_bool())
	, saved_game(cfg["savegame"].to_bool())
	, random_faction_mode(cfg["random_faction_mode"].to_enum<RANDOM_FACTION_MODE>(RANDOM_FACTION_MODE::DEFAULT))
	, options(cfg.child_or_empty("options"))
	, addons()
{
	for (const config & a : cfg.child_range("addon")) {
		if (!a["id"].empty()) {
			addons.emplace(a["id"].str(), addon_version_info(a));
		}
	}
}

config mp_game_settings::to_config() const
{
	config cfg;

	cfg["scenario"] = name;
	cfg["hash"] = hash;
	cfg["mp_era"] = mp_era;
	cfg["mp_era_addon_id"] = mp_era_addon_id;
	cfg["mp_scenario"] = mp_scenario;
	cfg["mp_scenario_name"] = mp_scenario_name;
	cfg["mp_campaign"] = mp_campaign;
	cfg["active_mods"] = utils::join(active_mods, ",");
	cfg["side_users"] = utils::join_map(side_users);
	cfg["show_connect"] = show_connect;
	cfg["experience_modifier"] = xp_modifier;
	cfg["mp_countdown"] = mp_countdown;
	cfg["mp_countdown_init_time"] = mp_countdown_init_time;
	cfg["mp_countdown_turn_bonus"] = mp_countdown_turn_bonus;
	cfg["mp_countdown_reservoir_time"] = mp_countdown_reservoir_time;
	cfg["mp_countdown_action_bonus"] = mp_countdown_action_bonus;
	cfg["mp_num_turns"] = num_turns;
	cfg["mp_village_gold"] = village_gold;
	cfg["mp_village_support"] = village_support;
	cfg["mp_fog"] = fog_game;
	cfg["mp_shroud"] = shroud_game;
	cfg["mp_use_map_settings"] = use_map_settings;
	cfg["mp_random_start_time"] = random_start_time;
	cfg["observer"] = allow_observers;
	cfg["registered_users_only"] = registered_users_only;
	cfg["shuffle_sides"] = shuffle_sides;
	cfg["random_faction_mode"] = random_faction_mode;
	cfg["savegame"] = saved_game;
	cfg.add_child("options", options);

	for(auto& p : addons) {
		config & c = cfg.add_child("addon");
		p.second.write(c);
		c["id"] = p.first;
	}

	return cfg;
}

mp_game_settings::addon_version_info::addon_version_info(const config & cfg)
	: version()
	, min_version()
{
	if (!cfg["version"].empty()) {
		version = cfg["version"].str();
	}
	if (!cfg["min_version"].empty()) {
		min_version = cfg["min_version"].str();
	}
}

void mp_game_settings::addon_version_info::write(config & cfg) const {
	if (version) {
		cfg["version"] = *version;
	}
	if (min_version) {
		cfg["min_version"] = *min_version;
	}
}

void mp_game_settings::update_addon_requirements(const config & cfg) {
	if (cfg["id"].empty()) {
		WRN_NG << "Tried to add add-on metadata to a game, missing mandatory id field... skipping.\n" << cfg.debug() << "\n";
		return;
	}

	mp_game_settings::addon_version_info new_data(cfg);

	// Check if this add-on already has an entry as a dependency for this scenario. If so, try to reconcile their version info,
	// by taking the larger of the min versions. The version should be the same for all WML from the same add-on...
	std::map<std::string, addon_version_info>::iterator it = addons.find(cfg["id"].str());
	if (it != addons.end()) {
		addon_version_info & addon = it->second;

		if (new_data.version) {
			if (!addon.version || (*addon.version != *new_data.version)) {
				WRN_NG << "Addon version data mismatch -- not all local WML has same version of '" << cfg["id"].str() << "' addon.\n";
			}
		}
		if (addon.version && !new_data.version) {
			WRN_NG << "Addon version data mismatch -- not all local WML has same version of '" << cfg["id"].str() << "' addon.\n";
		}
		if (new_data.min_version) {
			if (!addon.min_version || (*new_data.min_version > *addon.min_version)) {
				addon.min_version = *new_data.min_version;
			}
		}
	} else {
		// Didn't find this addon-id in the map, so make a new entry.
		addons.emplace(cfg["id"].str(), new_data);
	}
}
