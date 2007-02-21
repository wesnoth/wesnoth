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

	recorder.set_save_info(state);
	starting_pos = state.starting_pos;
	scenario = &starting_pos;

	controller_map controllers;

	const std::string current_scenario = state.scenario;

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
	}

	controller_map controllers;

	while(scenario != NULL) {
		const config::child_list& story = scenario->get_children("story");
		const std::string current_scenario = state.scenario;
		const std::string next_scenario = (*scenario)["next_scenario"];

		bool save_game_after_scenario = true;

		const set_random_generator generator_setter(&recorder);

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

			LEVEL_RESULT res;
			switch (io_type){
			case IO_NONE:
				res = playsingle_scenario(units_data,game_config,scenario,video,state,story, skip_replay);
				break;
			}


			state.snapshot = config();
			if (res == DEFEAT) {
				gui::show_dialog(disp, NULL,
				                 _("Defeat"),
				                 _("You have been defeated!"),
				                 gui::OK_ONLY);
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
			if(res != VICTORY)
				return res;
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
			state.scenario = next_scenario;

		scenario = game_config.find_child(type,"id",state.scenario);

		if(scenario != NULL) {
			// update the label
			std::string oldlabel = state.label;
			state.label = (*scenario)["name"];

			//if this isn't the last scenario, then save the game
			if(save_game_after_scenario) {
				state.starting_pos = *scenario;

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

			//update the replay start
			//FIXME: this should only be done if the scenario was not tweaked.
			//state.starting_pos = *scenario;
		}

		recorder.set_save_info(state);
	}

	if (!state.scenario.empty() && state.scenario != "null") {
		gui::show_error_message(disp, _("Unknown scenario: '") + state.scenario + '\'');
		return QUIT;
	}

	return VICTORY;
}

