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

#include "config.hpp"
#include "dialogs.hpp"
#include "formula_string_utils.hpp"
#include "gamestatus.hpp"
#include "game_config_manager.hpp"
#include "gettext.hpp"
#include "log.hpp"
#include "mp_options.hpp"
#include "replay.hpp"
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

config initial_level_config(game_display& disp, const mp_game_settings& params,
	game_state& state)
{
	config level;

	if (params.saved_game) {
		try {
			savegame::loadgame load(disp,
				resources::config_manager->game_config(), state);
			load.load_multiplayer_game();
			load.fill_mplevel_config(level);

			resources::config_manager->
				load_game_config_for_game(state.classification());
		}
		catch (load_game_cancelled_exception){
			return config();
		} catch(config::error&) {
			return config();
		}
	} else {
		level.merge_with(params.scenario_data);
		level["turns"] = params.num_turns;
		level["difficulty"] = params.difficulty_define;
		level.add_child("multiplayer", params.to_config());

		// Convert options to events
		level.add_child_at("event", options::to_event(params.options.find_child(
			"multiplayer", "id", params.mp_scenario)), 0);
		if(!level.has_attribute("next_underlying_unit_id"))
		{
			level["next_underlying_unit_id"] = 0;
		}
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
		config& cfg = level.add_child("era", era_cfg);

		const config& custom_side = resources::config_manager->
			game_config().find_child("multiplayer_side", "id", "Custom");
		level.child("era").add_child_at("multiplayer_side", custom_side, 0);

		// Convert options to event
		cfg.add_child_at("event", options::to_event(
			params.options.find_child("era", "id", era)), 0);
	}

	// Add modifications
	const std::vector<std::string>& mods = params.active_mods;
	for (unsigned i = 0; i < mods.size(); i++) {
		config& cfg = level.add_child("modification",
			resources::config_manager->
				game_config().find_child("modification", "id", mods[i]));

		// Convert options to event
		cfg.add_child_at("event", options::to_event(
			params.options.find_child("modification", "id", mods[i])), 0);
	}

	// This will force connecting clients to be using the same version number as us.
	level["version"] = game_config::version;

	// If game was reloaded, params won't contain all required information and so we
	// need to take it from the actual level config.
	if (params.saved_game) {
		level["observer"] = level.child("multiplayer")["observer"];
		level["shuffle_sides"] = level.child("multiplayer")["shuffle_sides"];
	} else {
		level["observer"] = params.allow_observers;
		level["shuffle_sides"] = params.shuffle_sides;
	}

	if (level["objectives"].empty()) {
		level["objectives"] = "<big>" + t_string(N_("Victory:"), "wesnoth") +
			"</big>\n<span foreground=\"#00ff00\">&#8226; " +
			t_string(N_("Defeat enemy leader(s)"), "wesnoth") + "</span>";
	}

	return level;
}

void level_to_gamestate(config& level, game_state& state)
{
	// Any replay data is only temporary and should be removed from
	// the level data in case we want to save the game later.
	const config& replay_data = level.child("replay");
	config replay_data_store;
	if (replay_data) {
		replay_data_store = replay_data;
		LOG_NW << "setting replay\n";
		state.replay_data = replay_data;
		recorder = replay(replay_data_store);
		if (!recorder.empty()) {
			recorder.set_skip(false);
			recorder.set_to_end();
		}
	}

	carryover_info sides = carryover_info(state.carryover_sides_start);

	n_unit::id_manager::instance().set_save_id(level["next_underlying_unit_id"]);

	// Set random.
	const config::attribute_value& seed = level["random_seed"];
	if (!seed.empty()) {
		const unsigned calls = level["random_calls"].to_unsigned();
		sides.rng().seed_random(seed.to_int(42), calls);
	} else {
		ERR_NG << "No random seed found, random "
			"events will probably be out of sync.\n";
	}

	// Adds the starting pos to the level.
	if (!level.child("replay_start")) {
		level.add_child("replay_start", level);
		level.child("replay_start").remove_child("multiplayer", 0);
	}
	// This is important, if it does not happen, the starting position is
	// missing and will be drawn from the snapshot instead
	// (which is not what we want since we have
	// all needed information here already).
	state.replay_start() = level.child("replay_start");

	level["campaign_type"] = "multiplayer";
	state.classification().campaign_type = "multiplayer";
	state.classification().completion = level["completion"].str();
	state.classification().version = level["version"].str();

	if (const config& vars = level.child("variables")) {
		sides.set_variables(vars);
	}
	sides.get_wml_menu_items().set_menu_items(level);
	state.mp_settings().set_from_config(level);

	// Check whether it is a save-game by looking for snapshot data.
	const config& snapshot = level.child("snapshot");
	const bool saved_game = snapshot && snapshot.child("side");

	// It might be a MP campaign start-of-scenario save.
	// In this case, it's not entirely a new game, but not a save, either.
	// Check whether it is no savegame and the starting_pos
	// contains [player] information.
	bool start_of_scenario =
		!saved_game && state.replay_start().child("player");

	// If we start a fresh game, there won't be any snapshot information.
	// If however this is a savegame, we got a valid snapshot here.
	if (saved_game) {
		state.snapshot = snapshot;
		if (const config& v = snapshot.child("variables")) {
			sides.set_variables(v);
		}
		sides.get_wml_menu_items().set_menu_items(snapshot);
	}

	// In any type of reload (normal save or start-of-scenario) the players
	// could have changed and need to be replaced.
	if (saved_game || start_of_scenario){
		config::child_itors saved_sides = saved_game ?
			state.snapshot.child_range("side") :
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
	if (sides.get_variables().empty()) {
		LOG_NG << "No variables were found for the game_state." << std::endl;
	} else {
		LOG_NG << "Variables found and loaded into game_state:" << std::endl;
		LOG_NG << sides.get_variables();
	}

	state.carryover_sides_start = sides.to_config();
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

