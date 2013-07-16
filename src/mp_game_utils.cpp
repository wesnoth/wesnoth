/*
   Copyright (C) 2013 by Andrius Silinskas <silinskas.andrius@gmail.com>
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

#include "config.hpp"
#include "formula_string_utils.hpp"
#include "gamestatus.hpp"
#include "game_config_manager.hpp"
#include "gettext.hpp"
#include "log.hpp"
#include "savegame.hpp"
#include "tod_manager.hpp"
#include "unit_id.hpp"

#include <boost/foreach.hpp>

static lg::log_domain log_config("config");
#define LOG_CF LOG_STREAM(info, log_config)
#define WRN_CF LOG_STREAM(warn, log_config)
#define ERR_CF LOG_STREAM(err, log_config)

namespace mp {

config initial_level_config(game_display& disp, mp_game_settings& params,
	game_state& state)
{
	config level;

	if(params.saved_game) {
		try{
			savegame::loadgame load(disp,
				resources::config_manager->game_config(), state);
			load.load_multiplayer_game();
			load.fill_mplevel_config(level);
		}
		catch (load_game_cancelled_exception){
			return config();
		}
	} else {
		level.clear();
		params.mp_scenario = params.scenario_data["id"].str();
		level.merge_with(params.scenario_data);
		level["turns"] = params.num_turns;
		level.add_child("multiplayer", params.to_config());

		// Convert options to events
		//FIXME
		//level.add_child_at("event", mp::options::to_event(
		//	params.options.find_child("multiplayer", "id",
		//		params.mp_scenario)), 0);

		params.hash = level.hash();
		level["next_underlying_unit_id"] = 0;
		n_unit::id_manager::instance().clear();

		if (params.random_start_time)
		{
			if (!tod_manager::is_start_ToD(level["random_start_time"]))
			{
				level["random_start_time"] = true;
			}
		}
		else
		{
			level["random_start_time"] = false;
		}

		level["experience_modifier"] = params.xp_modifier;
		level["random_seed"] = state.carryover_sides_start["random_seed"];
	}

	std::string era = params.mp_era;
	if (params.saved_game) {
		if (const config &c = level.child("snapshot").child("era"))
			era = c["id"].str();
	}

	// Initialize the list of sides available for the current era.
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
		WRN_CF << "Missing era in MP load game " << era << "\n";
	}
	else
	{
		/*config& cfg =*/
		level.add_child("era", era_cfg);

		// Convert options to event
		//FIXME
		//cfg.add_child_at("event", mp::options::to_event(
		//	params.options.find_child("era", "id", era)), 0);
	}

	// Add modifications
	const std::vector<std::string>& mods = params.active_mods;
	for (unsigned i = 0; i < mods.size(); i++) {
		/*config& cfg = */
		level.add_child("modification",
			resources::config_manager->
				game_config().find_child("modification", "id", mods[i]));

		// Convert options to event
		//FIXME
		//cfg.add_child_at("event", mp::options::to_event(
		//	params.options.find_child("modification", "id", mods[i])), 0);
	}

	// This will force connecting clients to be using the same version number as us.
	level["version"] = game_config::version;

	level["observer"] = params.allow_observers;
	level["shuffle_sides"] = params.shuffle_sides;

	if(level["objectives"].empty()) {
		level["objectives"] = "<big>" + t_string(N_("Victory:"), "wesnoth") +
			"</big>\n<span foreground=\"#00ff00\">&#8226; " +
			t_string(N_("Defeat enemy leader(s)"), "wesnoth") + "</span>";
	}

	return level;
}

config next_level_config(const config& level, game_state& state)
{
	config next_level;

	config& next_level_data = next_level.add_child("store_next_scenario", level);

	// Adds player information, and other state
	// information, to the configuration object.
	state.write_snapshot(next_level_data);

	next_level_data["next_scenario"] = level["next_scenario"];
	next_level_data.add_child("snapshot");

	next_level_data["random_seed"] = state.carryover_sides_start["random_seed"];
	next_level_data["random_calls"] = state.carryover_sides_start["random_calls"];
	next_level_data.add_child("variables", state.carryover_sides_start.child("variables"));
	next_level_data.add_child("multiplayer", state.mp_settings().to_config());

	// Merge in-game information from carryover_sides_start
	// with level to create replay_start.
	next_level_data.add_child("replay_start", level);
	config gamedata;
	game_data(state.carryover_sides_start).write_snapshot(gamedata);
	next_level_data.child("replay_start").merge_with(gamedata);

	// Move side information from state into the config
	// that is sent to the other clients.
	next_level_data.clear_children("side");
	BOOST_FOREACH(const config& side, level.child_range("side")){
		next_level_data.add_child("side", side);
	}

	return next_level;
}

} // end namespace mp

