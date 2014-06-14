/*
   Copyright (C) 2013 - 2014 by Andrius Silinskas <silinskas.andrius@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#include "mp_game_utils.hpp"

#include "carryover.hpp"
#include "config.hpp"
#include "dialogs.hpp"
#include "formula_string_utils.hpp"
#include "saved_game.hpp"
#include "game_config_manager.hpp"
#include "gettext.hpp"
#include "log.hpp"
#include "mp_options.hpp"
#include "replay.hpp"
#include "resources.hpp"
#include "savegame.hpp"
#include "tod_manager.hpp"
#include "unit_id.hpp"

#include <boost/foreach.hpp>

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

// To remove radundant informaion in the clientside internal programmflow
// I want to remove these values from mp_settings so i need to readd them here
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
	//Also impliers state.expand_scenario()
	state.expand_mp_events();
	state.expand_mp_options();

	config& scenario = state.get_starting_pos();
	if(!state.mp_settings().saved_game)
	{
		if(state.carryover_sides_start["random_seed"].str() == "")
		{
			state.carryover_sides_start["random_seed"] = rand();
			state.carryover_sides_start["random_calls"] = 0;
		}
		scenario["turns"] = params.num_turns;

		if (params.random_start_time)
		{
			if (!tod_manager::is_start_ToD(scenario["random_start_time"]))
			{
				scenario["random_start_time"] = true;
			}
		}
		else
		{
			scenario["random_start_time"] = false;
		}

		scenario["experience_modifier"] = params.xp_modifier;
	}
	
	if (scenario["objectives"].empty()) {
		scenario["objectives"] = "<big>" + t_string(N_("Victory:"), "wesnoth") +
			"</big>\n<span foreground=\"#00ff00\">&#8226; " +
			t_string(N_("Defeat enemy leader(s)"), "wesnoth") + "</span>";
	}

	config level = state.to_config();
	add_multiplayer_classification(level.child_or_add("multiplayer"), state);

	std::string era = params.mp_era;
	//[multiplayer] mp_era= shoudl be psersistent over saves.

	//[era], [modification]s are toplevel tags here, they are not part of teh saved_game and only used during mp_connect/mp_wait
	// Initialize the list of sides available for the current era.
	// We also need this no not get a segfault in mp_connect for ai configuation
	const config &era_cfg =
		resources::config_manager->game_config().find_child("era", "id", era);
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

		const config& custom_side = resources::config_manager->
			game_config().find_child("multiplayer_side", "id", "Custom");
		level.child("era").add_child_at("multiplayer_side", custom_side, 0);

	}
	// Add modifications, needed for ai aglorithms which are applied in mp_connect

	const std::vector<std::string>& mods = params.active_mods;
	for (unsigned i = 0; i < mods.size(); i++) {
		/*config& cfg = */level.add_child("modification",
			resources::config_manager->
				game_config().find_child("modification", "id", mods[i]));
	}

	
#if 0
	// we have this alredy in [multiplayer]. If removing this causes a bug than that's most likley bacause some is searchin for this information at the wrng place (not in [multiplayer]) 
	// If game was reloaded, params won't contain all required information and so we
	// need to take it from the actual level config.
	if (params.saved_game) {
		level["observer"] = level.child("multiplayer")["observer"];
		level["shuffle_sides"] = level.child("multiplayer")["shuffle_sides"];
	} else {
		level["observer"] = params.allow_observers;
		level["shuffle_sides"] = params.shuffle_sides;
	}
#endif
	
	// This will force connecting clients to be using the same version number as us.
	level["version"] = game_config::version;
	return level;
}

void level_to_gamestate(const config& level, saved_game& state)
{
	state = saved_game(level);
	state.classification().campaign_type = game_classification::MULTIPLAYER;
	// Any replay data is only temporary and should be removed from
	// the level data in case we want to save the game later.
	if (const config& replay_data = level.child("replay")) 
	{
		LOG_NW << "setting replay\n";
		recorder = replay(replay_data);
		if (!recorder.empty()) {
			recorder.set_skip(false);
			recorder.set_to_end();
		}
	}

	
	//save id setting  was moved to play_controller.
	
	// Adds the starting pos to the level.
	
	// This is important, if it does not happen, the starting position is
	// missing and will be drawn from the snapshot instead
	// (which is not what we want since we have
	// all needed information here already).
	//state.replay_start() = level.child("replay_start");


	// Check whether it is a save-game by looking for snapshot data.
	//const config& snapshot = level.child("snapshot");
	//const bool saved_game = state.mp_settings().saved_game;

	// It might be a MP campaign start-of-scenario save.
	// In this case, it's not entirely a new game, but not a save, either.
	// Check whether it is no savegame and the starting_pos
	// contains [player] information.
	// Edit: idk what this code did before, but i most liley didn't work because [replay_start] never contains [player]
	//bool start_of_scenario = !saved_game && state.replay_start().child("player");

	// In any type of reload (normal save or start-of-scenario) the players
	// could have changed and need to be replaced.
	// EDIT: We directy use the starting_pos() sides now, so no need to so this anymore.
#if 0
	if (saved_game)
	{
		config::child_itors saved_sides = saved_game ?
			state.get_starting_pos().child_range("side") :
			state.replay_start().child_range("side");
		config::const_child_itors level_sides = level.child_range("side");

		BOOST_FOREACH(config& side, saved_sides) {
			BOOST_FOREACH(const config& lside, level_sides) {
				if (side["side"] == lside["side"] &&
						(side["current_player"] != lside["current_player"] ||
						 side["controller"] != lside["controller"])) {

					side["current_player"] = lside["current_player"];
					side["id"] = lside["id"];
					side["save_id"] = lside["save_id"];
					side["controller"] = lside["controller"];
					break;
				}
			}
		}
	}
#endif
}

void check_response(network::connection res, const config& data)
{
	if (!res) {
		throw network::error(_("Connection timed out"));
	}

	if (const config& err = data.child("error")) {
		throw network::error(err["message"]);
	}
}

} // end namespace mp

