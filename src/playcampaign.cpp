/* $Id$ */
/*
   Copyright (C) 2003-2005 by David White <dave@whitevine.net>
   Copyright (C) 2005 - 2011 by Philippe Plantier <ayin@anathas.org>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 *  @file playcampaign.cpp
 *  Controls setup, play, (auto)save and replay of campaigns.
 */

#include "global.hpp"

#include "foreach.hpp"
#include "game_preferences.hpp"
#include "gui/dialogs/message.hpp"
#include "gui/dialogs/transient_message.hpp"
#include "playcampaign.hpp"
#include "map_create.hpp"
#include "playmp_controller.hpp"
#include "replay_controller.hpp"
#include "log.hpp"
#include "map_exception.hpp"
#include "dialogs.hpp"
#include "gettext.hpp"
#include "savegame.hpp"
#include "sound.hpp"
#include "wml_exception.hpp"
#include "formula_string_utils.hpp"

#define LOG_G LOG_STREAM(info, lg::general)

static lg::log_domain log_engine("engine");
#define LOG_NG LOG_STREAM(info, log_engine)

namespace {

struct player_controller
{
	player_controller() :
		controller(),
		description()
		{}

	player_controller(const std::string& controller, const std::string& description) :
		controller(controller),
		description(description)
		{}

	std::string controller;
	std::string description;
};

typedef std::map<std::string, player_controller> controller_map;

} // end anon namespace

/** Marks all the toplevel [lua] tag as preload events. */
static void preload_lua_tags(const config &game_config, config &target)
{
	BOOST_FOREACH (const config &cfg, game_config.child_range("lua"))
	{
		config &ev = target.add_child("event");
		ev["name"] = "preload";
		ev["first_time_only"] = "no";
		ev.add_child("lua", cfg);
	}
}

void play_replay(display& disp, game_state& gamestate, const config& game_config,
		CVideo& video)
{
	std::string type = gamestate.classification().campaign_type;
	if(type.empty())
		type = "scenario";

	// 'starting_pos' will contain the position we start the game from.
	config starting_pos;

	if (gamestate.starting_pos.empty()){
		// Backwards compatibility code for 1.2 and 1.2.1
		const config &scenario = game_config.find_child(type,"id",gamestate.classification().scenario);
		assert(scenario);
		gamestate.starting_pos = scenario;
	}

	// Detect replays stored by the server, since they are missing core lua tags.
	if (const config &core = game_config.child("lua"))
	{
		bool found_core = false;
		BOOST_FOREACH (const config &cfg, gamestate.starting_pos.child_range("lua")) {
			if (cfg["code"] == core["code"]) {
				found_core = true;
				break;
			}
		}
		if (!found_core)
			preload_lua_tags(game_config, gamestate.starting_pos);
	}

	starting_pos = gamestate.starting_pos;

	//for replays, use the variables specified in starting_pos
	if (const config &vars = starting_pos.child("variables")) {
		gamestate.set_variables(vars);
	}

	try {
		// Preserve old label eg. replay
		if (gamestate.classification().label.empty())
			gamestate.classification().label = starting_pos["name"];
		//if (gamestate.abbrev.empty())
		//	gamestate.abbrev = (*scenario)["abbrev"];

		play_replay_level(game_config, &starting_pos, video, gamestate);

		gamestate.snapshot = config();
		recorder.clear();
		gamestate.replay_data.clear();

	} catch(game::load_game_failed& e) {
		gui2::show_error_message(disp.video(), _("The game could not be loaded: ") + e.message);
	} catch(game::game_error& e) {
		gui2::show_error_message(disp.video(), _("Error while playing the game: ") + e.message);
	} catch(incorrect_map_format_exception& e) {
		gui2::show_error_message(disp.video(), std::string(_("The game map could not be loaded: ")) + e.msg_);
	} catch(twml_exception& e) {
		e.show(disp);
	}
}

static LEVEL_RESULT playsingle_scenario(const config& game_config,
		const config* level, display& disp, game_state& state_of_game,
		const config::const_child_itors &story, 
		bool skip_replay, end_level_data &end_level)
{
	const int ticks = SDL_GetTicks();
	const int num_turns = atoi((*level)["turns"].c_str());

	LOG_NG << "creating objects... " << (SDL_GetTicks() - ticks) << "\n";
	playsingle_controller playcontroller(*level, state_of_game, ticks, num_turns, game_config, disp.video(), skip_replay);
	LOG_NG << "created objects... " << (SDL_GetTicks() - playcontroller.get_ticks()) << "\n";

	LEVEL_RESULT res = playcontroller.play_scenario(story, skip_replay);

	end_level = playcontroller.get_end_level_data();

	if (res == DEFEAT) {
		gui2::show_transient_message(disp.video(),
				    _("Defeat"),
				    _("You have been defeated!")
				    );
	}

	if (!disp.video().faked() && res != QUIT && end_level.linger_mode)
	{
		try {
			playcontroller.linger();
		} catch(end_level_exception& e) {
			if (e.result == QUIT) {
				return QUIT;
			}
		}
	}

	return res;
}


static LEVEL_RESULT playmp_scenario(const config& game_config,
		config const* level, display& disp, game_state& state_of_game,
		const config::const_child_itors &story, bool skip_replay,
		io_type_t& io_type, end_level_data &end_level)
{
	const int ticks = SDL_GetTicks();
	const int num_turns = atoi((*level)["turns"].c_str());
	playmp_controller playcontroller(*level, state_of_game, ticks, num_turns,
		game_config, disp.video(), skip_replay, io_type == IO_SERVER);
	LEVEL_RESULT res = playcontroller.play_scenario(story, skip_replay);

	end_level = playcontroller.get_end_level_data();
	//Check if the player started as mp client and changed to host
	if (io_type == IO_CLIENT && playcontroller.is_host())
		io_type = IO_SERVER;

	if (res == DEFEAT) {
		gui2::show_transient_message(disp.video(),
				    _("Defeat"),
				    _("You have been defeated!")
				    );
	}

	if (!disp.video().faked() && res != QUIT) {
		if (!end_level.linger_mode) {
			if(!playcontroller.is_host()) {
				// If we continue without lingering we need to
				// make sure the host uploads the next scenario
				// before we attempt to download it.
				playcontroller.wait_for_upload();
			}
		} else {
			try {
				playcontroller.linger();
			} catch(end_level_exception& e) {
				if (e.result == QUIT) {
					return QUIT;
				}
			}
		}
	}

	return res;
}

LEVEL_RESULT play_game(display& disp, game_state& gamestate, const config& game_config,
		               io_type_t io_type, bool skip_replay)
{
	std::string type = gamestate.classification().campaign_type;
	if(type.empty())
		type = "scenario";

	config const* scenario = NULL;

	// 'starting_pos' will contain the position we start the game from.
	config starting_pos;

	// Do we have any snapshot data?
	// yes => this must be a savegame
	// no  => we are starting a fresh scenario
	if(gamestate.snapshot.child("side") == NULL || !recorder.at_end()) {
		gamestate.classification().completion = "running";
		// Campaign or Multiplayer?
		// If the gamestate already contains a starting_pos,
		// then we are starting a fresh multiplayer game.
		// Otherwise this is the start of a campaign scenario.
		if(gamestate.starting_pos["id"].empty() == false) {
			LOG_G << "loading starting position...\n";
			preload_lua_tags(game_config, gamestate.starting_pos);
			starting_pos = gamestate.starting_pos;
			scenario = &starting_pos;
		} else {
			//reload of the scenario, as starting_pos contains carryover information only
			LOG_G << "loading scenario: '" << gamestate.classification().scenario << "'\n";
			scenario = &game_config.find_child(type, "id", gamestate.classification().scenario);
			if (*scenario) {
				starting_pos = *scenario;
				preload_lua_tags(game_config, starting_pos);
				config temp(starting_pos);
				write_players(gamestate, temp, false, true);
				gamestate.starting_pos = temp;
				scenario = &starting_pos;
			} else
				scenario = NULL;
			LOG_G << "scenario found: " << (scenario != NULL ? "yes" : "no") << "\n";
		}
	} else {
		// This game was started from a savegame
		LOG_G << "loading snapshot...\n";
		starting_pos = gamestate.starting_pos;
		scenario = &gamestate.snapshot;
		// When starting wesnoth --multiplayer there might be
		// no variables which leads to a segfault
		if (const config &vars = gamestate.snapshot.child("variables")) {
			gamestate.set_variables(vars);
		}
		gamestate.set_menu_items(gamestate.snapshot.child_range("menu_item"));
		// Replace game label with that from snapshot
		if (!gamestate.snapshot["label"].empty()){
			gamestate.classification().label = gamestate.snapshot["label"];
		}
		// Helper for transitioning middle-of-scenario savefiles from 1.6 to 1.8.
		// To be removed for 1.10.
		if (!scenario->find_child("event", "name", "preload")) {
			LOG_NG << "Adding dummy preload event.\n";
			config &ev = gamestate.snapshot.add_child("event");
			ev["name"] = "preload";
			ev["first_time_only"] = "no";
			config &lua = ev.add_child("lua");
			lua["code"] = "wesnoth.dofile 'lua/wml-tags.lua'";
		}
	}

	controller_map controllers;

	if(io_type == IO_SERVER) {
		BOOST_FOREACH (config &side, const_cast<config *>(scenario)->child_range("side"))
		{
			if (side["current_player"] == preferences::login()) {
				side["controller"] = preferences::client_type();
			}
			std::string id = side["save_id"];
			if(id.empty())
				continue;
			controllers[id] = player_controller(side["controller"], side["id"]);
		}
	}

	while(scenario != NULL) {
		// If we are a multiplayer client, tweak the controllers
		if(io_type == IO_CLIENT) {
			if(scenario != &starting_pos) {
				starting_pos = *scenario;
				scenario = &starting_pos;
			}

			BOOST_FOREACH (config &side, starting_pos.child_range("side"))
			{
				if (side["current_player"] == preferences::login()) {
					side["controller"] = preferences::client_type();
				} else if (side["controller"] != "null") {
					// @todo: Fix logic to use network_ai controller
					// if it is networked ai
					side["controller"] = "network";
				}
			}
		}

		config::const_child_itors story = scenario->child_range("story");
		gamestate.classification().next_scenario = (*scenario)["next_scenario"];

		bool save_game_after_scenario = true;

		LEVEL_RESULT res = VICTORY;
		end_level_data end_level;

		try {
			// Preserve old label eg. replay
			if (gamestate.classification().label.empty()) {
				if (gamestate.classification().abbrev.empty())
					gamestate.classification().label = (*scenario)["name"];
				else {
					gamestate.classification().label = std::string(gamestate.classification().abbrev);
					gamestate.classification().label.append("-");
					gamestate.classification().label.append((*scenario)["name"]);
				}
			}

			// If the entire scenario should be randomly generated
			if((*scenario)["scenario_generation"] != "") {
				LOG_G << "randomly generating scenario...\n";
				const cursor::setter cursor_setter(cursor::WAIT);

				static config scenario2;
				scenario2 = random_generate_scenario((*scenario)["scenario_generation"], scenario->child("generator"));
				//level_ = scenario;
				preload_lua_tags(game_config, scenario2);
				//merge carryover information into the newly generated scenario
				config temp(scenario2);
				write_players(gamestate, temp, false, true);
				gamestate.starting_pos = temp;
				scenario = &scenario2;
			}
			std::string map_data = (*scenario)["map_data"];
			if(map_data.empty() && (*scenario)["map"] != "") {
				map_data = read_map((*scenario)["map"]);
			}

			// If the map should be randomly generated
			if(map_data.empty() && (*scenario)["map_generation"] != "") {
				const cursor::setter cursor_setter(cursor::WAIT);
				map_data = random_generate_map((*scenario)["map_generation"],scenario->child("generator"));

				// Since we've had to generate the map,
				// make sure that when we save the game,
				// it will not ask for the map to be generated again on reload
				static config new_level;
				new_level = *scenario;
				new_level["map_data"] = map_data;
				scenario = &new_level;

				//merge carryover information into the scenario
				config temp(new_level);
				write_players(gamestate, temp, false, true);
				gamestate.starting_pos = temp;
				LOG_G << "generated map\n";
			}

			sound::empty_playlist();

			//add the variables to the starting_pos unless they are already there
			const config &wmlvars = gamestate.starting_pos.child("variables");
			if (!wmlvars || wmlvars.empty()){
				gamestate.starting_pos.clear_children("variables");
				gamestate.starting_pos.add_child("variables", gamestate.get_variables());
			}

			switch (io_type){
			case IO_NONE:
				res = playsingle_scenario(game_config, scenario, disp, gamestate, story, skip_replay, end_level);
				break;
			case IO_SERVER:
			case IO_CLIENT:
				res = playmp_scenario(game_config, scenario, disp, gamestate, story, skip_replay, io_type, end_level);
				break;
			}
		} catch(game::load_game_failed& e) {
			gui2::show_error_message(disp.video(), _("The game could not be loaded: ") + e.message);
			return QUIT;
		} catch(game::game_error& e) {
			gui2::show_error_message(disp.video(), _("Error while playing the game: ") + e.message);
			return QUIT;
		} catch(incorrect_map_format_exception& e) {
			gui2::show_error_message(disp.video(), std::string(_("The game map could not be loaded: ")) + e.msg_);
			return QUIT;
		} catch(config::error& e) {
			std::cerr << "caught config::error...\n";
			gui2::show_error_message(disp.video(), _("Error while reading the WML: ") + e.message);
			return QUIT;
		} catch(twml_exception& e) {
			e.show(disp);
			return QUIT;
		}


		// Save-management options fire on game end.
		// This means: (a) we have a victory, or
		// or (b) we're multiplayer live, in which
		// case defeat is also game end.  Someday,
		// if MP campaigns ever work again, we might
		// need to change this test.
		if (res == VICTORY || (io_type != IO_NONE && res == DEFEAT)) {
			if (preferences::delete_saves())
				savegame::manager::clean_saves(gamestate.classification().label);

			if (preferences::save_replays()) {
				savegame::replay_savegame save(gamestate, preferences::compress_saves());
				save.save_game_automatic(disp.video(), true);
			}
		}

		recorder.clear();
		gamestate.replay_data.clear();

		// On DEFEAT, QUIT, or OBSERVER_END, we're done now
		if (res != VICTORY)
		{
			if (res != OBSERVER_END || gamestate.classification().next_scenario.empty()) {
				gamestate.snapshot = config();
				return res;
			}

			const int dlg_res = gui::dialog(disp,"Game Over",
				_("This scenario has ended. Do you want to continue the campaign?"),
				gui::YES_NO).show();
			if (dlg_res != 0) {
				gamestate.snapshot = config();
				return res;
			}
		}

		// Continue without saving is like a victory,
		// but the save game dialog isn't displayed
		if (!end_level.prescenario_save)
			save_game_after_scenario = false;

		// Switch to the next scenario.
		gamestate.classification().scenario = gamestate.classification().next_scenario;
		gamestate.rng().rotate_random();

		if(io_type == IO_CLIENT) {
			if (gamestate.classification().next_scenario.empty()) {
				gamestate.snapshot = config();
				return res;
			}

			// Ask for the next scenario data.
			network::send_data(config("load_next_scenario"), 0, true);
			config cfg;
			std::string msg = _("Downloading next scenario...");
			do {
				cfg.clear();
				network::connection data_res = dialogs::network_receive_dialog(disp,
						msg, cfg);
				if(!data_res) {
					gamestate.snapshot = config();
					return QUIT;
				}
			} while (!cfg.child("next_scenario"));

			if (const config &c = cfg.child("next_scenario")) {
				starting_pos = c;
				scenario = &starting_pos;
				gamestate = game_state(starting_pos);
			} else {
				gamestate.snapshot = config();
				return QUIT;
			}
		} else {
			scenario = &game_config.find_child(type, "id", gamestate.classification().scenario);
			if (!*scenario)
				scenario = NULL;
			else
			{
				starting_pos = *scenario;
				preload_lua_tags(game_config, starting_pos);
				scenario = &starting_pos;
			}

			if(io_type == IO_SERVER && scenario != NULL) {
				// Tweaks sides to adapt controllers and descriptions.
				BOOST_FOREACH (config &side, starting_pos.child_range("side"))
				{
					std::string id = side["save_id"];
					if(id.empty()) {
						id = side["id"];
					}
					if(!id.empty()) {
						/* Update side info to match current_player info
						 * to allow it taking the side in next scenario
						 * and to be set in the players list on side server
						 */
						controller_map::const_iterator ctr = controllers.find(id);
						if(ctr != controllers.end()) {
							if (const config& c = gamestate.snapshot.find_child("side", "save_id", id)) {
								side["current_player"] = c["current_player"];
							}
							side["controller"] = ctr->second.controller;
						}
					}
					if (side["controller"].empty())
						side["controller"] = "ai";
				}

				// If the entire scenario should be randomly generated
				if((*scenario)["scenario_generation"] != "") {
					LOG_G << "randomly generating scenario...\n";
					const cursor::setter cursor_setter(cursor::WAIT);

					static config scenario2;
					scenario2 = random_generate_scenario((*scenario)["scenario_generation"], scenario->child("generator"));
					//level_ = scenario;
					preload_lua_tags(game_config, scenario2);
					gamestate.starting_pos = scenario2;
					scenario = &scenario2;
				}
				std::string map_data = (*scenario)["map_data"];
				if(map_data.empty() && (*scenario)["map"] != "") {
					map_data = read_map((*scenario)["map"]);
				}

				// If the map should be randomly generated
				if(map_data.empty() && (*scenario)["map_generation"] != "") {
					const cursor::setter cursor_setter(cursor::WAIT);
					map_data = random_generate_map((*scenario)["map_generation"],scenario->child("generator"));

					// Since we've had to generate the map,
					// make sure that when we save the game,
					// it will not ask for the map to be generated again on reload
					static config new_level;
					new_level = *scenario;
					new_level["map_data"] = map_data;
					scenario = &new_level;

					gamestate.starting_pos = new_level;
					LOG_G << "generated map\n";
				}

				// Sends scenario data
				config cfg;
				config& next_cfg = cfg.add_child("store_next_scenario", *scenario);

				// Adds player information, and other state
				// information, to the configuration object
				gamestate.write_snapshot(next_cfg);
				next_cfg["next_scenario"] = (*scenario)["next_scenario"];
				next_cfg.add_child("snapshot");
				//move the player information into the hosts gamestate
				write_players(gamestate, starting_pos, true, true);
				gamestate.starting_pos = *scenario;

				next_cfg.add_child("multiplayer", gamestate.mp_settings().to_config());
				next_cfg.add_child("replay_start", gamestate.starting_pos);
				//move side information from gamestate into the config that is sent to the other clients
				next_cfg.clear_children("side");
				BOOST_FOREACH (config& side, gamestate.starting_pos.child_range("side"))
					next_cfg.add_child("side", side);

				network::send_data(cfg, 0, true);
			}
		}

		if(scenario != NULL) {
			// Update the label
			std::string oldlabel = gamestate.classification().label;
			if (gamestate.classification().abbrev.empty())
				gamestate.classification().label = (*scenario)["name"];
			else {
				gamestate.classification().label = std::string(gamestate.classification().abbrev);
				gamestate.classification().label.append("-");
				gamestate.classification().label.append((*scenario)["name"]);
			}

			// If this isn't the last scenario, then save the game
			if(save_game_after_scenario) {

				// For multiplayer, we want the save
				// to contain the starting position.
				// For campaigns however, this is the
				// start-of-scenario save and the
				// starting position needs to be empty,
				// to force a reload of the scenario config.
				if (gamestate.classification().campaign_type != "multiplayer"){
					gamestate.starting_pos = config();
				}

				//add the variables to the starting position
				gamestate.starting_pos.add_child("variables", gamestate.get_variables());

				savegame::scenariostart_savegame save(gamestate, preferences::compress_saves());

#ifdef USE_TINY_GUI
				bool retry = true;

				while(retry) {
					retry = false;
					retry = !save.save_game_interactive(disp.video()
									, _("Do you want to save your game?")
									, gui::YES_NO);
				}
#else
				save.save_game_automatic(disp.video());
#endif /* USE_TINY_GUI */
			}

			if (gamestate.classification().campaign_type != "multiplayer"){
				gamestate.starting_pos = *scenario;
				//add the variables to the starting position
				gamestate.starting_pos.add_child("variables", gamestate.get_variables());
				write_players(gamestate, gamestate.starting_pos, true, true);
			}
		}
		gamestate.snapshot = config();
	}

	if (!gamestate.classification().scenario.empty() && gamestate.classification().scenario != "null") {
		std::string message = _("Unknown scenario: '$scenario|'");
		utils::string_map symbols;
		symbols["scenario"] = gamestate.classification().scenario;
		message = utils::interpolate_variables_into_string(message, &symbols);
		gui2::show_error_message(disp.video(), message);
		return QUIT;
	}

	if (gamestate.classification().campaign_type == "scenario"){
		if (preferences::delete_saves())
			savegame::manager::clean_saves(gamestate.classification().label);
	}
	return VICTORY;
}

