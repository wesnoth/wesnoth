/* $Id$ */
/*
   Copyright (C) 2003-2005 by David White <davidnwhite@optusnet.com.au>
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
#include "playlevel.hpp"
#include "config.hpp"
#include "gamestatus.hpp"
#include "replay.hpp"
#include "log.hpp"
#include "preferences.hpp"
#include "dialogs.hpp"
#include "gettext.hpp"
#include "game_errors.hpp"
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

LEVEL_RESULT play_game(display& disp, game_state& state, const config& game_config,
		const game_data& units_data, CVideo& video, 
		io_type_t io_type)
{
	std::string type = state.campaign_type;
	if(type.empty())
		type = "scenario";

	config const* scenario = NULL;

	//'starting_pos' will contain the position we start the game from.
	config starting_pos;

	recorder.set_save_info(state);

	//see if we load the scenario from the scenario data -- if there is
	//no snapshot data available from a save, or if the user has selected
	//to view the replay from scratch
	if(state.snapshot.child("side") == NULL || !recorder.at_end()) {
		//if the starting state is specified, then use that,
		//otherwise get the scenario data and start from there.
		if(state.starting_pos.empty() == false) {
			LOG_G << "loading starting position...\n";
			starting_pos = state.starting_pos;
			scenario = &starting_pos;
		} else {
			LOG_G << "loading scenario: '" << state.scenario << "'\n";
			scenario = game_config.find_child(type,"id",state.scenario);
			LOG_G << "scenario found: " << (scenario != NULL ? "yes" : "no") << "\n";
		}
	} else {
		LOG_G << "loading snapshot...\n";
		//load from a save-snapshot.
		starting_pos = state.snapshot;
		scenario = &starting_pos;
		state = read_game(units_data, &state.snapshot);
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
						(**side)["description"] == preferences::login()) {
					(**side)["controller"] = preferences::client_type();
				} else if((**side)["controller"] != "null") {
					(**side)["controller"] = "network";
				}
			}
		}

		const config::child_list& story = scenario->get_children("story");
		const std::string current_scenario = state.scenario;

		bool save_game_after_scenario = true;

		try {
			state.label = (*scenario)["name"];

			LEVEL_RESULT res = play_level(units_data,game_config,scenario,video,state,story);

			state.snapshot = config();

			//ask to save a replay of the game
			if(res == VICTORY || res == DEFEAT) {
				const std::string orig_scenario = state.scenario;
				state.scenario = current_scenario;

				std::string label = state.label + " replay";

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

			if(res != VICTORY) {
				return res;
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

		//if the scenario hasn't been set in-level, set it now.
		if(state.scenario == current_scenario)
			state.scenario = (*scenario)["next_scenario"];

		if(io_type == IO_CLIENT) {
			config cfg;
			do {
				cfg.clear();
				network::connection data_res = gui::network_data_dialog(disp, 
						_("Downloading next level..."), cfg);
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
					
					controller_map::const_iterator ctr = controllers.find(id);
					if(ctr != controllers.end()) {
						(**side)["description"] = ctr->second.description;
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

		//if this isn't the last scenario, then save the game
		if(scenario != NULL && save_game_after_scenario) {
			state.label = (*scenario)["name"];
			state.starting_pos = config();

			bool retry = true;

			while(retry) {
				retry = false;

				const int should_save = dialogs::get_save_name(disp,
						_("Do you want to save your game?"),
						_("Name:"),
						&state.label);

				
				if(should_save == 0) {
					try {
						save_game(state);
					} catch(game::save_game_failed&) {
						gui::show_error_message(disp, _("The game could not be saved"));
						retry = true;
					}
				}
			}
		}

		//update the replay start
		if(scenario != NULL) {
			//FIXME: this should only be done if the scenario was not tweaked.
			state.starting_pos = *scenario;
		}

		recorder.set_save_info(state);
	}

	if (!state.scenario.empty() && state.scenario != "null") {
		gui::show_error_message(disp, _("Unknown scenario: '") + state.scenario + '\'');
		return QUIT;
	}

	return VICTORY;
}

