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

#include "about.hpp"
#include "actions.hpp"
#include "ai.hpp"
#include "config.hpp"
#include "cursor.hpp"
#include "dialogs.hpp"
#include "display.hpp"
#include "events.hpp"
#include "filesystem.hpp"
#include "font.hpp"
#include "game_config.hpp"
#include "game_events.hpp"
#include "gamestatus.hpp"
#include "key.hpp"
#include "language.hpp"
#include "log.hpp"
#include "mapgen.hpp"
#include "multiplayer.hpp"
#include "multiplayer_client.hpp"
#include "network.hpp"
#include "pathfind.hpp"
#include "playlevel.hpp"
#include "preferences.hpp"
#include "replay.hpp"
#include "show_dialog.hpp"
#include "sound.hpp"
#include "statistics.hpp"
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

	config* scenario = NULL;

	config starting_pos;

	//see if we load the scenario from the scenario data -- if there is
	//no snapshot data available from a save, or if the user has selected
	//to view the replay from scratch
	if(state.snapshot.child("side") == NULL || !recorder.at_end()) {
		//if the starting state is specified, then use that,
		//otherwise get the scenario data and start from there.
		if(state.starting_pos.empty() == false) {
			std::cerr << "loading starting position: '" << state.starting_pos.write() << "'\n";
			starting_pos = state.starting_pos;
			scenario = &starting_pos;
		} else {
			scenario = game_config.find_child(type,"id",state.scenario);
		}
	} else {
		std::cerr << "loading snapshot...\n";
		//load from a save-snapshot.
		starting_pos = state.snapshot;
		scenario = &starting_pos;
		state = read_game(units_data,&state.snapshot);
	}

	while(scenario != NULL) {

		const config::child_list& story = scenario->get_children("story");
		const std::string current_scenario = state.scenario;

		try {
			LEVEL_RESULT res = REPLAY;

			state.label = translate_string_default((*scenario)["id"],(*scenario)["name"]);

			recorder.set_save_info(state);

			while(res == REPLAY) {
				state = recorder.get_save_info();

				res = play_level(units_data,game_config,scenario,
				                 video,state,story);
			}

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
												string_table["save_replay_message"],
												string_table["save_game_label"],
												&label);
					if(should_save == 0) {
						try {
							config snapshot;

							recorder.save_game(units_data,label,snapshot,starting_pos);
						} catch(gamestatus::save_game_failed& e) {
							gui::show_dialog(disp,NULL,"",string_table["save_game_failed"],gui::MESSAGE);
							retry = true;
						};
					}
				}

				state.scenario = orig_scenario;
			}

			recorder.clear();
			state.replay_data.clear();

			if(res != VICTORY) {
				return res;
			}
		} catch(gamestatus::load_game_failed& e) {
			gui::show_dialog(disp,NULL,"","The game could not be loaded: " + e.message,gui::OK_ONLY);
			std::cerr << "The game could not be loaded: " << e.message << "\n";
			return QUIT;
		} catch(gamestatus::game_error& e) {
			gui::show_dialog(disp,NULL,"","An error occurred while playing the game: " + e.message,gui::OK_ONLY);
			std::cerr << "An error occurred while playing the game: "
			          << e.message << "\n";
			return QUIT;
		} catch(gamemap::incorrect_format_exception& e) {
			gui::show_dialog(disp,NULL,"",e.msg_,gui::OK_ONLY);
			std::cerr << "The game map could not be loaded: " << e.msg_ << "\n";
			return QUIT;
		}

		//if the scenario hasn't been set in-level, set it now.
		if(state.scenario == current_scenario)
			state.scenario = (*scenario)["next_scenario"];

	    scenario = game_config.find_child(type,"id",state.scenario);

		//if this isn't the last scenario, then save the game
		if(scenario != NULL) {
			state.label = translate_string_default((*scenario)["id"],(*scenario)["name"]);
			state.starting_pos = *scenario;

			bool retry = true;

			while(retry) {
				retry = false;

				const int should_save = dialogs::get_save_name(disp,
													string_table["save_game_message"],
													string_table["save_game_label"],
													&state.label);

				if(should_save == 0) {
					try {
						save_game(state);
					} catch(gamestatus::save_game_failed& e) {
						gui::show_dialog(disp,NULL,"",string_table["save_game_failed"],gui::MESSAGE);
						retry = true;
					}
				}
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
	srand(time(NULL));

	std::cerr << "starting play_game\n";

	CVideo video;
	const font::manager font_manager;

	const sound::manager sound_manager;
	const preferences::manager prefs_manager;
	const image::manager image_manager;
	const events::event_context main_event_context;

	std::cerr << "initialized managers\n";

	bool test_mode = false, multiplayer_mode = false, no_gui = false;

	int arg;
	for(arg = 1; arg != argc; ++arg) {
		const std::string val(argv[arg]);
		if(val.empty()) {
			continue;
		}

		if(val == "--nogui") {
			no_gui = true;
		} else if(val == "--windowed" || val == "-w") {
			preferences::set_fullscreen(false);
		} else if(val == "--fullscreen" || val == "-f") {
			preferences::set_fullscreen(true);
		} else if(val == "--multiplayer") {
			multiplayer_mode = true;
			break; //parse the rest of the arguments when we set up the game
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

	std::cerr << "parsed arguments\n";

	if(no_gui && !multiplayer_mode) {
		std::cerr << "--nogui flag is only valid with --multiplayer flag\n";
		return 0;
	}

	if(!no_gui) {
		#if !(defined(__APPLE__))
                image::set_wm_icon();
                #endif

		int video_flags = preferences::fullscreen() ? FULL_SCREEN : 0;

		std::pair<int,int> resolution = preferences::resolution();

		std::cerr << "checking mode possible...\n";
		const int bpp = video.modePossible(resolution.first,resolution.second,
		                                   16,video_flags);
	
		std::cerr << bpp << "\n";
	
		if(bpp == 0) {
	 		//Video mode not supported, maybe from bad prefs.
	 		std::cerr << "The video mode, " << resolution.first
	 		          << "x" << resolution.second << "x16 "
	 		          << "is not supported\nAttempting 1024x768x16...\n";
	 		
	 		//Attempt 1024x768.
	 		resolution.first = 1024;
	 		resolution.second = 768;
	
	 		int bpp = video.modePossible(resolution.first,resolution.second,16,video_flags);
	
			if(bpp == 0) {
				 //Attempt 1024x768.
	 			resolution.first = 1024;
	 			resolution.second = 768;
				std::cerr << "1024x768x16 is not possible.\nAttempting 800x600x16...\n";
	
				resolution.first = 800;
				resolution.second = 600;
	
				bpp = video.modePossible(resolution.first,resolution.second,16,video_flags);
			}
	
	 		if(bpp == 0) {
	 			//couldn't do 1024x768 or 800x600
	
				std::cerr << "The required video mode, " << resolution.first
				          << "x" << resolution.second << "x16 "
				          << "is not supported\n";
	
				if((video_flags&FULL_SCREEN) != 0 && argc == 0)
					std::cerr << "Try running the program with the --windowed option "
					          << "using a 16bpp X windows setting\n";
	
				if((video_flags&FULL_SCREEN) == 0 && argc == 0)
					std::cerr << "Try running with the --fullscreen option\n";
	
				return 0;
			}
		}
	
		if(bpp != 16) {
			std::cerr << "Video mode must be emulated; the game may run slowly. "
			          << "For best results, run the program on a 16 bpp display\n";
		}
	
		std::cerr << "setting mode to " << resolution.first << "x" << resolution.second << "\n";
		const int res = video.setMode(resolution.first,resolution.second,16,video_flags);
		video.setBpp(bpp);
		if(res != 16) {
			std::cerr << "required video mode, " << resolution.first << "x"
			          << resolution.second << "x16 is not supported\n";
			return 0;
		}

		cursor::set(cursor::NORMAL);
	} else {
		video.make_fake();
	}

	const cursor::manager cursor_manager;

	std::cerr << "initialized gui\n";

	//load in the game's configuration files
	preproc_map defines_map;
	defines_map["NORMAL"] = preproc_define();
	std::vector<line_source> line_src;

	config game_config;

	try {
		log_scope("loading config");
		const std::string game_cfg = preprocess_file("data/game.cfg",&defines_map,&line_src);
		game_config.read(game_cfg,&line_src);
	} catch(config::error& e) {
		display::unit_map u_map;
		config dummy_cfg("");
		display disp(u_map,video,gamemap(dummy_cfg,"1"),gamestatus(dummy_cfg,0),
		             std::vector<team>(),dummy_cfg,dummy_cfg);

		gui::show_dialog(disp,NULL,"","Error loading game configuration files: '" + e.message + "' (The game will now exit)",
		                 gui::MESSAGE);
		throw e;
	}

	game_config::load_config(game_config.child("game_config"));

	std::cerr << "parsed config files\n";

	const config::child_list& units = game_config.get_children("units");
	if(units.empty()) {
		std::cerr << "ERROR: Could not find game configuration files\n";
		std::cerr << game_config.write();
		return 0;
	}

	game_data units_data(*units[0]);

	const bool lang_res = set_language(get_locale());
	if(!lang_res) {
		std::cerr << "No translation for locale '" << get_locale()
		          << "', default to locale 'en'\n";

		const bool lang_res = set_language("en");
		if(!lang_res) {
			std::cerr << "Language data not found\n";
		}
	}

	std::cerr << "set language\n";

	if(!no_gui) {
		SDL_WM_SetCaption(string_table["game_title"].c_str(), NULL);
	}

	for(;;) {
		statistics::fresh_stats();

		sound::play_music(game_config::title_music);

		std::cerr << "started music\n";

		game_state state;

		display::unit_map u_map;
		config dummy_cfg("");
		display disp(u_map,video,gamemap(dummy_cfg,"1"),gamestatus(dummy_cfg,0),
		             std::vector<team>(),dummy_cfg,dummy_cfg);

		std::cerr << "initialized display object\n";

		if(test_mode) {
			state.campaign_type = "test";
			state.scenario = "test";

			play_game(disp,state,game_config,units_data,video);
			return 0;
		}

		//multiplayer mode skips straight into a multiplayer game, bypassing the main menu
		//it is all handled inside this 'if' statement
		if(multiplayer_mode) {

			std::string scenario = "multiplayer_test";
			std::map<int,std::string> side_types, side_controllers, side_algorithms;
			std::map<int,string_map> side_parameters;

			int sides_counted = 0;

			for(++arg; arg < argc; ++arg) {
				const std::string val(argv[arg]);
				if(val.empty()) {
					continue;
				}

				std::vector<std::string> name_value = config::split(val,'=');
				if(name_value.size() > 2) {
					std::cerr << "invalid argument '" << val << "'\n";
					return 0;
				} else if(name_value.size() == 2) {
					const std::string name = name_value.front();
					const std::string value = name_value.back();

					const std::string name_head = name.substr(0,name.size()-1);
					const char name_tail = name[name.size()-1];
					const bool last_digit = isdigit(name_tail) ? true:false;
					const int side = name_tail - '0';

					if(last_digit && side > sides_counted) {
						std::cerr << "counted sides: " << side << "\n";
						sides_counted = side;
					}

					if(name == "--scenario") {
						scenario = value;
					} else if(last_digit && name_head == "--controller") {
						side_controllers[side] = value;
					} else if(last_digit && name_head == "--algorithm") {
						side_algorithms[side] = value;
					} else if(last_digit && name_head == "--side") {
						side_types[side] = value;
					} else if(last_digit && name_head == "--parm") {
						const std::vector<std::string> name_value = config::split(value,':');
						if(name_value.size() != 2) {
							std::cerr << "argument to '" << name << "' must be in the format name:value\n";
							return 0;
						}

						side_parameters[side][name_value.front()] = name_value.back();						
					} else {
						std::cerr << "unrecognized option: '" << name << "'\n";
						return 0;
					}
				}
			}

			const config* const lvl = game_config.find_child("multiplayer","id",scenario);
			if(lvl == NULL) {
				std::cerr << "Could not find scenario '" << scenario << "'\n";
				return 0;
			}

			state.campaign_type = "multiplayer";
			state.scenario = "";
			state.snapshot = config();

			config level = *lvl;
			std::vector<config*> story;

			const config* const side = game_config.child("multiplayer_side");
			if(side == NULL) {
				std::cerr << "Could not find side\n";
				return 0;
			}

			while(level.get_children("side").size() < sides_counted) {
				std::cerr << "now adding side...\n";
				level.add_child("side");
			}

			int side_num = 1;
			for(config::child_itors itors = level.child_range("side"); itors.first != itors.second; ++itors.first, ++side_num) {
				std::map<int,std::string>::const_iterator type = side_types.find(side_num),
				                                          controller = side_controllers.find(side_num),
				                                          algorithm = side_algorithms.find(side_num);

				const config* side = type == side_types.end() ? game_config.child("multiplayer_side") :
				                                                game_config.find_child("multiplayer_side","type",type->second);
				if(side == NULL) {
					std::string side_name = (type == side_types.end() ? "default" : type->second);
					std::cerr << "Could not find side '" << side_name << "' for side " << side_num << "\n";
					return 0;
				}

				char buf[20];
				sprintf(buf,"%d",side_num);
				(*itors.first)->values["side"] = buf;

				(*itors.first)->values["canrecruit"] = "1";

				for(string_map::const_iterator i = side->values.begin(); i != side->values.end(); ++i) {
					(*itors.first)->values[i->first] = i->second;
				}

				if(controller != side_controllers.end()) {
					(*itors.first)->values["controller"] = controller->second;
				}

				if(algorithm != side_algorithms.end()) {
					(*itors.first)->values["ai_algorithm"] = algorithm->second;
				}

				//now add in any arbitrary parameters given to the side
				for(string_map::const_iterator j = side_parameters[side_num].begin(); j != side_parameters[side_num].end(); ++j) {
					(*itors.first)->values[j->first] = j->second;
				}
			}

			try {
				play_level(units_data,game_config,&level,video,state,story);
			} catch(...) {
				std::cerr << "caught error playing level...\n";
				return 0;
			}

			return 0;
		}

		recorder.clear();

		std::cerr << "showing title screen...\n";
		gui::TITLE_RESULT res = gui::show_title(disp);

		std::cerr << "title screen returned result\n";

		if(res == gui::QUIT_GAME) {
			return 0;
		} else if(res == gui::LOAD_GAME) {

			bool show_replay;

			const std::string game = dialogs::load_game_dialog(disp,&show_replay);

			if(game == "")
				continue;

			try {
				load_game(units_data,game,state);
				if(state.version != game_config::version) {
					const int res = gui::show_dialog(disp,NULL,"",
					                      string_table["version_save_message"],
					                      gui::YES_NO);
					if(res == 1)
						continue;
				}

				defines_map.clear();
				defines_map[state.difficulty] = preproc_define();
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

			std::cerr << "has snapshot: " << (state.snapshot.child("side") ? "yes" : "no") << "\n";

			//only play replay data if the user has selected to view the replay,
			//or if there is no starting position data to use.
			if(!show_replay && state.snapshot.child("side") != NULL) {
				std::cerr << "setting replay to end...\n";
				recorder.set_to_end();
				if(!recorder.at_end())
					std::cerr << "recorder is not at the end!!!\n";
			} else {

				recorder.start_replay();

				//set whether the replay is to be skipped or not
				if(show_replay) {
					recorder.set_skip(0);
				} else {
					std::cerr << "skipping...\n";
					recorder.set_skip(-1);
				}
			}

			if(state.campaign_type == "multiplayer") {
				//make all network players local
				for(config::child_itors sides = state.snapshot.child_range("side");
				    sides.first != sides.second; ++sides.first) {
					if((**sides.first)["controller"] == "network")
						(**sides.first)["controller"] = "human";
				}
				
				recorder.set_save_info(state);
				std::vector<config*> story;

				config starting_pos;
				if(recorder.at_end()) {
					starting_pos = state.snapshot;
					state.gold = -100000;
				} else {
					starting_pos = state.starting_pos;
				}

				try {
					play_level(units_data,game_config,&starting_pos,video,state,story);
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

			const config::const_child_itors campaigns = game_config.child_range("campaign");

			std::vector<std::string> campaign_names;

			for(config::const_child_iterator i = campaigns.first; i != campaigns.second; ++i) {
				std::stringstream str;
				const std::string& icon = (**i)["icon"];
				if(icon == "") {
					str << " ,";
				} else {
					str << "&" << icon << ",";
				}

				str << translate_string_default((**i)["id"],(**i)["name"]);

				campaign_names.push_back(str.str());
			}

			if(campaign_names.empty()) {
				gui::show_dialog(disp,NULL,"",string_table["error_no_campaigns"],gui::OK_ONLY);
				continue;
			}

			int res = 0;

			if(campaign_names.size() > 1) {
				res = gui::show_dialog(disp,NULL,"",
				                                 string_table["choose_campaign"],
												 gui::OK_CANCEL,&campaign_names);

				if(res == -1)
					continue;
			}

			const config& campaign = **(campaigns.first+res);

			state.scenario = campaign["first_scenario"];

			const std::string difficulty_descriptions = translate_string_default(campaign["id"] + "_difficulties",campaign["difficulty_descriptions"]);
			std::vector<std::string> difficulty_options = config::split(difficulty_descriptions,';');

			const std::vector<std::string> difficulties = config::split(campaign["difficulties"]);

			if(difficulties.empty() == false) {
				if(difficulty_options.size() != difficulties.size()) {
					difficulty_options.resize(difficulties.size());
					std::transform(difficulties.begin(),difficulties.end(),difficulty_options.begin(),translate_string);
				}

				const int res = gui::show_dialog(disp,NULL,"",
				                            string_table["difficulty_level"],
				                            gui::OK_CANCEL,&difficulty_options);
				if(res == -1)
					continue;

				state.difficulty = difficulties[res];
				defines_map.clear();
				defines_map[difficulties[res]] = preproc_define();

				//lots of people seem to get 'NORMAL' and 'MEDIUM' mixed up,
				//so we make it that if it's NORMAL, MEDIUM is also accepted
				if(difficulties[res] == "NORMAL") {
					defines_map["MEDIUM"] = preproc_define();
				}
			}
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
				gui::show_dialog(disp,NULL,"","error loading the game: " + e.message,gui::OK_ONLY);
				std::cerr << "error loading the game: " << e.message << "\n";
				return 0;
			} catch(gamestatus::game_error& e) {
				gui::show_dialog(disp,NULL,"","error while playing the game: " + e.message,gui::OK_ONLY);
				std::cerr << "error while playing the game: "
				          << e.message << "\n";
				return 0;
			} catch(network::error& e) {
				std::cerr << "caught network error...\n";
				if(e.message != "") {
					gui::show_dialog(disp,NULL,"",e.message,gui::OK_ONLY);
				}
			} catch(config::error& e) {
				std::cerr << "caught config::error...\n";
				if(e.message != "") {
					gui::show_dialog(disp,NULL,"",e.message,gui::OK_ONLY);
				}
			} catch(gamemap::incorrect_format_exception& e) {
				gui::show_dialog(disp,NULL,"",std::string("The game map could not be loaded: ") + e.msg_,gui::OK_ONLY);
				std::cerr << "The game map could not be loaded: " << e.msg_ << "\n";
				continue;
			}

			continue;
		} else if(res == gui::CHANGE_LANGUAGE) {

			std::vector<std::string> langs = get_languages();

			std::sort(langs.begin(),langs.end());

			const std::vector<std::string>::iterator current = std::find(langs.begin(),langs.end(),get_language());
			if(current != langs.end())
				*current = "*" + *current;

			const int res = gui::show_dialog(disp,NULL,"",
			                         string_table["language_button"] + ":",
			                         gui::OK_CANCEL,&langs);
			if(size_t(res) < langs.size()) {
				set_language(langs[res]);
				preferences::set_locale(langs[res]);
			}
			continue;
		} else if(res == gui::EDIT_PREFERENCES) {
			const preferences::display_manager disp_manager(&disp);
			preferences::show_preferences_dialog(disp);

			disp.redraw_everything();
			continue;
		} else if(res == gui::SHOW_ABOUT) {
			about::show_about(disp);
			continue;
		}

		//make a new game config item based on the difficulty level
		config game_config(preprocess_file("data/game.cfg", &defines_map));

		const config::child_list& units = game_config.get_children("units");
		if(units.empty()) {
			std::cerr << "ERROR: Could not find game configuration files\n";
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
