/* $Id$ */
/*
   Copyright (C) 2003-2005 by David White <davidnwhite@verizon.net>
   Copyright (C) 2005 by Philippe Plantier <ayin@anathas.org>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "global.hpp"

#include <map>

#include "playcampaign.hpp"
#include "config.hpp"
#include "filesystem.hpp"
#include "gamestatus.hpp"
#include "map_create.hpp"
#include "playmp_controller.hpp"
#include "playsingle_controller.hpp"
#include "replay.hpp"
#include "replay_controller.hpp"
#include "log.hpp"
#include "preferences.hpp"
#include "dialogs.hpp"
#include "gettext.hpp"
#include "game_errors.hpp"
#include "sound.hpp"
#include "wassert.hpp"

#define LOG_G LOG_STREAM(info, general)

namespace {

struct player_controller
{
	player_controller()
	{}

	player_controller(const std::string& controller, const std::string& description) :
		controller(controller), description(description)
	{}

	std::string controller;
	std::string description;
};

typedef std::map<std::string, player_controller> controller_map;

}

void play_replay(display& disp, game_state& state, const config& game_config,
		const game_data& units_data, CVideo& video)
{
	std::string type = state.campaign_type;
	if(type.empty())
		type = "scenario";

	config const* scenario = NULL;

	//'starting_pos' will contain the position we start the game from.
	config starting_pos;

	if (state.starting_pos.empty()){
		//backwards compatibility code for 1.2 and 1.2.1
		scenario = game_config.find_child(type,"id",state.scenario);
		state.starting_pos = *scenario;
	}
	recorder.set_save_info(state);
	starting_pos = state.starting_pos;
	scenario = &starting_pos;

	try {
		// preserve old label eg. replay
		if (state.label.empty())
			state.label = (*scenario)["name"];

		play_replay_level(units_data,game_config,scenario,video,state);

		state.snapshot = config();
		recorder.clear();
		state.replay_data.clear();

	} catch(game::load_game_failed& e) {
		gui::show_error_message(disp, _("The game could not be loaded: ") + e.message);
	} catch(game::game_error& e) {
		gui::show_error_message(disp, _("Error while playing the game: ") + e.message);
	} catch(gamemap::incorrect_format_exception& e) {
		gui::show_error_message(disp, std::string(_("The game map could not be loaded: ")) + e.msg_);
	}
}

void clean_autosaves(const std::string &label)
{
	std::vector<save_info> games = get_saves_list();
	std::string prefix = label + "-" + _("Auto-Save");
	std::cerr << "Cleaning autosaves with prefix '" << prefix << "'\n";
	for (std::vector<save_info>::iterator i = games.begin(); i != games.end(); i++) {
		if (i->name.compare(0,prefix.length(),prefix) == 0) {
			std::cerr << "Deleting autosave '" << i->name << "'\n";
			delete_game(i->name);
		}
	}
}

LEVEL_RESULT play_game(display& disp, game_state& state, const config& game_config,
		const game_data& units_data, CVideo& video,
		upload_log &log,
		io_type_t io_type, bool skip_replay)
{
	std::string type = state.campaign_type;
	if(type.empty())
		type = "scenario";

	config const* scenario = NULL;

	//'starting_pos' will contain the position we start the game from.
	config starting_pos;

	recorder.set_save_info(state);

	//do we have any snapshot data?
	//yes => this must be a savegame
	//no  => we are starting a fresh scenario
	if(state.snapshot.child("side") == NULL || !recorder.at_end()) {
		//campaign or multiplayer?
		//if the gamestate already contains a starting_pos, then we are 
		//starting a fresh multiplayer game. Otherwise this is the start
		//of a campaign scenario.
		if(state.starting_pos.empty() == false) {
			LOG_G << "loading starting position...\n";
			starting_pos = state.starting_pos;
			scenario = &starting_pos;
		} else {
			LOG_G << "loading scenario: '" << state.scenario << "'\n";
			scenario = game_config.find_child(type,"id",state.scenario);
			starting_pos = *scenario;
			state.starting_pos = *scenario;
			LOG_G << "scenario found: " << (scenario != NULL ? "yes" : "no") << "\n";
		}
	} else {
		//This game was started from a savegame
		LOG_G << "loading snapshot...\n";
		starting_pos = state.starting_pos;
		scenario = &state.snapshot;
		// when starting wesnoth --multiplayer there might be 
		// no variables which leads to a segfault
		if(state.snapshot.child("variables") != NULL) {
			state.variables = *state.snapshot.child("variables");
		}
		//Replace game label with that from snapshot
		if (!state.snapshot["label"].empty()){
			state.label = state.snapshot["label"];
		}
		{
			//get the current gold values of players so they don't start with the amount
			//they had at the start of the scenario
			const std::vector<config*>& player_cfg = state.snapshot.get_children("player");
			for (std::vector<config*>::const_iterator p = player_cfg.begin(); p != player_cfg.end(); p++){
				std::string save_id = (**p)["save_id"];
				player_info* player = state.get_player(save_id);
				if (player != NULL){
					player->gold = lexical_cast <int> ((**p)["gold"]);
				}
			}
		}
		{
			//also get the recruitment list if there are some specialties in this scenario
			const std::vector<config*>& player_cfg = state.snapshot.get_children("side");
			for (std::vector<config*>::const_iterator p = player_cfg.begin(); p != player_cfg.end(); p++){
				std::string save_id = (**p)["save_id"];
				player_info* player = state.get_player(save_id);
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
			std::string id = (**side)["save_id"];
			if(id.empty())
				continue;
			controllers[id] = player_controller((**side)["controller"],
					(**side)["description"]);
		}
	}

	while(scenario != NULL) {
		//If we are a multiplayer client, tweak the controllers
		if(io_type == IO_CLIENT) {
			if(scenario != &starting_pos) {
				starting_pos = *scenario;
				scenario = &starting_pos;
			}

			const config::child_list& sides_list = starting_pos.get_children("side");
			for(config::child_list::const_iterator side = sides_list.begin();
					side != sides_list.end(); ++side) {
				if((**side)["controller"] == "network" &&
						(**side)["current_player"] == preferences::login()) {
					(**side)["controller"] = preferences::client_type();
					(**side)["persistent"] = "1";
				} else if((**side)["controller"] != "null") {
					(**side)["controller"] = "network";
					(**side)["persistent"] = "0";
				}
			}
		}

		const config::child_list& story = scenario->get_children("story");
		const std::string current_scenario = state.scenario;
		const std::string next_scenario = (*scenario)["next_scenario"];

		bool save_game_after_scenario = true;

		const set_random_generator generator_setter(&recorder);
		LEVEL_RESULT res;

		try {
			// preserve old label eg. replay
			if (state.label.empty())
				state.label = (*scenario)["name"];

			//if the entire scenario should be randomly generated
			if((*scenario)["scenario_generation"] != "") {
				LOG_G << "randomly generating scenario...\n";
				const cursor::setter cursor_setter(cursor::WAIT);

				static config scenario2;
				scenario2 = random_generate_scenario((*scenario)["scenario_generation"], scenario->child("generator"));
				//level_ = scenario;

				state.starting_pos = scenario2;
				scenario = &scenario2;
			}

			std::string map_data = (*scenario)["map_data"];
			if(map_data == "" && (*scenario)["map"] != "") {
				map_data = read_map((*scenario)["map"]);
			}

			//if the map should be randomly generated
			if(map_data == "" && (*scenario)["map_generation"] != "") {
				const cursor::setter cursor_setter(cursor::WAIT);
				map_data = random_generate_map((*scenario)["map_generation"],scenario->child("generator"));

				//since we've had to generate the map, make sure that when we save the game,
				//it will not ask for the map to be generated again on reload
				static config new_level;
				new_level = *scenario;
				new_level.values["map_data"] = map_data;
				scenario = &new_level;

				state.starting_pos = new_level;
				LOG_G << "generated map\n";
			}

			sound::play_no_music();

			switch (io_type){
			case IO_NONE:
				res = playsingle_scenario(units_data,game_config,scenario,video,state,story,log, skip_replay);
				break;
			case IO_SERVER:
			case IO_CLIENT:
				res = playmp_scenario(units_data,game_config,scenario,video,state,story,log, skip_replay);
				break;
			}


			state.snapshot = config();
			if (res == DEFEAT) {
				// tell all clients that the campaign won't continue
				if(io_type == IO_SERVER) {
					config end;
					end.add_child("end_scenarios");
					network::send_data(end);
				}
				gui::show_dialog(disp, NULL,
				                 _("Defeat"),
				                 _("You have been defeated!"),
				                 gui::OK_ONLY);
			}
			if(res == QUIT && io_type == IO_SERVER) {
					config end;
					end.add_child("end_scenarios");
					network::send_data(end);
			}
			//ask to save a replay of the game
			if(res == VICTORY || res == DEFEAT) {
				const std::string orig_scenario = state.scenario;
				state.scenario = current_scenario;

				std::string label = state.label + _(" replay");

				bool retry = true;

				while(retry) {
					retry = false;

					const int should_save = dialogs::get_save_name(disp,
							_("Do you want to save a replay of this scenario?"),
							_("Name:"),
							&label);
					if(should_save == 0) {
						try {
							config snapshot;

							recorder.save_game(label, snapshot, state.starting_pos);
						} catch(game::save_game_failed&) {
							gui::show_error_message(disp, _("The game could not be saved"));
							retry = true;
						};
					}
				}

				state.scenario = orig_scenario;
			}

			recorder.clear();
			state.replay_data.clear();

			//continue without saving is like a victory, but the
			//save game dialog isn't displayed
			if(res == LEVEL_CONTINUE_NO_SAVE) {
				res = VICTORY;
				save_game_after_scenario = false;
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
		}

		//This is the end either of the multiplayer scenario or the last campaign mission
		//or the user has been defeated. Make sure he gets an opportunity to delete the autosaves.
		if ((next_scenario == "") || (res != VICTORY)){
			const int autosave_res = gui::show_dialog(disp, NULL, _("Autosaves"), 
				_("Do you want to delete the autosaves of this scenario?"), gui::YES_NO);
			if (autosave_res == 0){
				clean_autosaves(state.label);
			}
		}

		if(res != VICTORY)
			return res;

		//if the scenario hasn't been set in-level, set it now.
		if(state.scenario == current_scenario)
			state.scenario = next_scenario;

		if(io_type == IO_CLIENT) {
			config cfg;

			std::string msg;
			if (state.scenario.empty())
				msg = _("Receiving data...");
			else
				msg = _("Downloading next scenario...");

			do {
				cfg.clear();
				network::connection data_res = gui::network_receive_dialog(disp,
						msg, cfg);
				if(!data_res)
					throw network::error(_("Connection timed out"));
			} while(cfg.child("next_scenario") == NULL &&
					cfg.child("end_scenarios") == NULL);

			if(cfg.child("next_scenario")) {
				starting_pos = (*cfg.child("next_scenario"));
				scenario = &starting_pos;
				state = read_game(units_data, scenario);
			} else if(scenario->child("end_scenarios")) {
				scenario = NULL;
				state.scenario = "null";
			} else {
				return QUIT;
			}

		} else {
			scenario = game_config.find_child(type,"id",state.scenario);

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

					/*Upadate side info to match current_player info to
					  allow it taking the side in next scenario and to be
					  set in the players list on side server */
					controller_map::const_iterator ctr = controllers.find(id);
					if(ctr != controllers.end()) {
						player_info *player = state.get_player(id);
						if (player) {
							(**side)["current_player"] = player->name;
							//TODO : remove (see TODO line 276 in server/game.cpp)
							(**side)["user_description"] = player->name;
						}
						(**side)["controller"] = ctr->second.controller;
					}
				}

				// Sends scenario data
				config cfg;
				cfg.add_child("next_scenario", *scenario);

				// Adds player information, and other state
				// information, to the configuration object
				wassert(cfg.child("next_scenario") != NULL);
				write_game(state, *cfg.child("next_scenario"), WRITE_SNAPSHOT_ONLY);
				network::send_data(cfg);

			} else if(io_type == IO_SERVER && scenario == NULL) {
				config end;
				end.add_child("end_scenarios");
				network::send_data(end);
			}
		}

		if(scenario != NULL) {
			// update the label
			std::string oldlabel = state.label;
			state.label = (*scenario)["name"];

			//if this isn't the last scenario, then save the game
			if(save_game_after_scenario) {
				//For multiplayer, we want the save to contain the starting position.
				//For campaings however, this is the start-of-scenario save and the
				//starting position needs to be empty to force a reload of the scenario
				//config.
				if (state.campaign_type == "multiplayer"){
					state.starting_pos = *scenario;
				}
				else{
					state.starting_pos = config();
				}

				bool retry = true;

				while(retry) {
					retry = false;

					const int should_save = dialogs::get_save_name(disp,
						_("Do you want to save your game? (Also erases Auto-Save files)"),
						_("Name:"),
						&state.label);


					if(should_save == 0) {
						try {
							save_game(state);
							if (!oldlabel.empty())
								clean_autosaves(oldlabel);
						} catch(game::save_game_failed&) {
							gui::show_error_message(disp, _("The game could not be saved"));
							retry = true;
						}
					}
				}
			}

			if (state.campaign_type != "multiplayer"){
				state.starting_pos = *scenario;
			}
		}

		recorder.set_save_info(state);
	}

	if (!state.scenario.empty() && state.scenario != "null") {
		gui::show_error_message(disp, _("Unknown scenario: '") + state.scenario + '\'');
		return QUIT;
	}

	return VICTORY;
}

