/*
	Copyright (C) 2013 - 2024
	by Andrius Silinskas <silinskas.andrius@gmail.com>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#include "game_initialization/mp_game_utils.hpp"

#include "config.hpp"
#include "formula/string_utils.hpp"
#include "game_config_manager.hpp"
#include "gettext.hpp"
#include "log.hpp"
#include "saved_game.hpp"
#include "serialization/markup.hpp"

static lg::log_domain log_engine("engine");
#define LOG_NG LOG_STREAM(info, log_engine)
#define ERR_NG LOG_STREAM(err, log_engine)

static lg::log_domain log_config("config");
#define LOG_CF LOG_STREAM(info, log_config)
#define WRN_CF LOG_STREAM(warn, log_config)
#define ERR_CF LOG_STREAM(err, log_config)

static lg::log_domain log_network("network");
#define LOG_NW LOG_STREAM(info, log_network)
#define ERR_NW LOG_STREAM(err, log_network)

namespace mp
{
// This is for the wesnothd server, it expects a more detailed summary in [multiplayer]
static void add_multiplayer_classification(config& multiplayer, saved_game& state)
{
	multiplayer["mp_scenario"] = state.get_scenario_id();
	multiplayer["mp_scenario_name"] = state.get_starting_point()["name"];
	multiplayer["difficulty_define"] = state.classification().difficulty;
	multiplayer["mp_campaign"] = state.classification().campaign;
	multiplayer["mp_campaign_name"] = state.classification().campaign_name;
	multiplayer["mp_era"] = state.classification().era_id;
	multiplayer["active_mods"] = utils::join(state.classification().active_mods, ",");
}

config initial_level_config(saved_game& state)
{
	const mp_game_settings& params = state.mp_settings();
	state.set_defaults();

	// Also impliers state.expand_scenario()
	// We need to call this before expand_mp_events/options otherwise they might be overwritten.
	state.expand_random_scenario();
	state.expand_mp_events();
	state.expand_mp_options();

	if(!state.valid()) {
		throw config::error("Failed to load the scenario");
	}

	config& scenario = state.get_starting_point();
	if(state.mp_settings().saved_game == saved_game_mode::type::no) {
		state.set_random_seed();
	}

	if(scenario["objectives"].empty()) {
		// Generic victory objectives.
		std::ostringstream ss;
		ss << markup::tag("big", t_string(N_("Victory:"), "wesnoth")) << "\n";
		ss << markup::span_color("#00ff00",
			font::unicode_bullet, " ", t_string(N_("Defeat enemy leader(s)"), "wesnoth"));
		scenario["objectives"] = ss.str();
	}

	config level = state.to_config();
	add_multiplayer_classification(level.child_or_add("multiplayer"), state);

	// [multiplayer] mp_era= should be persistent over saves.
	std::string era = state.classification().era_id;

	/**
	 * [era] and [modification]s are toplevel tags here.
	 * They are not part of the saved_game and are only used during mp_staging/mp_join_game.
	 *
	 * @todo: see if all the comments ai algorithms are still up-to-date and relevant.
	 *
	 * -- vultraz, 2017-11-24
	 */

	const game_config_view& game_config = game_config_manager::get()->game_config();
	auto era_cfg = game_config.find_child("era", "id", era);

	if(!era_cfg) {
		if(params.saved_game == saved_game_mode::type::no) {
			throw config::error(VGETTEXT("Cannot find era ‘$era’", {{"era", era}}));
		}

		// FIXME: @todo We should tell user about missing era but still load game...
		WRN_CF << "Missing era in MP load game '" << era << "'";

	} else {
		level.add_child("era", *era_cfg);

		// Initialize the list of sides available for the current era.
		// We also need this so not to get a segfault in mp_staging for ai configuration.
		const config& custom_side = game_config.find_mandatory_child("multiplayer_side", "id", "Custom");
		level.mandatory_child("era").add_child_at("multiplayer_side", custom_side, 0);
	}

	// Add modifications, needed for ai algorithms which are applied in mp_staging.
	const std::vector<std::string>& mods = state.classification().active_mods;

	for(unsigned i = 0; i < mods.size(); ++i) {
		if(auto mod_cfg = game_config.find_child("modification", "id", mods[i])) {
			level.add_child("modification", *mod_cfg);
		}
	}

	// This will force connecting clients to be using the same version number as us.
	level["version"] = game_config::wesnoth_version.str();
	return level;
}

void level_to_gamestate(const config& level, saved_game& state)
{
	campaign_type::type type = state.classification().type;
	state = saved_game(level);
	state.classification().type = type;
}

} // end namespace mp
