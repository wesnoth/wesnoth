/* $Id$ */
/*
   Copyright (C) 2003-2005 by David White <dave@whitevine.net>
   Copyright (C) 2005 - 2008 by Philippe Plantier <ayin@anathas.org>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

//! @file playcampaign.cpp 
//! Controls setup, play, (auto)save and replay of campaigns.

#include "global.hpp"

#include "playcampaign.hpp"
#include "config.hpp"
#include "filesystem.hpp"
#include "game_preferences.hpp"
#include "gamestatus.hpp"
#include "map_create.hpp"
#include "playmp_controller.hpp"
#include "playsingle_controller.hpp"
#include "replay.hpp"
#include "replay_controller.hpp"
#include "log.hpp"
#include "dialogs.hpp"
#include "gettext.hpp"
#include "game_errors.hpp"
#include "sound.hpp"
#include "wml_exception.hpp"

#include <cassert>
#include <map>

#define LOG_G LOG_STREAM(info, general)
#define LOG_NG LOG_STREAM(info, engine)

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

void play_replay(display& disp, game_state& gamestate, const config& game_config,
		const game_data& units_data, CVideo& video)
{
	std::string type = gamestate.campaign_type;
	if(type.empty())
		type = "scenario";

	config const* scenario = NULL;

	// 'starting_pos' will contain the position we start the game from.
	config starting_pos;

	if (gamestate.starting_pos.empty()){
		// Backwards compatibility code for 1.2 and 1.2.1
		scenario = game_config.find_child(type,"id",gamestate.scenario);
		gamestate.starting_pos = *scenario;
	}
	recorder.set_save_info(gamestate);
	starting_pos = gamestate.starting_pos;
	scenario = &starting_pos;

	try {
		// Preserve old label eg. replay
		if (gamestate.label.empty())
			gamestate.label = (*scenario)["name"];
		//if (gamestate.abbrev.empty())
		//	gamestate.abbrev = (*scenario)["abbrev"];

		play_replay_level(units_data,game_config,scenario,video,gamestate);

		gamestate.snapshot = config();
		recorder.clear();
		gamestate.replay_data.clear();

	} catch(game::load_game_failed& e) {
		gui::show_error_message(disp, _("The game could not be loaded: ") + e.message);
	} catch(game::game_error& e) {
		gui::show_error_message(disp, _("Error while playing the game: ") + e.message);
	} catch(gamemap::incorrect_format_exception& e) {
		gui::show_error_message(disp, std::string(_("The game map could not be loaded: ")) + e.msg_);
	} catch(twml_exception& e) {
		e.show(disp);
	}
}

static void clean_saves(const std::string &label)
{
	std::vector<save_info> games = get_saves_list();
	std::string prefix = label + "-" + _("Auto-Save");
	std::cerr << "Cleaning saves with prefix '" << prefix << "'\n";
	for (std::vector<save_info>::iterator i = games.begin(); i != games.end(); i++) {
		if (i->name.compare(0, prefix.length(), prefix) == 0) {
			std::cerr << "Deleting savegame '" << i->name << "'\n";
			delete_game(i->name);
		}
	}
}

LEVEL_RESULT playsingle_scenario(const game_data& gameinfo, const config& game_config,
		const config* level, display& disp, game_state& state_of_game,
		const std::vector<config*>& story, upload_log& log, bool skip_replay)
{
	const int ticks = SDL_GetTicks();
	const int num_turns = atoi((*level)["turns"].c_str());
	LOG_NG << "creating objects... " << (SDL_GetTicks() - ticks) << "\n";
	playsingle_controller playcontroller(*level, gameinfo, state_of_game, ticks, num_turns, game_config, disp.video(), skip_replay);
	LOG_NG << "created objects... " << (SDL_GetTicks() - playcontroller.get_ticks()) << "\n";

	const LEVEL_RESULT res = playcontroller.play_scenario(story, log, skip_replay);

	if (res == DEFEAT) {
		gui::message_dialog(disp,
				    _("Defeat"),
				    _("You have been defeated!")
				    ).show();
	}

	if (!disp.video().faked() && res != QUIT && res != LEVEL_CONTINUE && res != LEVEL_CONTINUE_NO_SAVE)
		try {
			playcontroller.linger(log);
		} catch(end_level_exception& e) {
			if (e.result == QUIT) {
				return QUIT;
			}
		}

	return res;
}


LEVEL_RESULT playmp_scenario(const game_data& gameinfo, const config& game_config,
		config const* level, display& disp, game_state& state_of_game,
		const config::child_list& story, upload_log& log, bool skip_replay,
		io_type_t& io_type)
{
	const int ticks = SDL_GetTicks();
	const int num_turns = atoi((*level)["turns"].c_str());
	playmp_controller playcontroller(*level, gameinfo, state_of_game, ticks, num_turns, 
		game_config, disp.video(), skip_replay, io_type == IO_SERVER);
	const LEVEL_RESULT res = playcontroller.play_scenario(story, log, skip_replay);

	//Check if the player started as mp client and changed to host
	if (io_type == IO_CLIENT && playcontroller.is_host())
		io_type = IO_SERVER;

	if (res == DEFEAT) {
		gui::message_dialog(disp,
				    _("Defeat"),
				    _("You have been defeated!")
				    ).show();
	}

	if (!disp.video().faked() && res != QUIT) {
		if(res == LEVEL_CONTINUE || res == LEVEL_CONTINUE_NO_SAVE) {
			if(!playcontroller.is_host()) {
				// If we continue without lingering we need to 
				// make sure the host uploads the next scenario 
				// before we attempt to download it.
				playcontroller.wait_for_upload();
			}
		} else {
			try {
				playcontroller.linger(log);
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
		const game_data& units_data, upload_log &log,
		io_type_t io_type, bool skip_replay)
{
	std::string type = gamestate.campaign_type;
	if(type.empty())
		type = "scenario";

	config const* scenario = NULL;

	// 'starting_pos' will contain the position we start the game from.
	config starting_pos;

	recorder.set_save_info(gamestate);

	// Do we have any snapshot data?
	// yes => this must be a savegame
	// no  => we are starting a fresh scenario
	if(gamestate.snapshot.child("side") == NULL || !recorder.at_end()) {
		gamestate.completion = "running";
		recorder.set_save_info_completion(gamestate.completion);
		// Campaign or Multiplayer?
		// If the gamestate already contains a starting_pos, 
		// then we are starting a fresh multiplayer game. 
		// Otherwise this is the start of a campaign scenario.
		if(gamestate.starting_pos.empty() == false) {
			LOG_G << "loading starting position...\n";
			starting_pos = gamestate.starting_pos;
			scenario = &starting_pos;
		} else {
			LOG_G << "loading scenario: '" << gamestate.scenario << "'\n";
			scenario = game_config.find_child(type,"id",gamestate.scenario);
			if(scenario) {
				starting_pos = *scenario;
				gamestate.starting_pos = *scenario;
			}
			LOG_G << "scenario found: " << (scenario != NULL ? "yes" : "no") << "\n";
		}
	} else {
		// This game was started from a savegame
		LOG_G << "loading snapshot...\n";
		starting_pos = gamestate.starting_pos;
		scenario = &gamestate.snapshot;
		// When starting wesnoth --multiplayer there might be
		// no variables which leads to a segfault
		if(gamestate.snapshot.child("variables") != NULL) {
			gamestate.set_variables(*gamestate.snapshot.child("variables"));
		}
		gamestate.set_menu_items(gamestate.snapshot.get_children("menu_item"));
		// Replace game label with that from snapshot
		if (!gamestate.snapshot["label"].empty()){
			gamestate.label = gamestate.snapshot["label"];
		}
		{
			// Get the current gold values of players, so they don't start 
			// with the amount they had at the start of the scenario
			const std::vector<config*>& player_cfg = gamestate.snapshot.get_children("player");
			for (std::vector<config*>::const_iterator p = player_cfg.begin(); p != player_cfg.end(); p++){
				std::string save_id = (**p)["save_id"];
				player_info* player = gamestate.get_player(save_id);
				if (player != NULL){
					player->gold = lexical_cast <int> ((**p)["gold"]);
				}
			}
		}
		{
			// Also get the recruitment list if there are some specialties in this scenario
			const std::vector<config*>& player_cfg = gamestate.snapshot.get_children("side");
			for (std::vector<config*>::const_iterator p = player_cfg.begin(); p != player_cfg.end(); p++){
				std::string save_id = (**p)["save_id"];
				player_info* player = gamestate.get_player(save_id);
				if (player != NULL){
					const std::string& can_recruit_str = (**p)["recruit"];
					if(can_recruit_str != "") {
						player->can_recruit.clear();
						const std::vector<std::string> can_recruit = utils::split(can_recruit_str);
						std::copy(can_recruit.begin(),can_recruit.end(),std::inserter(player->can_recruit,player->can_recruit.end()));
					}
				}
			}
		}
	}

	controller_map controllers;

	if(io_type == IO_SERVER) {
		const config::child_list& sides_list = scenario->get_children("side");
		for(config::child_list::const_iterator side = sides_list.begin();
				side != sides_list.end(); ++side) {
			if ((**side)["current_player"] == preferences::login())
			{
				(**side)["controller"] = preferences::client_type();
			}
			std::string id = (**side)["save_id"];
			if(id.empty())
				continue;
			controllers[id] = player_controller((**side)["controller"],
					(**side)["description"]);
		}
	}

	while(scenario != NULL) {
		// If we are a multiplayer client, tweak the controllers
		if(io_type == IO_CLIENT) {
			if(scenario != &starting_pos) {
				starting_pos = *scenario;
				scenario = &starting_pos;
			}

			const config::child_list& sides_list = starting_pos.get_children("side");
			for(config::child_list::const_iterator side = sides_list.begin();
					side != sides_list.end(); ++side) {
				if((**side)["current_player"] == preferences::login()) {
					(**side)["controller"] = preferences::client_type();
					(**side)["persistent"] = "1";
				} else if((**side)["controller"] != "null") {
					(**side)["controller"] = "network";
					(**side)["persistent"] = "0";
				}
			}
		}

		const config::child_list& story = scenario->get_children("story");
		gamestate.next_scenario = (*scenario)["next_scenario"];

		bool save_game_after_scenario = true;

		const set_random_generator generator_setter(&recorder);
		LEVEL_RESULT res = LEVEL_CONTINUE;

		try {
			// Preserve old label eg. replay
			if (gamestate.label.empty()) {
				if (gamestate.abbrev.empty())
					gamestate.label = (*scenario)["name"];
				else {
					gamestate.label = std::string(gamestate.abbrev);
					gamestate.label.append("-");
					gamestate.label.append((*scenario)["name"]);
				}
			}

			// If the entire scenario should be randomly generated
			if((*scenario)["scenario_generation"] != "") {
				LOG_G << "randomly generating scenario...\n";
				const cursor::setter cursor_setter(cursor::WAIT);

				static config scenario2;
				scenario2 = random_generate_scenario((*scenario)["scenario_generation"], scenario->child("generator"));
				//level_ = scenario;

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
				new_level.values["map_data"] = map_data;
				scenario = &new_level;

				gamestate.starting_pos = new_level;
				LOG_G << "generated map\n";
			}

			sound::play_no_music();

			switch (io_type){
			case IO_NONE:
				res = playsingle_scenario(units_data,game_config,scenario,disp,gamestate,story,log, skip_replay);
				break;
			case IO_SERVER:
			case IO_CLIENT:
				res = playmp_scenario(units_data,game_config,scenario,disp,gamestate,story,log, skip_replay, io_type);
				break;
			}
		} catch(game::load_game_failed& e) {
			gui::show_error_message(disp, _("The game could not be loaded: ") + e.message);
			return QUIT;
		} catch(game::game_error& e) {
			gui::show_error_message(disp, _("Error while playing the game: ") + e.message);
			return QUIT;
		} catch(gamemap::incorrect_format_exception& e) {
			gui::show_error_message(disp, std::string(_("The game map could not be loaded: ")) + e.msg_);
			return QUIT;
		} catch(config::error& e) {
			std::cerr << "caught config::error...\n";
			gui::show_error_message(disp, _("Error while reading the WML: ") + e.message);
			return QUIT;
		} catch(twml_exception& e) {
			e.show(disp);
			return QUIT;
		}

		gamestate.snapshot = config();

		// Save-nagement ioptions fire on game end.
		// This means: (a) we have a victory, or
		// or (b) we're multiplayer live, in which
		// case defeat is also game end.  Someday, 
		// if MP campaigns ever work again, we might
		// need to change this test.

		// OBSERVER_END probably can be removed here if the observer disconnect
		// bug (#10077) is fixed so that when the host ends the game observers
		// again get asked if they want to save a replay of the game
		if (res == VICTORY || io_type != IO_NONE && (res == DEFEAT || res == OBSERVER_END)) {
			if (preferences::delete_saves())
				clean_saves(gamestate.label);

			if (preferences::save_replays()) {
				std::string label = gamestate.label + _(" replay");
				if (dialogs::get_save_name(disp, "", _("Name: "), &label,
					gui::OK_CANCEL, _("Save Replay"), false, false) == 0) {
				try {
						config snapshot;
						recorder.save_game(label, snapshot, gamestate.starting_pos);
					} catch(game::save_game_failed&) {
						gui::show_error_message(disp, _("The replay could not be saved"));
					}
				}
			}
		}

		recorder.clear();
		gamestate.replay_data.clear();

		// On DEFEAT, QUIT, or OBSERVER_END, we're done now
		if (res != VICTORY && res != LEVEL_CONTINUE_NO_SAVE 
			&& res != LEVEL_CONTINUE)
		{
			if (res != OBSERVER_END || gamestate.next_scenario.empty())
				return res;

			const int dlg_res = gui::dialog(disp,"Game Over",
				_("This scenario has ended. Do you want to continue the campaign?"),
				gui::YES_NO).show();
			if (dlg_res != 0)
				return res;
		}

		// Continue without saving is like a victory, 
		// but the save game dialog isn't displayed
		if(res == LEVEL_CONTINUE_NO_SAVE)
			save_game_after_scenario = false;

		// Switch to the next scenario.
		gamestate.scenario = gamestate.next_scenario;
		gamestate.rotate_random();

		if(io_type == IO_CLIENT) {
			if (gamestate.next_scenario.empty()) return res;

			// Ask for the next scenario data.
			network::send_data(config("load_next_scenario"), 0, true);
			config cfg;
			std::string msg = _("Downloading next scenario...");
			do {
				cfg.clear();
				network::connection data_res = dialogs::network_receive_dialog(disp,
						msg, cfg);
				if(!data_res) return QUIT;
			} while(cfg.child("next_scenario") == NULL);

			if(cfg.child("next_scenario")) {
				starting_pos = (*cfg.child("next_scenario"));
				scenario = &starting_pos;
				gamestate = game_state(units_data, starting_pos);
			} else {
				return QUIT;
			}
		} else {
			scenario = game_config.find_child(type,"id",gamestate.scenario);

			if(io_type == IO_SERVER && scenario != NULL) {
				starting_pos = *scenario;
				scenario = &starting_pos;

				// Tweaks sides to adapt controllers and descriptions.
				const config::child_list& sides_list = starting_pos.get_children("side");
				for(config::child_list::const_iterator side = sides_list.begin();
						side != sides_list.end(); ++side) {
					std::string id = (**side)["save_id"];
					if(id.empty()) {
						continue;
					}

					/* Update side info to match current_player info 
					 * to allow it taking the side in next scenario 
					 * and to be set in the players list on side server 
					 */
					controller_map::const_iterator ctr = controllers.find(id);
					if(ctr != controllers.end()) {
						player_info *player = gamestate.get_player(id);
						if (player) {
							(**side)["current_player"] = player->name;
							//! @todo TODO : remove (see TODO line 276 in server/game.cpp)
							(**side)["user_description"] = player->name;
						}
						(**side)["controller"] = ctr->second.controller;
					}
				}

				// Sends scenario data
				config cfg;
				cfg.add_child("store_next_scenario", *scenario);

				// Adds player information, and other state
				// information, to the configuration object
				assert(cfg.child("store_next_scenario") != NULL);
				write_game(gamestate, *cfg.child("store_next_scenario"), WRITE_SNAPSHOT_ONLY);
				network::send_data(cfg, 0, true);
			}
		}

		if(scenario != NULL) {
			// Update the label
			std::string oldlabel = gamestate.label;
			if (gamestate.abbrev.empty())
				gamestate.label = (*scenario)["name"];
			else {
				gamestate.label = std::string(gamestate.abbrev);
				gamestate.label.append("-");
				gamestate.label.append((*scenario)["name"]);
			}

			// If this isn't the last scenario, then save the game
			if(save_game_after_scenario) {

				// For multiplayer, we want the save
				// to contain the starting position.  
				// For campaigns however, this is the
				// start-of-scenario save and the
				// starting position needs to be empty,
				// to force a reload of the scenario config.
				if (gamestate.campaign_type == "multiplayer"){
					gamestate.starting_pos = *scenario;
				}
				else{
					gamestate.starting_pos = config();
				}

				bool retry = true;

				while(retry) {
					retry = false;

#ifdef TINY_GUI
					const int should_save = dialogs::get_save_name(disp,
						_("Do you want to save your game?"),
						_("Name:"),
						&gamestate.label);
					if(should_save == 0)
#endif /* TINY_GUI */
					{
						try {
							save_game(gamestate);
						} catch(game::save_game_failed&) {
							gui::show_error_message(disp, _("The game could not be saved"));
							retry = true;
						}
					}
				}
			}

			if (gamestate.campaign_type != "multiplayer"){
				gamestate.starting_pos = *scenario;
			}
		}

		recorder.set_save_info(gamestate);
	}

	if (!gamestate.scenario.empty() && gamestate.scenario != "null") {
		std::string message = _("Unknown scenario: '$scenario|'");
		utils::string_map symbols;
		symbols["scenario"] = gamestate.scenario;
		message = utils::interpolate_variables_into_string(message, &symbols);
		gui::show_error_message(disp, message);
		return QUIT;
	}

	if (gamestate.campaign_type == "scenario"){
		if (preferences::delete_saves())
			clean_saves(gamestate.label);
	}
	return VICTORY;
}

