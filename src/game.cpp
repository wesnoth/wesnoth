/* $Id$ */
/*
   Copyright (C) 2003 by David White <davidnwhite@optusnet.com.au>
   Part of the Battle for Wesnoth Project http://wesnoth.whitevine.net

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "SDL.h"

#include "actions.hpp"
#include "ai.hpp"
#include "config.hpp"
#include "dialogs.hpp"
#include "display.hpp"
#include "filesystem.hpp"
#include "font.hpp"
#include "game_config.hpp"
#include "game_events.hpp"
#include "gamestatus.hpp"
#include "key.hpp"
#include "language.hpp"
#include "multiplayer.hpp"
#include "network.hpp"
#include "pathfind.hpp"
#include "playlevel.hpp"
#include "preferences.hpp"
#include "replay.hpp"
#include "show_dialog.hpp"
#include "sound.hpp"
#include "team.hpp"
#include "unit_types.hpp"
#include "unit.hpp"
#include "video.hpp"
#include "widgets/button.hpp"

#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

LEVEL_RESULT play_game(display& disp, game_state& state, config& game_config,
				       game_data& units_data, CVideo& video)
{
	std::string type = state.campaign_type;
	if(type.empty())
		type = "scenario";

	config* scenario = game_config.find_child(type,"id",state.scenario);
	while(scenario != NULL) {

		std::vector<config*>& story = scenario->children["story"];
		const std::string current_scenario = state.scenario;

		try {
			LEVEL_RESULT res = REPLAY;

			state.label = scenario->values["name"];

			recorder.set_save_info(state);

			while(res == REPLAY) {
				state = recorder.get_save_info();
				if(!recorder.empty()) {
					recorder.start_replay();
				}

				res = play_level(units_data,game_config,scenario,
				                 video,state,story);
			}

			//ask to save a replay of the game
			if(res == VICTORY || res == DEFEAT) {
				const std::string orig_scenario = state.scenario;
				state.scenario = current_scenario;

				std::string label = state.label + " replay";
				const int should_save = gui::show_dialog(disp,NULL,"",
				                string_table["save_replay_message"],
				                gui::YES_NO,NULL,NULL,
								string_table["save_game_label"],&label);
				if(should_save == 0) {
					recorder.save_game(units_data,label);
				}

				state.scenario = orig_scenario;
			}

			recorder.clear();
			state.replay_data.clear();

			if(res != VICTORY) {
				return res;
			}
		} catch(gamestatus::load_game_failed& e) {
			std::cerr << "The game could not be loaded: " << e.message << "\n";
			return QUIT;
		} catch(gamestatus::game_error& e) {
			std::cerr << "An error occurred while playing the game: "
			          << e.message << "\n";
			return QUIT;
		} catch(gamemap::incorrect_format_exception& e) {
			std::cerr << "The game map could not be loaded: " << e.msg_ << "\n";
			return QUIT;
		}

		//if the scenario hasn't been set in-level, set it now.
		if(state.scenario == current_scenario)
			state.scenario = (*scenario)["next_scenario"];

	    scenario = game_config.find_child(type,"id",state.scenario);

		//if this isn't the last scenario, then save the game
		if(scenario != NULL) {
			state.label = (*scenario)["name"];

			const int should_save = gui::show_dialog(disp,NULL,"",
			                    string_table["save_game_message"],
			                    gui::YES_NO,NULL,NULL,
								string_table["save_game_label"],&state.label);

			if(should_save == 0) {
				save_game(state);
			}
		}
	}

	if(state.scenario != "" && state.scenario != "null") {
		gui::show_dialog(disp,NULL,"",
		                 "Error - Unknown scenario: '" + state.scenario + "'");
	}

	return VICTORY;
}

int play_game(int argc, char** argv)
{
	CVideo video;
	const font::manager font_manager;
	const sound::manager sound_manager;
	const preferences::manager prefs_manager;

	bool test_mode = false;

	for(int arg = 1; arg != argc; ++arg) {
		const std::string val(argv[arg]);
		if(val.empty()) {
			continue;
		}

		if(val == "--windowed" || val == "-w") {
			preferences::set_fullscreen(false);
		} else if(val == "--fullscreen" || val == "-f") {
			preferences::set_fullscreen(true);
		} else if(val == "--test" || val == "-t") {
			test_mode = true;
		} else if(val == "--debug" || val == "-d") {
			game_config::debug = true;
		} else if(val == "--help" || val == "-h") {
			std::cout << "usage: " << argv[0]
		    << " [options] [data-directory]\n"
			<< "  -d, --debug       Shows debugging information in-game\n"
			<< "  -f, --fullscreen  Runs the game in full-screen\n"
			<< "  -h, --help        Prints this message and exits\n"
			<< "  -t, --test        Runs the game in a small example scenario\n"
			<< "  -w, --windowed    Runs the game in windowed mode\n";
			return 0;
		} else if(val == "--version" || val == "-v") {
			std::cout << "Battle for Wesnoth " << game_config::version
			          << "\n";
			return 0;
		} else if(val[0] == '-') {
			std::cerr << "unknown option: " << val << "\n";
			return 0;
		} else {
			if(!is_directory(val)) {
				std::cerr << "Could not find directory '" << val << "'\n";
				return 0;
			}

			game_config::path = val;
		}
	}

	std::map<std::string,std::string> defines_map;
	defines_map["NORMAL"] = "";
	std::vector<line_source> line_src;

	std::string game_cfg = preprocess_file("data/game.cfg",&defines_map,
	                                              &line_src);

	config game_config(game_cfg,&line_src);

	//clear game_cfg so it doesn't take up memory
	std::string().swap(game_cfg);

	const std::vector<config*>& units = game_config.children["units"];
	if(units.empty()) {
		std::cerr << "Could not find units configuration\n";
		std::cerr << game_config.write();
		return 0;
	}

	game_data units_data(*units[0]);

	const bool lang_res = set_language(get_locale(), game_config);
	if(!lang_res) {
		std::cerr << "No translation for locale '" << get_locale()
		          << "', default to locale 'en'\n";

		const bool lang_res = set_language("en", game_config);
		if(!lang_res) {
			std::cerr << "Language data not found\n";
		}
	}

	int video_flags = preferences::fullscreen() ? FULL_SCREEN : 0;

	const std::pair<int,int>& resolution = preferences::resolution();

	std::cerr << "checking mode possible...\n";
	const int bpp = video.modePossible(resolution.first,resolution.second,
	                                   16,video_flags);

	std::cerr << bpp << "\n";

	if(bpp == 0) {
		std::cerr << "The required video mode, " << resolution.first
		          << "x" << resolution.second << "x16 "
		          << "is not supported\n";

		if((video_flags&FULL_SCREEN) != 0 && argc == 0)
			std::cerr << "Try running the program with the -windowed option "
			          << "using a 16bpp X windows setting\n";

		if((video_flags&FULL_SCREEN) == 0 && argc == 0)
			std::cerr << "Try running with the -fullscreen option\n";

		return 0;
	}

	if(bpp != 16) {
		std::cerr << "Video mode must be emulated; the game may run slowly. "
		          << "For best results, run the program on a 16 bpp display\n";
	}

	std::cerr << "setting mode to " << resolution.first << "x" << resolution.second << "\n";
	const int res = video.setMode(resolution.first,resolution.second,
	                              16,video_flags);
	if(res != 16) {
		std::cerr << "required video mode, " << resolution.first << "x"
		          << resolution.second << "x16 is not supported\n";
		return 0;
	}

	SDL_WM_SetCaption(string_table["game_title"].c_str(), NULL);

	for(;;) {
		sound::play_music("wesnoth-1.ogg");

		game_state state;

		display::unit_map u_map;
		config dummy_cfg("");
		display disp(u_map,video,gamemap(dummy_cfg,"1"),gamestatus(dummy_cfg,0),
		             std::vector<team>());

		if(test_mode) {
			state.campaign_type = "test";
			state.scenario = "test";

			play_game(disp,state,game_config,units_data,video);
			return 0;
		}

		gui::TITLE_RESULT res = gui::show_title(disp);
		if(res == gui::QUIT_GAME) {
			return 0;
		} else if(res == gui::LOAD_GAME) {
			srand(SDL_GetTicks());

			const std::vector<std::string>& games = get_saves_list();

			if(games.empty()) {
				gui::show_dialog(disp,NULL,
				                 string_table["no_saves_heading"],
								 string_table["no_saves_message"],
				                 gui::OK_ONLY);
				continue;
			}

			const int res = gui::show_dialog(disp,NULL,
							 string_table["load_game_heading"],
							 string_table["load_game_message"],
					         gui::OK_CANCEL, &games);
			if(res == -1)
				continue;

			try {
				load_game(units_data,games[res],state);
				if(state.version != game_config::version) {
					const int res = gui::show_dialog(disp,NULL,"",
					                      string_table["version_save_message"],
					                      gui::YES_NO);
					if(res == 1)
						continue;
				}

				defines_map.clear();
				defines_map[state.difficulty] = "";
			} catch(gamestatus::load_game_failed& e) {
				std::cerr << "caught load_game_failed\n";
				gui::show_dialog(disp,NULL,"",
				           string_table["bad_save_message"],gui::OK_ONLY);
				continue;
			} catch(config::error& e) {
				std::cerr << "caught config::error\n";
				gui::show_dialog(disp,NULL,"",
				    string_table["bad_save_message"] + ": " + e.message + "\n",
				    gui::OK_ONLY);
				continue;
			}

			recorder = replay(state.replay_data);
			if(!recorder.empty()) {
				const int res = gui::show_dialog(disp,NULL,
				               "", string_table["replay_game_message"],
							   gui::YES_NO);
				//if yes, then show the replay, otherwise
				//skip showing the replay
				if(res == 0) {
					recorder.set_skip(0);
				} else {
					std::cerr << "skipping...\n";
					recorder.set_skip(-1);
				}
			}

			if(state.campaign_type == "multiplayer") {
				recorder.set_save_info(state);
				std::vector<config*> story;

				try {
					play_level(units_data,game_config,&state.starting_pos,
					           video,state,story);
					recorder.clear();
				} catch(gamestatus::load_game_failed& e) {
					std::cerr << "error loading the game: " << e.message
					          << "\n";
					return 0;
				} catch(gamestatus::game_error& e) {
					std::cerr << "error while playing the game: "
					          << e.message << "\n";
					return 0;
				}

				continue;
			}
		} else if(res == gui::TUTORIAL) {
			state.campaign_type = "tutorial";
			state.scenario = "tutorial";
		} else if(res == gui::NEW_CAMPAIGN) {
			state.campaign_type = "scenario";
			state.scenario = "scenario1";

			static const std::string difficulties[] = {"EASY","NORMAL","HARD"};
			const int ndiff = sizeof(difficulties)/sizeof(*difficulties);
			std::vector<std::string> options;

			for(int i = 0; i != ndiff; ++i) {
				options.push_back(string_table[difficulties[i]]);
				if(options.back().empty())
					options.back() = difficulties[i];
			}

			const int res = gui::show_dialog(disp,NULL,"",
			                            string_table["difficulty_level"],
			                            gui::OK_CANCEL,&options);
			if(res == -1)
				continue;

			assert(size_t(res) < options.size());

			state.difficulty = difficulties[res];
			defines_map.clear();
			defines_map[difficulties[res]] = "";
		} else if(res == gui::MULTIPLAYER) {
			state.campaign_type = "multiplayer";
			state.scenario = "";

			std::vector<std::string> host_or_join;
			host_or_join.push_back(string_table["host_game"]);
			host_or_join.push_back(string_table["join_game"]);

			const int res = gui::show_dialog(disp,NULL,"","",gui::MESSAGE,
			                                 &host_or_join);
			
			try {
				if(res == 0) {
					play_multiplayer(disp,units_data,game_config,state);
				} else if(res == 1) {
					play_multiplayer_client(disp,units_data,game_config,state);
				}
			} catch(gamestatus::load_game_failed& e) {
				std::cerr << "error loading the game: " << e.message
				          << "\n";
				return 0;
			} catch(gamestatus::game_error& e) {
				std::cerr << "error while playing the game: "
				          << e.message << "\n";
				return 0;
			} catch(network::error& e) {
				gui::show_dialog(disp,NULL,"",e.message,gui::OK_ONLY);
			}

			continue;
		} else if(res == gui::CHANGE_LANGUAGE) {

			const std::vector<std::string>& langs = get_languages(game_config);
			const int res = gui::show_dialog(disp,NULL,"",
			                         string_table["language_button"] + ":",
			                         gui::OK_CANCEL,&langs);
			if(size_t(res) < langs.size()) {
				set_language(langs[res], game_config);
				preferences::set_locale(langs[res]);
			}
			continue;
		} else if(res == gui::EDIT_PREFERENCES) {
			const preferences::display_manager disp_manager(&disp);
			preferences::show_preferences_dialog(disp);
			continue;
		}

		//make a new game config item based on the difficulty level
		config game_config(preprocess_file("data/game.cfg", &defines_map));

		const std::vector<config*>& units = game_config.children["units"];
		if(units.empty()) {
			std::cerr << "Could not find units configuration\n";
			std::cerr << game_config.write();
			return 0;
		}

		game_data units_data(*units[0]);

		const LEVEL_RESULT result = play_game(disp,state,game_config,
		                                      units_data,video);
		if(result == VICTORY) {
			gui::show_dialog(disp,NULL,
			  string_table["end_game_heading"],
			  string_table["end_game_message"],
			  gui::OK_ONLY);
		}
	}

	return 0;
}

int main(int argc, char** argv)
{
	try {
		return play_game(argc,argv);
	} catch(CVideo::error&) {
		std::cerr << "Could not initialize video. Exiting.\n";
	} catch(config::error& e) {
		std::cerr << e.message << "\n";
	} catch(gui::button::error&) {
		std::cerr << "Could not create button: Image could not be found\n";
	} catch(CVideo::quit&) {
		//just means the game should quit
	}

	return 0;
}
