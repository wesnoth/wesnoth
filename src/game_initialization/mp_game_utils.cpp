/*
   Copyright (C) 2013 - 2017 by Andrius Silinskas <silinskas.andrius@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#include "game_initialization/mp_game_utils.hpp"

#include "carryover.hpp"
#include "config.hpp"
#include "formula/string_utils.hpp"
#include "saved_game.hpp"
#include "game_config.hpp"
#include "game_config_manager.hpp"
#include "gettext.hpp"
#include "log.hpp"
#include "savegame.hpp"
#include "units/id.hpp"
#include "wesnothd_connection_error.hpp"

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

namespace mp {

// This is for the wesnothd server, it expects a more detailed summary in [multiplayer]
static void add_multiplayer_classification(config& multiplayer, saved_game& state)
{
	multiplayer["mp_scenario"] = state.get_scenario_id();
	multiplayer["mp_scenario_name"] = state.get_starting_pos()["name"];
	multiplayer["difficulty_define"] = state.classification().difficulty;
	multiplayer["mp_campaign"] = state.classification().campaign;
}

config initial_level_config(saved_game& state)
{
	const mp_game_settings& params = state.mp_settings();
	state.set_defaults();
	//Also impliers state.expand_scenario()
	//We need to call this before expand_mp_events/options oterwise they might be overwritten
	state.expand_random_scenario();
	state.expand_mp_events();
	state.expand_mp_options();

	if(!state.valid()) {
		throw config::error("Failed to load the scenario");
	}

	config& scenario = state.get_starting_pos();
	if(!state.mp_settings().saved_game)
	{
		state.set_random_seed();
	}

	if (scenario["objectives"].empty()) {
		scenario["objectives"] = "<big>" + t_string(N_("Victory:"), "wesnoth") +
			"</big>\n<span foreground=\"#00ff00\">&#8226; " +
			t_string(N_("Defeat enemy leader(s)"), "wesnoth") + "</span>";
	}

	config level = state.to_config();
	add_multiplayer_classification(level.child_or_add("multiplayer"), state);

	std::string era = params.mp_era;
	//[multiplayer] mp_era= should be persistent over saves.

	//[era], [modification]s are toplevel tags here, they are not part of the saved_game and only used during mp_connect/mp_wait
	// Initialize the list of sides available for the current era.
	// We also need this no not get a segfault in mp_connect for ai configuration
	const config &era_cfg =
		game_config_manager::get()->game_config().find_child("era", "id", era);
	if (!era_cfg) {
		if (!params.saved_game)
		{
			utils::string_map i18n_symbols;
			i18n_symbols["era"] = era;
			throw config::error(vgettext("Cannot find era $era", i18n_symbols));
		}
		// FIXME: @todo We should tell user about missing era but still load game
		WRN_CF << "Missing era in MP load game " << era << std::endl;
		//Otherwise we get an error when qwhen we try to add ai algirithms in moultiplayer_connect
		level.add_child("era");
	}
	else
	{
		/*config& cfg = */level.add_child("era", era_cfg);

		const config& custom_side = game_config_manager::get()->
			game_config().find_child("multiplayer_side", "id", "Custom");
		level.child("era").add_child_at("multiplayer_side", custom_side, 0);

	}
	// Add modifications, needed for ai aglorithms which are applied in mp_connect

	const std::vector<std::string>& mods = params.active_mods;
	for (unsigned i = 0; i < mods.size(); i++) {
		/*config& cfg = */level.add_child("modification",
			game_config_manager::get()->
				game_config().find_child("modification", "id", mods[i]));
	}

	// This will force connecting clients to be using the same version number as us.
	level["version"] = game_config::version;
	return level;
}

void level_to_gamestate(const config& level, saved_game& state)
{
	game_classification::CAMPAIGN_TYPE type = state.classification().campaign_type;
	bool show_connect = state.mp_settings().show_connect;
	state = saved_game(level);
	state.classification().campaign_type = type;
	state.mp_settings().show_connect = show_connect;
}

void check_response(bool res, const config& data)
{
	if (!res) {
		throw wesnothd_error(_("Connection timed out"));
	}

	if (const config& err = data.child("error")) {
		throw wesnothd_error(err["message"]);
	}
}

} // end namespace mp

