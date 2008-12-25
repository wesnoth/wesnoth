/* $Id$ */
/*
   Copyright (C) 2003 - 2008 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "global.hpp"

#include "SDL.h"
#include "SDL_mixer.h"

#include "about.hpp"
#include "config.hpp"
#include "config_cache.hpp"
#include "construct_dialog.hpp"
#include "cursor.hpp"
#include "dialogs.hpp"
#include "filechooser.hpp"
#include "foreach.hpp"
#include "game_display.hpp"
#include "filesystem.hpp"
#include "font.hpp"
#include "formula.hpp"
#include "game_config.hpp"
#include "game_errors.hpp"
#include "gamestatus.hpp"
#include "gettext.hpp"
#include "gui/dialogs/addon_connect.hpp"
#include "gui/dialogs/language_selection.hpp"
#include "gui/dialogs/mp_method_selection.hpp"
#include "gui/dialogs/title_screen.hpp"
#ifdef DEBUG_WINDOW_LAYOUT_GRAPHS
#include "gui/widgets/debug.hpp"
#endif
#include "gui/widgets/window.hpp"
#include "help.hpp"
#include "hotkeys.hpp"
#include "intro.hpp"
#include "language.hpp"
#include "loadscreen.hpp"
#include "log.hpp"
#include "map_exception.hpp"
#include "widgets/menu.hpp"
#include "marked-up_text.hpp"
#include "multiplayer.hpp"
#include "network.hpp"
#include "playcampaign.hpp"
#include "preferences_display.hpp"
#include "addon_management.hpp"
#include "replay.hpp"
#include "sound.hpp"
#include "statistics.hpp"
#include "thread.hpp"
#include "titlescreen.hpp"
#include "upload_log.hpp"
#include "wml_exception.hpp"
#include "wml_separators.hpp"
#include "serialization/binary_or_text.hpp"
#include "serialization/binary_wml.hpp"
#include "serialization/parser.hpp"
#include "serialization/preprocessor.hpp"
#include "serialization/string_utils.hpp"
#include "sha1.hpp"

//#ifdef _WIN32
//#include "locale.h"
//#endif

#ifdef HAVE_PYTHON
#include "ai_python.hpp"
#endif

#ifndef DISABLE_EDITOR2
#include "editor2/editor_main.hpp"
#endif

#include "wesconfig.h"

#include <clocale>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>
#include <iterator>
#include <sstream>
#include <string>


#include <boost/algorithm/string/replace.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/filter/gzip.hpp>

#define ERR_CONFIG LOG_STREAM(err, config)
#define WRN_CONFIG LOG_STREAM(warn, config)
#define DBG_CONFIG LOG_STREAM(debug, config)
#define LOG_CONFIG LOG_STREAM(info, config)
#define LOG_GENERAL LOG_STREAM(info, general)
#define WRN_GENERAL LOG_STREAM(warn, general)
#define DBG_GENERAL LOG_STREAM(debug, general)
#define ERR_NET LOG_STREAM(err, network)
#define LOG_NET LOG_STREAM(info, network)
#define ERR_FS LOG_STREAM(err, filesystem)

static bool less_campaigns_rank(const config* a, const config* b) {
	return lexical_cast_default<int>((*a)["rank"],1000) <
	       lexical_cast_default<int>((*b)["rank"],1000);
}

namespace {

class game_controller
{
public:
	game_controller(int argc, char** argv);
	~game_controller();

	game_display& disp();

	bool init_video();
	bool init_config();
	bool init_language();
	bool play_test();
	bool play_multiplayer_mode();

	void reload_changed_game_config();

	bool is_loading() const;
	bool load_game();
	void set_tutorial();

	bool new_campaign();
	bool goto_campaign();
	bool goto_multiplayer();
#ifndef DISABLE_EDITOR2
	bool goto_editor();
#endif
	bool play_multiplayer();
	bool change_language();

	void show_help();
	void show_preferences();
	void show_upload_begging();

	enum RELOAD_GAME_DATA { RELOAD_DATA, NO_RELOAD_DATA };
	void launch_game(RELOAD_GAME_DATA reload=RELOAD_DATA);
	void play_replay();
#ifndef DISABLE_EDITOR2
	editor2::EXIT_STATUS start_editor(const std::string& filename = "");
#endif
	void start_wesnothd();
	const config& game_config(){return game_config_;};

private:
	game_controller(const game_controller&);
	void operator=(const game_controller&);

	void load_game_cfg();
	void reset_translations();
	void set_unit_data();

	bool detect_video_settings(); // FIXME

	const int argc_;
	int arg_;
	const char* const * const argv_;

	//this should get destroyed *after* the video, since we want
	//to clean up threads after the display disappears.
	const threading::manager thread_manager;

	CVideo video_;

	const font::manager font_manager_;
	const preferences::manager prefs_manager_;
	const image::manager image_manager_;
	const events::event_context main_event_context_;
	const hotkey::manager hotkey_manager_;
	const upload_log::manager upload_log_manager_;
	sound::music_thinker music_thinker_;
	resize_monitor resize_monitor_;
	binary_paths_manager paths_manager_;

	std::string test_scenario_;

	bool test_mode_, multiplayer_mode_, no_gui_;
	int force_bpp_;

	config game_config_;
	preproc_map old_defines_map_;

	util::scoped_ptr<game_display> disp_;

	game_state state_;

	std::pair<int,int> resolution; // FIXME
	int bpp; // FIXME
	int video_flags; // FIXME

	std::string loaded_game_;
	bool loaded_game_show_replay_;
	bool loaded_game_cancel_orders_;

	std::string multiplayer_server_;
	bool jump_to_campaign_, jump_to_multiplayer_;
#ifndef DISABLE_EDITOR2
	bool jump_to_editor_;
#endif
	game_config::config_cache& cache_;
};

game_controller::game_controller(int argc, char** argv) :
	argc_(argc),
	arg_(1),
	argv_(argv),
	thread_manager(),
	video_(),
	font_manager_(),
	prefs_manager_(),
	image_manager_(),
	main_event_context_(),
	hotkey_manager_(),
	upload_log_manager_(),
	music_thinker_(),
	resize_monitor_(),
	paths_manager_(),
	test_scenario_("test"),
	test_mode_(false),
	multiplayer_mode_(false),
	no_gui_(false),
	force_bpp_(-1),
	game_config_(),
	old_defines_map_(),
	disp_(NULL),
	state_(),
	resolution(),
	bpp(0),
	video_flags(0),
	loaded_game_(),
	loaded_game_show_replay_(false),
	loaded_game_cancel_orders_(false),
	multiplayer_server_(),
	jump_to_campaign_(false),
	jump_to_multiplayer_(false)
#ifndef DISABLE_EDITOR2
	 ,jump_to_editor_(false)
#endif
	,cache_(game_config::config_cache::instance())
{
	bool no_music = false;
	bool no_sound = false;

	// The path can be hardcoded and it might be a relative path.
	if(!game_config::path.empty() &&
#ifdef _WIN32
		// use c_str to ensure that index 1 points to valid element since c_str() returns null-terminated string
		game_config::path.c_str()[1] != ':'
#else
		game_config::path[0] != '/'
#endif
	)
	{
		game_config::path = get_cwd() + '/' + game_config::path;
		font_manager_.update_font_path();
	}

#ifndef DISABLE_EDITOR2
	const std::string app_basename = file_name(argv[0]);
	jump_to_editor_ = app_basename.find("editor") != std::string::npos;
#endif

	for(arg_ = 1; arg_ != argc_; ++arg_) {
		const std::string val(argv_[arg_]);
		if(val.empty()) {
			continue;
		}

		if(val == "--fps") {
			preferences::set_show_fps(true);
		} else if(val == "--nocache") {
			cache_.set_use_cache(false);
		} else if(val == "--max-fps") {
			if(arg_+1 != argc_) {
				++arg_;
				int fps = lexical_cast_default<int>(argv_[arg_], 50);
				fps = std::min<int>(fps, 1000);
				fps = std::max<int>(fps, 1);
				fps = 1000 / fps;
				// increase the delay to avoid going above the maximum
				if(1000 % fps != 0) {
					++fps;
				}
				preferences::set_draw_delay(fps);
			}
		} else if(val == "--validcache") {
			cache_.set_force_valid_cache(true);
		} else if(val == "--resolution" || val == "-r") {
			if(arg_+1 != argc_) {
				++arg_;
				const std::string val(argv_[arg_]);
				const std::vector<std::string> res = utils::split(val, 'x');
				if(res.size() == 2) {
					const int xres = lexical_cast_default<int>(res.front());
					const int yres = lexical_cast_default<int>(res.back());
					if(xres > 0 && yres > 0) {
						const std::pair<int,int> resolution(xres,yres);
						preferences::set_resolution(resolution);
					}
				}
			}
		} else if(val == "--bpp") {
			if(arg_+1 != argc_) {
				++arg_;
				force_bpp_ = lexical_cast_default<int>(argv_[arg_],-1);
			}
		} else if(val == "--load" || val == "-l") {
			if(arg_+1 != argc_) {
				++arg_;
				loaded_game_ = argv_[arg_];
			}
		} else if(val == "--with-replay") {
			loaded_game_show_replay_ = true;

		} else if(val == "--nogui") {
			no_gui_ = true;
			no_sound = true;
			preferences::disable_preferences_save();
		} else if(val == "--smallgui") {
			game_config::small_gui = true;
		} else if(val == "--config-dir") {
			if (argc_ <= ++arg_)
				break;
		} else if(val == "--windowed" || val == "-w") {
			preferences::set_fullscreen(false);
		} else if(val == "--fullscreen" || val == "-f") {
			preferences::set_fullscreen(true);

		} else if(val == "--campaign" || val == "-c") {
			jump_to_campaign_ = true;

		} else if(val == "--server" || val == "-s"){
			jump_to_multiplayer_ = true;
			//Do we have any server specified ?
			if(argc_ > arg_+1){
				multiplayer_server_ = argv_[arg_+1];
				++arg_;
			//Pick the first server in config
			}else{
				if(game_config::server_list.size() > 0)
					multiplayer_server_ = preferences::network_host();
				else
					multiplayer_server_ = "";
			}

		} else if(val == "--multiplayer" || val == "-m") {
			multiplayer_mode_ = true;
			break; //parse the rest of the arguments when we set up the game
		} else if(val == "--test" || val == "-t") {
			test_mode_ = true;
			// If we have -t foo it's ambiguous whether it foo is the parameter
			// for Wesnoth or the start directory so we assume it's the starting
			// directory.
			if(arg_ + 2 < argc_ && argv_[arg_ + 1][0] != '-') {
				++arg_;
				test_scenario_ = argv_[arg_];
			}
		} else if(val == "--debug" || val == "-d") {
			game_config::debug = true;
			game_config::mp_debug = true;
#ifdef DEBUG_WINDOW_LAYOUT_GRAPHS
		 } else if (val.substr(0, 18) == "--debug-dot-level=") {
			 gui2::tdebug_layout_graph::set_level(val.substr(18));
		 } else if (val.substr(0, 19) == "--debug-dot-domain=") {
			 gui2::tdebug_layout_graph::set_level(val.substr(19));
#endif
		} else if(val == "--no-delay") {
			game_config::no_delay = true;
		} else if (val.substr(0, 6) == "--log-") {
		} else if(val == "--nosound") {
			no_sound = true;
		} else if(val == "--nomusic") {
			no_music = true;
		} else if(val == "--new-widgets") {
			// This is a hidden option to enable the new widget toolkit.
			gui2::new_widgets = true;
#ifndef DISABLE_EDITOR2
		} else if(val == "-e" || val == "--editor") {
			jump_to_editor_ = true;
#endif
		} else if(val == "--dummylocales") {
			std::cerr << "--dummylocales is deprecated use --dummy-locales instead.\n";
			game_config::use_dummylocales = true;
		} else if(val == "--dummy-locales") {
			game_config::use_dummylocales = true;
		} else if(val[0] == '-') {
			std::cerr << "unknown option: " << val << std::endl;
			throw config::error("unknown option");
		} else {
			std::cerr << "Overriding data directory with " << val << std::endl;
#ifdef _WIN32
			// use c_str to ensure that index 1 points to valid element since c_str() returns null-terminated string
			if(val.c_str()[1] == ':') {
#else				
			if(val[0] == '/') {
#endif				
				game_config::path = val;
			} else {
				game_config::path = get_cwd() + '/' + val;
			}

			if(!is_directory(game_config::path)) {
				std::cerr << "Could not find directory '" << game_config::path << "'\n";
				throw config::error("directory not found");
			}

			font_manager_.update_font_path();
		}
	}
	std::cerr << '\n';
	std::cerr << "Data directory: " << game_config::path << '\n'
	          << "User configuration directory: " << get_user_data_dir() << '\n'
	          << '\n';

	// disable sound in nosound mode, or when sound engine failed to initialize
	if (no_sound || ((preferences::sound_on() || preferences::music_on() ||
	                  preferences::turn_bell() || preferences::UI_sound_on()) &&
	                 !sound::init_sound())) {
		preferences::set_sound(false);
		preferences::set_music(false);
		preferences::set_turn_bell(false);
		preferences::set_UI_sound(false);
	}
	else if (no_music) { // else disable the music in nomusic mode
		preferences::set_music(false);
	}
}

game_display& game_controller::disp()
{
	if(disp_.get() == NULL) {
		if(get_video_surface() == NULL) {
			throw CVideo::error();
		}
		disp_.assign(game_display::create_dummy_display(video_));
	}
	return *disp_.get();
}

bool game_controller::detect_video_settings()
{
	video_flags = preferences::fullscreen() ? FULL_SCREEN : 0;
	resolution  = preferences::resolution();

	int DefaultBPP = 24;
	const SDL_VideoInfo* const video_info = SDL_GetVideoInfo();
	if(video_info != NULL && video_info->vfmt != NULL) {
		DefaultBPP = video_info->vfmt->BitsPerPixel;
	}

	std::cerr << "Checking video mode: " << resolution.first
		  << "x" << resolution.second << "x" << DefaultBPP << "...\n";
	bpp = video_.modePossible(resolution.first,resolution.second,DefaultBPP,video_flags);

	if(bpp == 0) {
		//Video mode not supported, maybe from bad prefs.
		std::cerr << "Video mode " << resolution.first
		          << "x" << resolution.second << "x" << DefaultBPP << " "
		          << "is not supported - attempting 1024x768x" << DefaultBPP << "...\n";

		//Attempt 1024x768.
		resolution.first = 1024;
		resolution.second = 768;

		bpp = video_.modePossible(resolution.first,resolution.second,DefaultBPP,video_flags);

		if(bpp == 0) {
			std::cerr << "1024x768x" << DefaultBPP << " not available - attempting 800x600x" << DefaultBPP << "...\n";

			resolution.first = 800;
			resolution.second = 600;

			bpp = video_.modePossible(resolution.first,resolution.second,DefaultBPP,video_flags);
		}

#ifdef USE_TINY_GUI
		if(bpp == 0) {
			std::cerr << "800x600x" << DefaultBPP << " not available - attempting 640x480x" << DefaultBPP << "...\n";

			resolution.first = 640;
			resolution.second = 480;

			bpp = video_.modePossible(resolution.first,resolution.second,DefaultBPP,video_flags);
		}

		if(bpp == 0) {
			std::cerr << "640x480x" << DefaultBPP << " not available - attempting 320x240x" << DefaultBPP << "...\n";

			resolution.first = 320;
			resolution.second = 240;

			bpp = video_.modePossible(resolution.first,resolution.second,DefaultBPP,video_flags);
		}
#endif

        if(game_config::small_gui && bpp == 0) {
            std::cerr << "800x600x" << DefaultBPP << " not available - attempting 800x480x" << DefaultBPP << "...\n";

            resolution.first = 800;
            resolution.second = 480;

            bpp = video_.modePossible(resolution.first,resolution.second,DefaultBPP,video_flags);
        }

		if(bpp == 0) {
			//couldn't do 1024x768 or 800x600

			std::cerr << "The required video mode, " << resolution.first
			          << "x" << resolution.second << "x" << DefaultBPP << " "
			          << "is not supported\n";

			if((video_flags&FULL_SCREEN) != 0) {
				std::cerr << "Try running the program with the --windowed option "
				          << "using a " << DefaultBPP << "bpp setting for your display adapter\n";
			}

			if((video_flags&FULL_SCREEN) == 0) {
				std::cerr << "Try running with the --fullscreen option\n";
			}

			return false;
		}
	}

	if(force_bpp_ > 0) {
		bpp = force_bpp_;
	}

	return true;
}

bool game_controller::init_video()
{
	if(no_gui_) {
		if(!multiplayer_mode_) {
			std::cerr << "--nogui flag is only valid with --multiplayer flag\n";
			return false;
		}
		video_.make_fake();
		game_config::no_delay = true;
		return true;
	}

	image::set_wm_icon();

	bool found_matching = detect_video_settings();
	if(!found_matching) {
		return false;
	}

	std::cerr << "setting mode to " << resolution.first << "x" << resolution.second << "x" << bpp << "\n";
	const int res = video_.setMode(resolution.first,resolution.second,bpp,video_flags);
	video_.setBpp(bpp);
	if(res == 0) {
		std::cerr << "required video mode, " << resolution.first << "x"
		          << resolution.second << "x" << bpp << " is not supported\n";
		return false;
	}

	return true;
}

bool game_controller::init_config()
{
	cache_.clear_defines();

	// make sure that multiplayer mode is set if command line parameter is selected
	if (multiplayer_mode_)
		cache_.add_define("MULTIPLAYER");
	load_game_cfg();

	game_config::load_config(game_config_.child("game_config"));
	hotkey::deactivate_all_scopes();
	hotkey::set_scope_active(hotkey::SCOPE_GENERAL);
	hotkey::set_scope_active(hotkey::SCOPE_GAME);

	hotkey::load_hotkeys(game_config_);
	paths_manager_.set_paths(game_config_);
	::init_textdomains(game_config_);
	about::set_about(game_config_);

	return true;
}

bool game_controller::init_language()
{
	if(!::load_language_list())
		return false;

	if (!::set_language(get_locale()))
		return false;

	if(!no_gui_) {
		std::string wm_title_string = _("The Battle for Wesnoth");
		wm_title_string += " - " + game_config::revision;
		SDL_WM_SetCaption(wm_title_string.c_str(), NULL);
	}

	hotkey::load_descriptions();

	return true;
}

bool game_controller::play_test()
{
	static bool first_time = true;

	if(test_mode_ == false) {
		return true;
	}
	if(!first_time)
		return false;

	first_time = false;

	state_.campaign_type = "test";
	state_.scenario = test_scenario_;

	try {
		load_game_cfg();
	} catch(config::error&) {
		cache_.clear_defines();
		load_game_cfg();
		return false;
	}

	try {
		upload_log nolog(false);
		play_game(disp(),state_,game_config_,nolog);
	} catch(game::load_game_exception& e) {
		loaded_game_ = e.game;
		loaded_game_show_replay_ = e.show_replay;
		loaded_game_cancel_orders_ = e.cancel_orders;
		test_mode_ = false;
		return true;
	}

	return false;
}

bool game_controller::play_multiplayer_mode()
{
	state_ = game_state();

	if(!multiplayer_mode_) {
		return true;
	}

	std::string era = "era_default";
	std::string scenario = "multiplayer_The_Freelands";
	std::map<int,std::string> side_types, side_controllers, side_algorithms;
	std::map<int,string_map> side_parameters;
	std::string turns = "50";

	size_t sides_counted = 0;

	for(++arg_; arg_ < argc_; ++arg_) {
		const std::string val(argv_[arg_]);
		if(val.empty()) {
			continue;
		}

		std::vector<std::string> name_value = utils::split(val, '=');
		if(name_value.size() > 2) {
			std::cerr << "invalid argument '" << val << "'\n";
			return false;
		} else if(name_value.size() == 2) {
			const std::string name = name_value.front();
			const std::string value = name_value.back();

			const std::string name_head = name.substr(0,name.size()-1);
			const char name_tail = name[name.size()-1];
			const bool last_digit = isdigit(name_tail) ? true:false;
			const size_t side = name_tail - '0';

			if(last_digit && side > sides_counted) {
				std::cerr << "counted sides: " << side << "\n";
				sides_counted = side;
			}

			if(name == "--scenario") {
				scenario = value;
			} else if(name == "--turns") {
				turns = value;
			} else if(name == "--era") {
				era = value;
			} else if(last_digit && name_head == "--controller") {
				side_controllers[side] = value;
			} else if(last_digit && name_head == "--algorithm") {
				side_algorithms[side] = value;
			} else if(last_digit && name_head == "--side") {
				side_types[side] = value;
			} else if(last_digit && name_head == "--parm") {
				const std::vector<std::string> name_value = utils::split(value, ':');
				if(name_value.size() != 2) {
					std::cerr << "argument to '" << name << "' must be in the format name:value\n";
					return false;
				}

				side_parameters[side][name_value.front()] = name_value.back();
			} else {
				std::cerr << "unrecognized option: '" << name << "'\n";
				return false;
			}
		} else {
			if (val == "--exit-at-end") {
				game_config::exit_at_end = true;
			}
		}
	}

	const config* const lvl = game_config_.find_child("multiplayer","id",scenario);
	if(lvl == NULL) {
		std::cerr << "Could not find scenario '" << scenario << "'\n";
		return false;
	}

	state_.campaign_type = "multiplayer";
	state_.scenario = "";
	state_.snapshot = config();

	config level = *lvl;
	std::vector<config*> story;

	const config* const era_cfg = game_config_.find_child("era","id",era);
	if(era_cfg == NULL) {
		std::cerr << "Could not find era '" << era << "'\n";
		return false;
	}

	level["turns"] = turns;

	const config* const side = era_cfg->child("multiplayer_side");
	if(side == NULL) {
		std::cerr << "Could not find multiplayer side\n";
		return false;
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

		const config* side = type == side_types.end() ?
			era_cfg->find_child("multiplayer_side", "random_faction", "yes") :
			era_cfg->find_child("multiplayer_side", "id", type->second);

		if (side == NULL) {
			std::string side_name = (type == side_types.end() ? "default" : type->second);
			std::cerr << "Could not find side '" << side_name << "' for side " << side_num << "\n";
			return false;
		}

		if (utils::string_bool((*side)["random_faction"])) {
			const config::child_list& factions = era_cfg->get_children("multiplayer_side");
			std::vector<std::string> faction_choices, faction_excepts;
			faction_choices = utils::split((*side)["choices"]);
			if(faction_choices.size() == 1 && faction_choices.front() == "") {
				faction_choices.clear();
			}
			faction_excepts = utils::split((*side)["except"]);;
			if(faction_excepts.size() == 1 && faction_excepts.front() == "") {
				faction_excepts.clear();
			}
			for(unsigned int i = 0, j = 0; i < factions.size(); ++i) {
				if (utils::string_bool((*factions[i])["random_faction"]) != true) {
					const std::string& faction_id = (*factions[i])["id"];
					if (
						!faction_choices.empty() &&
						std::find(faction_choices.begin(),faction_choices.end(),faction_id) == faction_choices.end()
					)
						continue;
					if (
						!faction_excepts.empty() &&
						std::find(faction_excepts.begin(),faction_excepts.end(),faction_id) != faction_excepts.end()
					)
						continue;
					j++;
					if (rand()%j == 0) {
						side = factions[i];
					}
				}
			}
			if (utils::string_bool((*side)["random_faction"], false) == true) {
				std::string side_name = (type == side_types.end() ? "default" : type->second);
				std::cerr << "Could not find any non-random faction for side " << side_num << "\n";
				return false;
			}
			std::cerr << " Faction " << (*side)["name"] << 
				" selected for side " << side_num << ".\n";
		}

		char buf[20];
		snprintf(buf,sizeof(buf),"%d",side_num);
		(*itors.first)->values["side"] = buf;

		(*itors.first)->values["canrecruit"] = "yes";

		(*itors.first)->append(*side);

		if(controller != side_controllers.end()) {
			(*itors.first)->values["controller"] = controller->second;
		}

		if(algorithm != side_algorithms.end()) {
			(*itors.first)->values["ai_algorithm"] = algorithm->second;
		}

		config& ai_params = (*itors.first)->add_child("ai");

		//now add in any arbitrary parameters given to the side
		for(string_map::const_iterator j = side_parameters[side_num].begin(); j != side_parameters[side_num].end(); ++j) {
			(*itors.first)->values[j->first] = j->second;
			ai_params[j->first] = j->second;
		}
	}

	try {
		upload_log nolog(false);
		state_.snapshot = level;
		play_game(disp(),state_,game_config_,nolog);
	} catch(game::error& e) {
		std::cerr << "caught error: '" << e.message << "'\n";
	} catch(game::load_game_exception& e) {
		//the user's trying to load a game, so go into the normal title screen loop and load one
		loaded_game_ = e.game;
		loaded_game_show_replay_ = e.show_replay;
		loaded_game_cancel_orders_ = e.cancel_orders;
		return true;
	} catch(twml_exception& e) {
		e.show(disp());
		return false;
	} catch(...) {
		std::cerr << "caught unknown error playing level...\n";
	}

	return false;
}

bool game_controller::is_loading() const
{
	return loaded_game_.empty() == false;
}

bool game_controller::load_game()
{
	state_ = game_state();

	bool show_replay = loaded_game_show_replay_;
	bool cancel_orders = loaded_game_cancel_orders_;

	const std::string game = loaded_game_.empty() ? dialogs::load_game_dialog(disp(),game_config_,&show_replay,&cancel_orders) : loaded_game_;

	loaded_game_ = "";

	if(game == "") {
		return false;
	}

	try {
		//to load a save file, we first load the file in, then we re-parse game
		//data with the save's #defines, and then we finally parse the save file,
		//with the game data ready to go.

		config cfg;
		std::string error_log;
		read_save_file(game,cfg,&error_log);
		if(!error_log.empty()) {
			gui::show_error_message(disp(),
					_("Warning: The file you have tried to load is corrupt. Loading anyway.\n") +
					error_log);
		}

		cache_.clear_defines();
		game_config::scoped_preproc_define dificulty_def(cfg["difficulty"]);

		const std::string& campaign_define = cfg["campaign_define"];

		game_config::scoped_preproc_define campaign_define_def(campaign_define, !campaign_define.empty());

		game_config::scoped_preproc_define campaign_type_def("MULTIPLAYER", campaign_define.empty() && (cfg["campaign_type"] == "multiplayer"));


		const std::vector<std::string> campaign_xtra_defines = utils::split(cfg["campaign_extra_defines"]);

		typedef boost::shared_ptr<game_config::scoped_preproc_define> define_ptr;
		std::deque<define_ptr> extra_defines;
		for(std::vector<std::string>::const_iterator i = campaign_xtra_defines.begin(); i != campaign_xtra_defines.end(); ++i) {
			define_ptr newdefine(new game_config::scoped_preproc_define(*i));
			extra_defines.push_back(newdefine);
		}

		try {
			load_game_cfg();
		} catch(config::error&) {
			cache_.clear_defines();
			load_game_cfg();
			return false;
		}

		const std::string version = cfg["version"];
		if(version != game_config::version) {
			// do not load if too old, if either the savegame or the current game
			// has the version 'test' allow loading
			if(!game_config::is_compatible_savegame_version(version)) {
				/* GCC-3.3 needs a temp var otherwise compilation fails */
				gui::message_dialog dlg(disp(), "", _("This save is from a version too old to be loaded."));
				dlg.show();
				return false;
			}

			const int res = gui::dialog(disp(),"",
			                      _("This save is from a different version of the game. Do you want to try to load it?"),
			                      gui::YES_NO).show();
			if(res == 1) {
				return false;
			}
		}

		state_ = game_state(cfg, show_replay);

		// Get the status of the random in the snapshot.
		// For a replay we need to restore the start only, the replaying gets at
		// proper location.
		// For normal loading also restore the call count.
		const int seed = lexical_cast_default<int>
			(cfg["random_seed"], 42);
		const unsigned calls = show_replay ? 0 :
			lexical_cast_default<unsigned> (state_.snapshot["random_calls"]);
		state_.rng().seed_random(seed, calls);

	} catch(config::error& e) {
		gui::show_error_message(disp(), _("The file you have tried to load is corrupt: '") + e.message + '\'');
		return false;
	} catch(game::error& e) {
		gui::show_error_message(disp(), _("The file you have tried to load is corrupt: '") + e.message + '\'');
		return false;
	} catch(io_exception&) {
		gui::show_error_message(disp(), _("File I/O Error while reading the game"));
		return false;
	} catch(twml_exception& e) {
		e.show(disp());
		return false;
	}
	recorder = replay(state_.replay_data);
	recorder.start_replay();
	recorder.set_skip(false);

	LOG_CONFIG << "has snapshot: " << (state_.snapshot.child("side") ? "yes" : "no") << "\n";

	if(state_.snapshot.child("side") == NULL) {
		// No snapshot; this is a start-of-scenario
		if (show_replay) {
			// There won't be any turns to replay, but the
			// user gets to watch the intro sequence again ...
			LOG_CONFIG << "replaying (start of scenario)\n";
		} else {
			LOG_CONFIG << "skipping...\n";
			recorder.set_skip(false);
		}
	} else {
		// We have a snapshot. But does the user want to see a replay?
		if(show_replay) {
			statistics::clear_current_scenario();
			LOG_CONFIG << "replaying (snapshot)\n";
		} else {
			LOG_CONFIG << "setting replay to end...\n";
			recorder.set_to_end();
			if(!recorder.at_end()) {
				WRN_CONFIG << "recorder is not at the end!!!\n";
			}
		}
	}

	if(state_.campaign_type == "multiplayer") {
		for(config::child_itors sides = state_.snapshot.child_range("side");
		    sides.first != sides.second; ++sides.first) {
			if((**sides.first)["controller"] == "network")
				(**sides.first)["controller"] = "human";
			if((**sides.first)["controller"] == "network_ai")
				(**sides.first)["controller"] = "human_ai";
		}
	}

	if (cancel_orders) {
		for(config::child_itors sides = state_.snapshot.child_range("side");
			    sides.first != sides.second; ++sides.first) {
				if((**sides.first)["controller"] == "human") {
					for (config::child_itors units = (**sides.first).child_range("unit");
							units.first != units.second; ++units.first) {
						(**units.first)["goto_x"] = "-999";
						(**units.first)["goto_y"] = "-999";
					}
				}
		}
	}

	return true;
}

void game_controller::set_tutorial()
{
	state_ = game_state();
	state_.campaign_type = "tutorial";
	state_.scenario = "tutorial";
	state_.campaign_define = "TUTORIAL";
	cache_.clear_defines();
	cache_.add_define("TUTORIAL");

}

bool game_controller::new_campaign()
{
	state_ = game_state();
	state_.campaign_type = "scenario";

	config::child_list campaigns = game_config_.get_children("campaign");
	std::sort(campaigns.begin(),campaigns.end(),less_campaigns_rank);

	std::vector<std::string> campaign_names;
	std::vector<std::pair<std::string,std::string> > campaign_desc;

	for(config::child_list::const_iterator i = campaigns.begin(); i != campaigns.end(); ++i) {
		std::stringstream str;
		const std::string& icon = (**i)["icon"];
		const std::string desc = (**i)["description"];
		const std::string image = (**i)["image"];
		if(icon.empty()) {
			str << COLUMN_SEPARATOR;
		} else {
			str << IMAGE_PREFIX << icon << COLUMN_SEPARATOR;
		}

		str << (**i)["name"];

		campaign_names.push_back(str.str());
		campaign_desc.push_back(std::pair<std::string,std::string>(desc,image));
	}

	if(campaign_names.size() <= 0) {
	  gui::show_error_message(disp(),
				  _("No campaigns are available.\n"));
		return false;
	}
	dialogs::campaign_preview_pane campaign_preview(disp().video(),&campaign_desc);
	gui::dialog cmenu(disp(), _("Play a campaign"), " ", gui::OK_CANCEL);
	cmenu.set_menu(campaign_names);
	cmenu.add_pane(&campaign_preview);
	gui::dialog::dimension_measurements dim = cmenu.layout();
	Uint16 screen_width = screen_area().w;
	Uint16 dialog_width = cmenu.get_frame().get_layout().exterior.w;
	if(screen_width < 850 && screen_width - dialog_width > 20) {
		// On small resolutions, reduce the amount of unused horizontal space
		campaign_preview.set_width(campaign_preview.width() + screen_width - dialog_width - 20);
		dim = cmenu.layout();
	}
	SDL_Rect& preview_loc = dim.panes[&campaign_preview];
	preview_loc.y = dim.menu_y;
	if(dim.menu_height > 0) {
		preview_loc.h = dim.menu_height;
	} else {
		preview_loc.h = cmenu.get_menu().height();
	}
	cmenu.set_layout(dim);

	if(cmenu.show() == -1) {
		return false;
	}

	const config& campaign = *campaigns[cmenu.result()];

	state_.campaign = campaign["id"];
	state_.abbrev = campaign["abbrev"];
	state_.scenario = campaign["first_scenario"];
	state_.end_text = campaign["end_text"];
	state_.end_text_duration = lexical_cast_default<unsigned int>(campaign["end_text_duration"]);

	const std::string difficulty_descriptions = campaign["difficulty_descriptions"];
	std::vector<std::string> difficulty_options = utils::split(difficulty_descriptions, ';');

	const std::vector<std::string> difficulties = utils::split(campaign["difficulties"]);

	if(difficulties.empty() == false) {
		if(difficulty_options.size() != difficulties.size()) {
			difficulty_options.resize(difficulties.size());
			std::copy(difficulties.begin(),difficulties.end(),difficulty_options.begin());
		}

		gui::dialog dlg(disp(), _("Difficulty"),
			_("Select difficulty level:"), gui::OK_CANCEL);
		dlg.set_menu(difficulty_options);
		if(dlg.show() == -1) {
			// canceled difficulty dialog, relaunch the campaign selection dialog
			return new_campaign();
		}

		state_.difficulty = difficulties[dlg.result()];
		cache_.clear_defines();
		cache_.add_define(difficulties[dlg.result()]);
	}

	state_.campaign_define = campaign["define"];
	state_.campaign_xtra_defines = utils::split(campaign["extra_defines"]);

	return true;
}

}

bool game_controller::goto_campaign()
{
	if(jump_to_campaign_){
		jump_to_campaign_ = false;
		if(new_campaign()) {
			launch_game(game_controller::RELOAD_DATA);
		}else{
			return false;
		}
	}
	return true;
}

bool game_controller::goto_multiplayer()
{
	if(jump_to_multiplayer_){
		jump_to_multiplayer_ = false;
		if(play_multiplayer()){
			;
		}else{
			return false;
		}
	}
	return true;
}

#ifndef DISABLE_EDITOR2
bool game_controller::goto_editor()
{
	if(jump_to_editor_){
		jump_to_editor_ = false;
#ifdef _WIN32
		boost::replace_all(loaded_game_, "\\", "/");
#endif
		if (start_editor(loaded_game_) == editor2::EXIT_QUIT_TO_DESKTOP) {
			return false;
		}
		loaded_game_ = "";	
	}
	return true;
}
#endif

namespace
{
	void game_controller::reload_changed_game_config()
	{
		// rebuild addon version info cache
		refresh_addon_version_info_cache();

		//force a reload of configuration information
		cache_.recheck_filetree_checksum();
		old_defines_map_.clear();
		clear_binary_paths_cache();
		init_config();
	}

void game_controller::start_wesnothd()
{
	typedef std::vector<std::string> path_store;
	// add all paths to try to list
	path_store paths_to_try;

	if (!preferences::get_mp_server_program_name().empty())
		paths_to_try.push_back(preferences::get_mp_server_program_name());

	std::string wesnothd_quess = game_config::wesnothd_name;

	paths_to_try.push_back(wesnothd_quess);


	std::string needle = "wesnothd";
	size_t found = wesnothd_quess.rfind(needle);
	if (found != std::string::npos)
	{
		wesnothd_quess = wesnothd_quess.substr(0, found + needle.size());
#ifdef _WIN32
		wesnothd_quess += ".exe";
#endif
		if (wesnothd_quess != game_config::wesnothd_name)
		{
			paths_to_try.push_back(wesnothd_quess);
		}
	}

	std::string config = get_user_data_dir() + "/lan_server.cfg";
	if (!file_exists(config))
	{
		// copy file if it isn't created yet
		write_file(config, read_file("data/lan_server.cfg"));
	}
	for (path_store::iterator iname = paths_to_try.begin();
			iname != paths_to_try.end(); ++iname)
	{
#ifndef _WIN32
		std::string command = "\"" + *iname +"\" -c " + config + " -d -t 2 -T 5 ";
#else
		// start wesnoth as background job
		std::string command = "cmd /C start \"wesnoth server\" /B \"" + *iname + "\" -c " + config + " -t 2 -T 5 ";
#endif
		LOG_GENERAL << "Starting wesnothd: "<< command << "\n";
		if (std::system(command.c_str()) == 0)
		{
			// Give server a moment to start up
			SDL_Delay(50);
			return;
		}
		preferences::set_mp_server_program_name("");
	}
	// Couldn't start server so throw error
	WRN_GENERAL << "Failed to run server start script\n";
	throw game::mp_server_error("Starting MP server failed!");
}

bool game_controller::play_multiplayer()
{
	int res;

	state_ = game_state();
	state_.campaign_type = "multiplayer";
	state_.campaign_define = "MULTIPLAYER";

	//Print Gui only if the user hasn't specified any server
	if( multiplayer_server_.empty() ){

		int start_server;
		do {
			start_server = 0;

			gui2::tmp_method_selection dlg;

			dlg.show(disp().video());

			if(dlg.get_retval() == gui2::twindow::OK) {
				res = dlg.get_choice();
			} else {
				return false;

			}

			if (res == 2 && preferences::mp_server_warning_disabled() < 2)
			{
				gui::dialog d(disp(), _("Do you really want to start the server?"),
					_("The server will run in a background process until all users have disconnected.")
					, gui::OK_CANCEL);
				bool checked = preferences::mp_server_warning_disabled() != 1;

				d.add_option(_("Don't show again"), checked, gui::dialog::BUTTON_CHECKBOX_LEFT);
				start_server = d.show();
				if (start_server == 0)
					preferences::set_mp_server_warning_disabled(d.option_checked()?2:1);

			}
		} while (start_server);
		if (res < 0) {
			return false;
		}

	}else{
		res = 4;
	}

	try {
		if (res == 2)
		{
			try {
				start_wesnothd();
			} catch(game::mp_server_error&)
			{
				std::string path = preferences::show_wesnothd_server_search(disp());

				if (!path.empty())
				{
					preferences::set_mp_server_program_name(path);
					start_wesnothd();
				}
				else
				{
					throw game::mp_server_error("No path given for mp server prgoram.");
				}
			}


		}

		/* do */ {
			cache_.clear_defines();
			game_config::scoped_preproc_define multiplayer(state_.campaign_define);
			load_game_cfg();
			events::discard(INPUT_MASK); // prevent the "keylogger" effect
			cursor::set(cursor::NORMAL);
			// update binary paths
			paths_manager_.set_paths(game_config_);
			clear_binary_paths_cache();
		}

		if(res == 3) {
			std::vector<std::string> chat;
			config game_data;

			const mp::controller cntr = mp::CNTR_LOCAL;
			const bool is_server = false;

			mp::start_server(disp(), game_config_, cntr, is_server);

		} else if((res >= 0 && res <= 2) || res == 4) {
			std::string host;
			if(res == 0) {
				host = preferences::server_list().front().address;
			}else if(res == 2) {
				host = "localhost";
			}else if(res == 4){
				host = multiplayer_server_;
				multiplayer_server_ = "";
			}
			mp::start_client(disp(), game_config_, host);
		}

	} catch(game::mp_server_error& e) {
		gui::show_error_message(disp(), _("Error while starting server: ") + e.message);
	} catch(game::load_game_failed& e) {
		gui::show_error_message(disp(), _("The game could not be loaded: ") + e.message);
	} catch(game::game_error& e) {
		gui::show_error_message(disp(), _("Error while playing the game: ") + e.message);
	} catch(network::error& e) {
		if(e.message != "") {
			ERR_NET << "caught network::error: " << e.message << "\n";
			/* GCC-3.3 needs a temp var otherwise compilation fails */
			gui::dialog dlg(disp(),"",e.message,gui::OK_ONLY);
			dlg.show();
		} else {
			ERR_NET << "caught network::error\n";
		}
	} catch(config::error& e) {
		if(e.message != "") {
			ERR_CONFIG << "caught config::error: " << e.message << "\n";
			/* GCC-3.3 needs a temp var otherwise compilation fails */
			gui::dialog dlg2(disp(),"",e.message,gui::OK_ONLY);
			dlg2.show();
		} else {
			ERR_CONFIG << "caught config::error\n";
		}
	} catch(incorrect_map_format_exception& e) {
		gui::show_error_message(disp(), std::string(_("The game map could not be loaded: ")) + e.msg_);
	} catch(game::load_game_exception& e) {
		//this will make it so next time through the title screen loop, this game is loaded
		loaded_game_ = e.game;
		loaded_game_show_replay_ = e.show_replay;
		loaded_game_cancel_orders_ = e.cancel_orders;
	} catch(twml_exception& e) {
		e.show(disp());
	}

	return false;
}

bool game_controller::change_language()
{
	gui2::tlanguage_selection dlg;

	dlg.show(disp().video());

	if(dlg.get_retval() == gui2::twindow::OK) {
		if(!no_gui_) {
			std::string wm_title_string = _("The Battle for Wesnoth");
			wm_title_string += " - " + game_config::revision;
			SDL_WM_SetCaption(wm_title_string.c_str(), NULL);
		}

		reset_translations();
	}

	return true;
}

void game_controller::show_preferences()
{
	const preferences::display_manager disp_manager(&disp());
	preferences::show_preferences_dialog(disp(),game_config_);

	disp().redraw_everything();
}

void game_controller::show_upload_begging()
{
	upload_log_dialog::show_beg_dialog(disp());

	disp().redraw_everything();
}


void game_controller::set_unit_data(){
    const config* const units = game_config_.child("units");
    if(units != NULL) {
        unit_type_data::types().set_config(*units);
    }
}

void game_controller::reset_translations()
{
	cursor::setter cur(cursor::WAIT);

	try {
		game_config_.reset_translation();
		// we may have translatable strings in [game_config]
		// e.g. team color names are defined there
		game_config::load_config(game_config_.child("game_config"));
	} catch(game::error& e) {
		ERR_CONFIG << "Error loading game configuration files\n";
		gui::show_error_message(disp(), _("Error loading game configuration files: '") +
			font::nullify_markup(e.message) + _("' (The game will now exit)"));
		throw;
	}

}

void game_controller::load_game_cfg()
{
	gui::set_background_dirty();
	if (!game_config_.empty()
			&& old_defines_map_ == cache_.get_preproc_map())
		return; // game_config already holds requested config in memory
	old_defines_map_ = cache_.get_preproc_map();
	loadscreen::global_loadscreen_manager loadscreen_manager(disp().video());
	cursor::setter cur(cursor::WAIT);
	// The loadscreen will erase the titlescreen
	// NOTE: even without loadscreen, needed after MP lobby
	try {
		/**
		 * Read all game configs
		 * First we should load data/
		 * Then handle terrains so that they are last loaded from data/
		 * 2nd everything in userdata
		 **/
		//reset the parse counter before reading the game files
		data_tree_checksum();
		loadscreen::global_loadscreen->parser_counter = 0;

		// start transaction so macros are shared
		game_config::config_cache_transaction main_transaction;

		cache_.get_config(game_config::path +"/data", game_config_);

		main_transaction.lock();

		// clone and put the gfx rules aside so that we can prepend the add-on
		// rules to them.
		config core_terrain_rules;
		// FIXME: there should be a canned algorithm for cloning child_list objects,
		// along with the memory their elements point to... little implementation detail.
		foreach(config const* p_cfg, game_config_.get_children("terrain_graphics")) {
			core_terrain_rules.add_child("terrain_graphics", *p_cfg);
		}
		game_config_.clear_children("terrain_graphics");

		// load usermade add-ons
		const std::string user_campaign_dir = get_addon_campaigns_dir();
		std::vector< std::string > error_addons;
		// Scan addon directories
		std::vector<std::string> user_dirs;
		// Scan for standalone files
		std::vector<std::string> user_files;

		// The addons that we'll actually load
		std::vector<std::string> addons_to_load;

		get_files_in_dir(user_campaign_dir,&user_files,&user_dirs,ENTIRE_FILE_PATH);
		std::string user_error_log;

		// Append the $user_campaign_dir/*.cfg files to addons_to_load.
		for(std::vector<std::string>::const_iterator uc = user_files.begin(); uc != user_files.end(); ++uc) {
			const std::string file = *uc;
			if(file.substr(file.size() - 4, file.size()) == ".cfg")
				addons_to_load.push_back(file);
		}

		// Append the $user_campaign_dir/*/_main.cfg files to addons_to_load.
		for(std::vector<std::string>::const_iterator uc = user_dirs.begin(); uc != user_dirs.end(); ++uc){
			const std::string main_cfg = *uc + "/_main.cfg";
			if (file_exists(main_cfg))
				addons_to_load.push_back(main_cfg);
		}

		// Load the addons
		for(std::vector<std::string>::const_iterator uc = addons_to_load.begin(); uc != addons_to_load.end(); ++uc) {
			const std::string toplevel = *uc;
			try {
				config umc_cfg;
				cache_.get_config(toplevel, umc_cfg);

				game_config_.append(umc_cfg);
			} catch(config::error& err) {
				ERR_CONFIG << "error reading usermade add-on '" << *uc << "'\n";
				error_addons.push_back(*uc);
				user_error_log += err.message + "\n";
			} catch(preproc_config::error& err) {
				ERR_CONFIG << "error reading usermade add-on '" << *uc << "'\n";
				error_addons.push_back(*uc);
				user_error_log += err.message + "\n";
			} catch(io_exception&) {
				ERR_CONFIG << "error reading usermade add-on '" << *uc << "'\n";
				error_addons.push_back(*uc);
			}
			if(error_addons.empty() == false) {
				std::stringstream msg;
				msg << _n("The following add-on had errors and could not be loaded:",
						"The following add-ons had errors and could not be loaded:",
						error_addons.size());
				for(std::vector<std::string>::const_iterator i = error_addons.begin(); i != error_addons.end(); ++i) {
					msg << "\n" << *i;
				}

				msg << "\n" << _("ERROR DETAILS:") << "\n" << font::nullify_markup(user_error_log);

				gui::show_error_message(disp(),msg.str());
			}
		}

		game_config_.merge_children("units");
		game_config_.append(core_terrain_rules);

		config& hashes = game_config_.add_child("multiplayer_hashes");
		for(config::child_list::const_iterator ch = game_config_.get_children("multiplayer").begin(); ch != game_config_.get_children("multiplayer").end(); ++ch) {
			hashes[(**ch)["id"]] = (*ch)->hash();
		}

		set_unit_data();

	} catch(game::error& e) {
		ERR_CONFIG << "Error loading game configuration files\n";
		gui::show_error_message(disp(), _("Error loading game configuration files: '") +
			font::nullify_markup(e.message) + _("' (The game will now exit)"));
		throw;
	}
}


void game_controller::launch_game(RELOAD_GAME_DATA reload)
{
	loadscreen::global_loadscreen_manager loadscreen_manager(disp().video());
	loadscreen::global_loadscreen->set_progress(0, _("Loading data files"));
	if(reload == RELOAD_DATA) {
		game_config::scoped_preproc_define campaign_define(state_.campaign_define, state_.campaign_define.empty() == false);

		typedef boost::shared_ptr<game_config::scoped_preproc_define> define_ptr;
		std::deque<define_ptr> extra_defines;
		for(std::vector<std::string>::const_iterator i = state_.campaign_xtra_defines.begin(); i != state_.campaign_xtra_defines.end(); ++i) {
			define_ptr newdefine(new game_config::scoped_preproc_define(*i));
			extra_defines.push_back(newdefine);
		}
		try {
			load_game_cfg();
		} catch(config::error&) {
			cache_.clear_defines();
			load_game_cfg();
			return;
		}
	}

	loadscreen::global_loadscreen->set_progress(60);

	const binary_paths_manager bin_paths_manager(game_config_);

	try {
		// Only record log for single-player games & tutorial.
		upload_log log(state_.campaign_type.empty()
					   || state_.campaign_type == "scenario"
					   || state_.campaign_type == "tutorial");

		const LEVEL_RESULT result = play_game(disp(),state_,game_config_, log);
		// don't show The End for multiplayer scenario
		// change this if MP campaigns are implemented
		if(result == VICTORY && (state_.campaign_type.empty() || state_.campaign_type != "multiplayer")) {
			the_end(disp(), state_.end_text, state_.end_text_duration);
			about::show_about(disp(),state_.campaign);
		}
	} catch(game::load_game_exception& e) {

		//this will make it so next time through the title screen loop, this game is loaded
		loaded_game_ = e.game;
		loaded_game_show_replay_ = e.show_replay;
		loaded_game_cancel_orders_ = e.cancel_orders;

	} catch(twml_exception& e) {
		e.show(disp());
	}
}

} //end anon namespace

void game_controller::play_replay()
{
	const binary_paths_manager bin_paths_manager(game_config_);

	try {
		::play_replay(disp(),state_,game_config_,video_);

	} catch(game::load_game_exception& e) {

		//this will make it so next time through the title screen loop, this game is loaded
		loaded_game_ = e.game;
		loaded_game_show_replay_ = e.show_replay;
		loaded_game_cancel_orders_ = e.cancel_orders;

	} catch(twml_exception& e) {
		e.show(disp());
	}
}

#ifndef DISABLE_EDITOR2
editor2::EXIT_STATUS game_controller::start_editor(const std::string& filename)
{
    cache_.clear_defines();
    cache_.add_define("EDITOR");
    cache_.add_define("EDITOR2");
	load_game_cfg();
    const binary_paths_manager bin_paths_manager(game_config_);
	::init_textdomains(game_config_);
	return editor2::start(game_config_, video_, filename);
}
#endif

game_controller::~game_controller()
{
	delete gui::empty_menu;
	sound::close_sound();
}

// this is needed to allow identical functionality with clean refactoring
// play_game only returns on an error, all returns within play_game can
// be replaced with this
static void safe_exit(int res) {

	LOG_GENERAL << "exiting with code " << res << "\n";
#ifdef OS2 /* required to correctly shutdown SDL on OS/2 */
        SDL_Quit();
#endif
	exit(res);
}

// maybe this should go in a util file somewhere?
static void gzip_codec(const std::string & input_file, const std::string & output_file, bool encode)
{
	try {
	std::ofstream ofile(output_file.c_str(), std::ios_base::out
			| std::ios_base::binary | std::ios_base::binary);
			std::ifstream ifile(input_file.c_str(),
			std::ios_base::in | std::ios_base::binary);
		boost::iostreams::filtering_streambuf<boost::iostreams::input> in;
		if(encode)
			in.push(boost::iostreams::gzip_compressor());
		else
			in.push(boost::iostreams::gzip_decompressor());
		in.push(ifile);
		boost::iostreams::copy(in, ofile);
			ifile.close();
		safe_exit(remove(input_file.c_str()));
		}  catch(io_exception& e) {
		std::cerr << "IO error: " << e.what() << "\n";
	}
}

static void gzip_encode(const std::string & input_file, const std::string & output_file)
{
	gzip_codec(input_file, output_file, true);
}

static void gzip_decode(const std::string & input_file, const std::string & output_file)
{
	gzip_codec(input_file, output_file, false);
}


/** Process commandline-arguments */
static int process_command_args(int argc, char** argv) {
	//parse arguments that shouldn't require a display device
	int arg;
	for(arg = 1; arg != argc; ++arg) {
		const std::string val(argv[arg]);
		if(val.empty()) {
			continue;
		}

		if(val == "--help" || val == "-h") {
			// When adding items don't forget to update doc/man/wesnoth.6
			// Options are sorted alphabetically by --long-option.
			// Please keep the output to 80 chars per line.
			std::cout << "usage: " << argv[0]
			<< " [<options>] [<data-directory>]\n"
			<< "Available options:\n"
			<< "  --bpp <number>               sets BitsPerPixel value. Example: --bpp 32\n"
			<< "  -c, --campaign               goes directly to the campaign selection menu.\n"
			<< "  --config-dir <name>          sets the path of the user config directory to\n"
			<< "                               $HOME/<name> or My Documents\\My Games\\<name> for windows.\n"
			<< "  --config-path                prints the path of the user config directory and\n"
			<< "                               exits.\n"
			<< "  -d, --debug                  enables additional command mode options in-game.\n"
#ifdef DEBUG_WINDOW_LAYOUT_GRAPHS
			<< "  --debug-dot-level=<level1>,<level2>,...\n"
			<< "                               sets the level of the debug dot files.\n"
			<< "                               These files are used for debugging the widgets\n"
			<< "                               especially the for the layout engine. When enabled\n"
			<< "                               the engine will produce dot files which can be\n"
			<< "                               converted to images with the dot tool.\n"
			<< "                               Available levels:\n"
			<< "                               - child : generate the data about the grid cells.\n"
			<< "                               - size  : generate the size info of the widget.\n"
			<< "                               - state : generate the state info of the widget.\n"
			<< "  --debug-dot-domain=<domain1>,<domain2>,...\n"
			<< "                               sets the domain of the debug dot files.\n"
			<< "                               see --debug-dot-level for more info.\n"
			<< "                               Available domains:\n"
			<< "                               show   : generate the data when the dialog is\n"
			<< "                                        about to be shown.\n"
			<< "                               layout : generate the data during the layout\n"
			<< "                                        phase (might result in multiple files. \n"
			<< "                               The data can also be generated when the F12 is\n"
			<< "                               pressed in a dialog.\n"
#endif
			<< "  --dummy-locales              enables dummy locales for switching to non-system\n"
			<< "                               locales.\n"
#ifndef DISABLE_EDITOR2
			<< "  -e, --editor                 starts the in-game map editor directly.\n"
#endif
			<< "  --fps                        displays the number of frames per second the\n"
			<< "                               game is currently running at, in a corner of\n"
			<< "                               the screen.\n"
			<< "  -f, --fullscreen             runs the game in full screen mode.\n"
			<< "  --gunzip <infile>.gz         decompresses a file (<infile>.gz) in gzip format\n"
			<< "                               and stores it without the .gz suffix.\n"
			<< "                               <infile>.gz will be removed.\n"
			<< "  --gzip <infile>              compresses a file (<infile>) in gzip format,\n"
			<< "                               stores it as <infile>.gz and removes <infile>.\n"
			<< "  -h, --help                   prints this message and exits.\n"
			<< "  -l, --load <file>            loads the save <file> from the standard save\n"
			<< "                               game directory.\n"
#ifndef DISABLE_EDITOR2
			<< "                               When launching the map editor via -e, the map\n"
			<< "                               <file> is loaded, relative to the current\n"
			<< "                               directory.\n"
#endif
			<< "  --log-<level>=<domain1>,<domain2>,...\n"
			<< "                               sets the severity level of the log domains.\n"
			<< "                               'all' can be used to match any log domain.\n"
			<< "                               Available levels: error, warning, info, debug.\n"
			<< "                               By default the 'error' level is used.\n"
			<< "  --logdomains                 lists defined log domains and exits.\n"
			<< "  --max-fps                    the maximum fps the game tries to run at. Values\n"
			<< "                               should be between 1 and 1000, the default is 50.\n"
			<< "  -m, --multiplayer            starts a multiplayer game. There are additional\n"
			<< "                               options that can be used as explained below:\n"
			<< "    --algorithm<number>=value  selects a non-standard algorithm to be used by\n"
			<< "                               the AI controller for this side.\n"
			<< "    --controller<number>=value selects the controller for this side.\n"
			<< "    --era=value                selects the era to be played in by its id.\n"
			<< "    --exit-at-end              exit Wesnoth at the end of the scenario.\n"
			<< "    --nogui                    runs the game without the GUI. Must appear before\n"
			<< "                               --multiplayer to have the desired effect.\n"
			<< "    --parm<number>=name:value  sets additional parameters for this side.\n"
			<< "    --scenario=value           selects a multiplayer scenario. The default\n"
			<< "                               scenario is \"multiplayer_The_Freelands\".\n"
			<< "    --side<number>=value       selects a faction of the current era for this\n"
			<< "                               side by id.\n"
			<< "    --turns=value              sets the number of turns. The default is \"50\".\n"
			<< "  --no-delay                   runs the game without any delays.\n"
			<< "  --nocache                    disables caching of game data.\n"
			<< "  --nomusic                    runs the game without music.\n"
			<< "  --nosound                    runs the game without sounds and music.\n"
			<< "  --path                       prints the path to the data directory and exits.\n"
#ifdef HAVE_PYTHON
			<< "  --python-api                 prints the runtime documentation for the python\n"
			<< "                               API.\n"
			<< "  --python-shell               invokes wesnoth's embedded python interpreter in\n"
			<< "                               interactive mode.\n"
#endif
			<< "  -r, --resolution XxY         sets the screen resolution. Example: -r 800x600\n"
			<< "  --smallgui                   allows to use screen resolutions down to 800x480\n"
			<< "                               and resizes a few interface elements.\n"
			<< "  -s, --server [<host>]        connects to the host if specified\n"
			<< "                               or to the first host in your preferences.\n"
			<< "  -t, --test                   runs the game in a small test scenario.\n"
			<< "  --validcache                 assumes that the cache is valid. (dangerous)\n"
			<< "  -v, --version                prints the game's version number and exits.\n"
			<< "  -w, --windowed               runs the game in windowed mode.\n"
			<< "  --with-replay                replays the file loaded with the --load option.\n"
			<< "\n"
			<< "  --new-widgets                there is a new WIP widget toolkit this switch\n"
			<< "                               enables the new toolkit (VERY EXPERIMENTAL don't\n"
			<< "                               file bug reports since most are known).\n"
			<< "                               Parts of the library are deemed stable and will\n"
			<< "                               work without this switch.\n"
			;
			return 0;
		} else if(val == "--version" || val == "-v") {
			std::cout << "Battle for Wesnoth" << " " << game_config::version
			          << "\n";
			return 0;
		} else if (val == "--config-path") {
			std::cout << get_user_data_dir() << '\n';
			return 0;
		} else if(val == "--path") {
			std::cout <<  game_config::path
			          << "\n";
			return 0;
#ifdef HAVE_PYTHON
		} else if(val == "--python-api") {
			python_ai::invoke("documentation");
			return 0;
        } else if(val == "--python-shell") {
			int ret = python_ai::run_shell();
			if (ret==0) { 
			    return 0;
		    } else {
		       return 2; 
		    }
#endif
		} else if(val == "--config-dir") {
			if (argc <= ++arg)
				break;
			set_preferences_dir(argv[arg]);
		} else if (val.substr(0, 6) == "--log-") {
			size_t p = val.find('=');
			if (p == std::string::npos) {
				std::cerr << "unknown option: " << val << '\n';
				return 0;
			}
			std::string s = val.substr(6, p - 6);
			int severity;
			if (s == "error") severity = 0;
			else if (s == "warning") severity = 1;
			else if (s == "info") severity = 2;
			else if (s == "debug") severity = 3;
			else {
				std::cerr << "unknown debug level: " << s << '\n';
				return 0;
			}
			while (p != std::string::npos) {
				size_t q = val.find(',', p + 1);
				s = val.substr(p + 1, q == std::string::npos ? q : q - (p + 1));
				if (!lg::set_log_domain_severity(s, severity)) {
					std::cerr << "unknown debug domain: " << s << '\n';
					return 0;
				}
				p = q;
			}
		} else if(val == "--gzip") {
			if(argc != arg + 2) {
				std::cerr << "format of " << val << " command: " << val << " <input file>\n";
				return 2;
			}

			const std::string input_file(argv[arg + 1]);
			const std::string output_file(input_file + ".gz");
			gzip_encode(input_file, output_file);

		} else if(val == "--gunzip") {
			if(argc != arg + 2) {
				std::cerr << "format of " << val << " command: " << val << " <input file>\n";
				return 2;
			}

			const std::string input_file(argv[arg + 1]);
			if(! is_gzip_file(input_file)) {

				std::cerr << "file '" << input_file << "'isn't a .gz file\n";
				return 2;
			}
			const std::string output_file(
				input_file, 0, input_file.length() - 3);

			gzip_decode(input_file, output_file);

		} else if(val == "--logdomains") {
			std::cout << lg::list_logdomains() << "\n";
			return 0;
		}
	}

	// Not the most intuitive solution, but I wanted to leave current semantics for now
	return -1;
}

/**
 * I would prefer to setup locale first so that early error
 * messages can get localized, but we need the game_controller
 * initialized to have get_intl_dir() to work.  Note: setlocale()
 * does not take GUI language setting into account.
 */
static void init_locale() {
	#ifdef _WIN32
	    std::setlocale(LC_ALL, "English");
	#else
		std::setlocale(LC_ALL, "C");
		std::setlocale(LC_MESSAGES, "");
	#endif
	const std::string& intl_dir = get_intl_dir();
	bindtextdomain (PACKAGE, intl_dir.c_str());
	bind_textdomain_codeset (PACKAGE, "UTF-8");
	bindtextdomain (PACKAGE "-lib", intl_dir.c_str());
	bind_textdomain_codeset (PACKAGE "-lib", "UTF-8");
	textdomain (PACKAGE);
}

/**
 * Setups the game environment and enters
 * the titlescreen or game loops.
 */
static int do_gameloop(int argc, char** argv)
{
	int finished = process_command_args(argc, argv);
	if(finished != -1) {
		return finished;
	}

	srand(time(NULL));

	game_controller game(argc,argv);
	const int start_ticks = SDL_GetTicks();

	init_locale();

	bool res;

	// do initialize fonts before reading the game config, to have game
	// config error messages displayed. fonts will be re-initialized later
	// when the language is read from the game config.
	res = font::load_font_config();
	if(res == false) {
		std::cerr << "could not initialize fonts\n";
		return 0;
	}

	res = game.init_video();
	if(res == false) {
		std::cerr << "could not initialize display\n";
		return 0;
	}

	const cursor::manager cursor_manager;
	cursor::set(cursor::WAIT);

	gui2::init();

	loadscreen::global_loadscreen_manager loadscreen_manager(game.disp().video());

	res = game.init_language();
	if(res == false) {
		std::cerr << "could not initialize the language\n";
		return 0;
	}

	loadscreen::global_loadscreen->increment_progress(5, _("Loading game configuration."));
	res = game.init_config();
	if(res == false) {
		std::cerr << "could not initialize game config\n";
		return 0;
	}
	loadscreen::global_loadscreen->increment_progress(10, _("Re-initialize fonts for the current language."));

	res = font::load_font_config();
	if(res == false) {
		std::cerr << "could not re-initialize fonts for the current language\n";
		return 0;
	}

	loadscreen::global_loadscreen->increment_progress(0, _("Searching for installed add-ons."));
	refresh_addon_version_info_cache();

#if defined(_X11) && !defined(__APPLE__)
	SDL_EventState(SDL_SYSWMEVENT, SDL_ENABLE);
#endif

	config tips_of_day;

	loadscreen::global_loadscreen->set_progress(100, _("Loading title screen."));

	LOG_CONFIG << "time elapsed: "<<  (SDL_GetTicks() - start_ticks) << " ms\n";

	for(;;){

		// reset the TC, since a game can modify it, and it may be used
		// by images in add-ons or campaigns dialogs
		image::set_team_colors();

		statistics::fresh_stats();

		sound::play_music_repeatedly(game_config::title_music);

		if(game.play_test() == false) {
			return 0;
		}

		if(game.play_multiplayer_mode() == false) {
			return 0;
		}

		recorder.clear();

		//Start directly a campaign
		if(game.goto_campaign() == false){
			continue; //Go to main menu
		}

		//Start directly a multiplayer
		//Eventually with a specified server
		if(game.goto_multiplayer() == false){
			continue; //Go to main menu
		}
#ifndef DISABLE_EDITOR2
		if (game.goto_editor() == false) {
			return 0;
		}
#endif
		loadscreen_manager.reset();

		gui::TITLE_RESULT res = game.is_loading() ? gui::LOAD_GAME : gui::NOTHING;

		if(gui2::new_widgets) {
			const preferences::display_manager disp_manager(&game.disp());
			const hotkey::basic_handler key_handler(&game.disp());

			const font::floating_label_context label_manager;

			cursor::set(cursor::NORMAL);
			gui2::ttitle_screen dlg;
			dlg.show(game.disp().video());

			res = static_cast<gui::TITLE_RESULT>(dlg.get_retval());

		} else {
			while(res == gui::NOTHING) {
				res = gui::show_title(game.disp(),tips_of_day);
				if (res == gui::REDRAW_BACKGROUND) {
					gui::set_background_dirty();
					res = gui::NOTHING;
				}
			}
		}

		game_controller::RELOAD_GAME_DATA should_reload = game_controller::RELOAD_DATA;

		if(res == gui::QUIT_GAME) {
			LOG_GENERAL << "quitting game...\n";
			return 0;
		} else if(res == gui::LOAD_GAME) {
			if(game.load_game() == false) {
				continue;
			}

			should_reload = game_controller::NO_RELOAD_DATA;
		} else if(res == gui::TUTORIAL) {
			game.set_tutorial();
		} else if(res == gui::NEW_CAMPAIGN) {
			if(game.new_campaign() == false) {
				continue;
			}
		} else if(res == gui::MULTIPLAYER) {
			if(game.play_multiplayer() == false) {
				continue;
			}
		} else if(res == gui::CHANGE_LANGUAGE) {
			if(game.change_language() == true) {
				tips_of_day.clear();
			}
			continue;
		} else if(res == gui::EDIT_PREFERENCES) {
			game.show_preferences();
			if (game.disp().video().modeChanged()) {
				gui::set_background_dirty();
			}
			continue;
		} else if(res == gui::SHOW_ABOUT) {
			about::show_about(game.disp());
			continue;
		} else if(res == gui::SHOW_HELP) {
			help::help_manager help_manager(&game.game_config(), NULL);
			help::show_help(game.disp());
			continue;
		} else if(res == gui::GET_ADDONS) {
			try {
				manage_addons(game.disp());
			} catch(config_changed_exception const&) {
				game.reload_changed_game_config();
			}
			continue;
		} else if(res == gui::BEG_FOR_UPLOAD) {
			game.show_upload_begging();
			continue;
		} else if(res == gui::RELOAD_GAME_DATA) {
			loadscreen::global_loadscreen_manager loadscreen(game.disp().video());
			game.reload_changed_game_config();
			continue;
#ifndef DISABLE_EDITOR2
		} else if(res == gui::START_MAP_EDITOR) {
			//@todo editor can ask the game to quit completely
			if (game.start_editor() == editor2::EXIT_QUIT_TO_DESKTOP) {
				return 0;
			} else {
				gui::set_background_dirty();
			}
			continue;
#endif
		}

		if (recorder.at_end()){
			game.launch_game(should_reload);
		}
		else{
			game.play_replay();
		}
	}

	return 0;
}

#ifndef DISABLE_POOL_ALLOC
extern "C" {
void init_custom_malloc();
}
#endif

int main(int argc, char** argv)
{
#ifndef DISABLE_POOL_ALLOC
	init_custom_malloc();
#endif
	if(SDL_Init(SDL_INIT_TIMER) < 0) {
		fprintf(stderr, "Couldn't initialize SDL: %s\n", SDL_GetError());
		return(1);
	}

	try {
		std::cerr << "Battle for Wesnoth v" << game_config::revision << '\n';
		const time_t t = time(NULL);
		std::cerr << "Started on " << ctime(&t) << "\n";

		/**
		 * @todo We try to guess the name of the server from the name of the
		 * binary started. This is very fragile it breaks in at least the
		 * following cases.
		 * - Wesnoth got renamed to something without wesnoth in it.
		 * - Wesnoth got a pre/suffix but the server not.
		 */
		std::string program = argv[0];
		const std::string wesnoth = "wesnoth";
		const size_t offset = program.rfind(wesnoth);
		if(offset != std::string::npos) {
			program.replace(offset, wesnoth.length(), "wesnothd");
			game_config::wesnothd_name = program;
		} else {
			std::cerr << "Game executable doesn't have the name \"" << wesnoth
			          << "\" or similar; impossible to guess the server executable name.\n";
		}

		const std::string exe_dir = get_exe_dir();
		if(!exe_dir.empty() && file_exists(exe_dir + "/data/_main.cfg")) {
			std::cerr << "Automatically found a possible data directory at "
			          << exe_dir << '\n';
			game_config::path = exe_dir;
		}

		const int res = do_gameloop(argc,argv);
		safe_exit(res);
	} catch(CVideo::error&) {
		std::cerr << "Could not initialize video. Exiting.\n";
	} catch(font::manager::error&) {
		std::cerr << "Could not initialize fonts. Exiting.\n";
	} catch(config::error& e) {
		std::cerr << e.message << "\n";
	} catch(gui::button::error&) {
		std::cerr << "Could not create button: Image could not be found\n";
	} catch(CVideo::quit&) {
		//just means the game should quit
	} catch(end_level_exception&) {
		std::cerr << "caught end_level_exception (quitting)\n";
	} catch(std::bad_alloc&) {
		std::cerr << "Ran out of memory. Aborted.\n";
	} catch(twml_exception& e) {
		std::cerr << "WML exception:\nUser message: "
			<< e.user_message << "\nDev message: " << e.dev_message << '\n';
	} catch(game_logic::formula_error& e) {
		std::cerr << "Formula error found in " << e.filename << ":" << e.line
			<< "\nIn formula " << e.formula
			<< "\nError: " << e.type
			<< "\n\nGame will be aborted.\n";
	}

	return 0;
} // end main

