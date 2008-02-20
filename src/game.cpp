/* $Id$ */
/*
   Copyright (C) 2003 by David White <davidnwhite@verizon.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "global.hpp"

#include "SDL.h"

#include "about.hpp"
#include "config.hpp"
#include "cursor.hpp"
#include "dialogs.hpp"
#include "display.hpp"
#include "filesystem.hpp"
#include "font.hpp"
#include "game_config.hpp"
#include "game_errors.hpp"
#include "gamestatus.hpp"
#include "gettext.hpp"
#include "gp2x.hpp"
#include "help.hpp"
#include "hotkeys.hpp"
#include "intro.hpp"
#include "language.hpp"
#include "loadscreen.hpp"
#include "playcampaign.hpp"
#include "preferences_display.hpp"
#include "replay.hpp"
#include "sound.hpp"
#include "statistics.hpp"
#include "thread.hpp"
#include "titlescreen.hpp"
#include "util.hpp"
#include "wassert.hpp"
#include "wml_separators.hpp"
#include "serialization/binary_or_text.hpp"
#include "serialization/binary_wml.hpp"
#include "serialization/parser.hpp"
#include "serialization/preprocessor.hpp"

#include "wesconfig.h"

#include <cmath>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>
#include <iterator>
#include <sstream>
#include <string>

namespace {


bool less_campaigns_rank(const config* a, const config* b) {
	return lexical_cast_default<int>((*a)["rank"],1000) <
	       lexical_cast_default<int>((*b)["rank"],1000);
}

class game_controller
{
public:
	game_controller(int argc, char** argv);

	display& disp();

	bool init_video();
	bool init_config();
	bool init_language();
	bool play_test();

	void reset_game_cfg();

	bool is_loading() const;
	bool load_game();
	void set_tutorial();

	bool new_campaign();
	bool change_language();

	void show_help();
	void show_preferences();

	enum RELOAD_GAME_DATA { RELOAD_DATA, NO_RELOAD_DATA };
	void play_game(RELOAD_GAME_DATA reload=RELOAD_DATA);
	void play_replay();
  const config& game_config(){return game_config_;};
  const game_data& units_data(){return units_data_;};

private:
	game_controller(const game_controller&);
	void operator=(const game_controller&);

	void read_game_cfg(const preproc_map& defines, config& cfg, bool use_cache);
	void refresh_game_cfg(bool reset_translations=false);

	const int argc_;
	int arg_;
	const char* const * const argv_;

	//this should get destroyed *after* the video, since we want to clean up threads
	//after the display disappears.
	const threading::manager thread_manager;

	CVideo video_;

	const font::manager font_manager_;
	const preferences::manager prefs_manager_;
	const sound::manager sound_manager_;
	const image::manager image_manager_;
	const events::event_context main_event_context_;
	const hotkey::manager hotkey_manager_;
	binary_paths_manager paths_manager_;

	bool test_mode_, no_gui_;
	bool use_caching_;
	bool force_valid_cache_;
	int force_bpp_;

	config game_config_;
	game_data units_data_;

	util::scoped_ptr<display> disp_;

	game_state state_;

	std::string loaded_game_;
	bool loaded_game_show_replay_;

	preproc_map defines_map_, old_defines_map_;
};

game_controller::game_controller(int argc, char** argv)
   : argc_(argc), arg_(1), argv_(argv), thread_manager(),
     test_mode_(false), 
     no_gui_(false), use_caching_(true), force_valid_cache_(true),
     force_bpp_(-1), disp_(NULL), loaded_game_show_replay_(false)
{
	for(arg_ = 1; arg_ != argc_; ++arg_) {
		const std::string val(argv_[arg_]);
		if(val.empty()) {
			continue;
		}

		if(val == "--fps") {
			preferences::set_show_fps(true);
		} else if(val == "--nocache") {
			use_caching_ = false;
		} else if(val == "--validcache") {
			force_valid_cache_ = "true";
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
		} else if(val == "--load") {
			if(arg_+1 != argc_) {
				++arg_;
				loaded_game_ = argv_[arg_];
			}
		} else if(val == "--nogui") {
			no_gui_ = true;
		} else if(val == "--windowed" || val == "-w") {
			preferences::set_fullscreen(false);
		} else if(val == "--fullscreen" || val == "-f") {
			preferences::set_fullscreen(true);
		} else if(val == "--test" || val == "-t") {
			test_mode_ = true;
		} else if(val == "--debug" || val == "-d") {
			game_config::debug = true;
			game_config::mp_debug = true;
		} else if(val == "--no-delay") {
			game_config::no_delay = true;
		} else if (val.substr(0, 6) == "--log-") {
		} else if(val == "--nosound") {
			preferences::set_sound(false);
			preferences::set_music(false);
		} else if(val[0] == '-') {
			std::cerr << "unknown option: " << val << std::endl;
			throw config::error("unknown option");
		} else {

		  std::cerr << "Setting path using " << val << std::endl;
			if(val[0] == '/') {
				game_config::path = val;
			} else {
				game_config::path = get_cwd() + '/' + val;
			}

			if(!is_directory(game_config::path)) {
				std::cerr << "Could not find directory '" << game_config::path << "'\n";
				throw config::error("directory not found");
			}

		}
	}

	if (preferences::sound_on() || preferences::music_on()) {
		if(!sound::init_sound()) {
			preferences::set_sound(false);
			preferences::set_music(false);
		}
	}
}

display& game_controller::disp()
{
	if(disp_.get() == NULL) {

		if(get_video_surface() == NULL) {
			throw CVideo::error();
		}

		static unit_map dummy_umap;
		static config dummy_cfg;
		static gamemap dummy_map(dummy_cfg, "1");
		static gamestatus dummy_status(dummy_cfg, 0);
		static std::vector<team> dummy_teams;
		disp_.assign(new display(dummy_umap, video_, dummy_map, dummy_status,
			dummy_teams, dummy_cfg, dummy_cfg, dummy_cfg));
	}

	return *disp_.get();
}

bool game_controller::init_video()
{
	image::set_wm_icon();

	int video_flags = preferences::fullscreen() ? FULL_SCREEN : 0;

	std::pair<int,int> resolution = preferences::resolution();

	int DefaultBPP = 24;
	const SDL_VideoInfo* const video_info = SDL_GetVideoInfo();
	if(video_info != NULL && video_info->vfmt != NULL) {
		DefaultBPP = video_info->vfmt->BitsPerPixel;
	}

	std::cerr << "Checking video mode: " << resolution.first
		  << "x" << resolution.second << "x" << DefaultBPP << "...\n";
	int bpp = video_.modePossible(resolution.first,resolution.second,DefaultBPP,video_flags);

	std::cerr << bpp << "\n";

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

		if(bpp == 0) {
			//couldn't do 1024x768 or 800x600

			std::cerr << "The required video mode, " << resolution.first
			          << "x" << resolution.second << "x" << DefaultBPP << " "
			          << "is not supported\n";

			if((video_flags&FULL_SCREEN) != 0) {
				std::cerr << "Try running the program with the --windowed option "
				          << "using a " << DefaultBPP << "bpp X windows setting\n";
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

	std::cerr << "setting mode to " << resolution.first << "x" << resolution.second << "x" << bpp << "\n";
	const int res = video_.setMode(resolution.first,resolution.second,bpp,video_flags);
	video_.setBpp(bpp);
	if(res == 0) {
		std::cerr << "required video mode, " << resolution.first << "x"
		          << resolution.second << "x" << bpp << " is not supported\n";
		return false;
	}

	cursor::set(cursor::NORMAL);

#ifdef GP2X
	gp2x::mmu_hack();
#endif

	return true;
}

bool game_controller::init_config()
{
	units_data_.clear();
	//Resets old_defines_map_, to force refresh_game_cfg to reload
	//everything.
	old_defines_map_.clear();

	reset_game_cfg();

	game_config::load_config(game_config_.child("game_config"));

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
		SDL_WM_SetCaption(_("The Battle for Wesnoth"), NULL);
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
	state_.scenario = "test";

	try {
		::play_game(disp(),state_,game_config_,units_data_,video_);
	} catch(game::load_game_exception& e) {
		loaded_game_ = e.game;
		loaded_game_show_replay_ = e.show_replay;

		return true;
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

	const std::string game = loaded_game_.empty() ? dialogs::load_game_dialog(disp(),game_config_,units_data_,&show_replay) : loaded_game_;

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

		defines_map_.clear();
		defines_map_[cfg["difficulty"]] = preproc_define();

		if(defines_map_.count("NORMAL")) {
			defines_map_["MEDIUM"] = preproc_define();
		}

		const std::string& campaign_define = cfg["campaign_define"];
		if(campaign_define.empty() == false) {
			defines_map_[campaign_define] = preproc_define();
		}

		refresh_game_cfg();

		state_ = read_game(units_data_,&cfg);

		if(state_.version != game_config::version) {
			const int res = gui::show_dialog(disp(),NULL,"",
			                      _("This save is from a different version of the game. Do you want to try to load it?"),
			                      gui::YES_NO);
			if(res == 1) {
				return false;
			}
		}

	} catch(game::error& e) {
		gui::show_error_message(disp(), _("The file you have tried to load is corrupt: '") + e.message + '\'');
		return false;
	} catch(config::error& e) {
		gui::show_error_message(disp(), _("The file you have tried to load is corrupt: '") + e.message + '\'');
		return false;
	} catch(io_exception&) {
		gui::show_error_message(disp(), _("File I/O Error while reading the game"));
		return false;
	}
	recorder = replay(state_.replay_data);
	recorder.start_replay();
	recorder.set_skip(false);

	std::cerr << "has snapshot: " << (state_.snapshot.child("side") ? "yes" : "no") << "\n";

	if(state_.snapshot.child("side") == NULL) {
		// No snapshot; this is a start-of-scenario
		if (show_replay) {
			// There won't be any turns to replay, but the
			// user gets to watch the intro sequence again ...
			std::cerr << "replaying (start of scenario)\n";
		} else {
			std::cerr << "skipping...\n";
			recorder.set_skip(false);
		}
	} else {
		// We have a snapshot. But does the user want to see a replay?
		if(show_replay) {
			statistics::clear_current_scenario();
			std::cerr << "replaying (snapshot)\n";
		} else {
			std::cerr << "setting replay to end...\n";
			recorder.set_to_end();
			if(!recorder.at_end()) {
				std::cerr << "recorder is not at the end!!!\n";
			}
		}
	}

	if(state_.campaign_type == "tutorial") {
		defines_map_["TUTORIAL"] = preproc_define();
	}

	return true;
}

void game_controller::set_tutorial()
{
	state_ = game_state();
	state_.campaign_type = "tutorial";
	state_.scenario = "tutorial";
	state_.campaign_define = "TUTORIAL";
	defines_map_.clear();
	defines_map_["TUTORIAL"] = preproc_define();
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

	int res = 0;

	dialogs::campaign_preview_pane campaign_preview(disp().video(),&campaign_desc);
	std::vector<gui::preview_pane*> preview_panes;
	preview_panes.push_back(&campaign_preview);

	if(campaign_names.size() <= 0) {
		return false;
	}

	res = gui::show_dialog2(disp(),NULL,_("Campaign"),
			_("Choose the campaign you want to play:"),
			gui::OK_CANCEL,&campaign_names,&preview_panes);

	if(res == -1) {
		return false;
	}

	const config& campaign = *campaigns[res];

	state_.campaign = campaign["id"];
	state_.scenario = campaign["first_scenario"];

	const std::string difficulty_descriptions = campaign["difficulty_descriptions"];
	std::vector<std::string> difficulty_options = utils::split(difficulty_descriptions, ';');

	const std::vector<std::string> difficulties = utils::split(campaign["difficulties"]);

	if(difficulties.empty() == false) {
		if(difficulty_options.size() != difficulties.size()) {
			difficulty_options.resize(difficulties.size());
			std::copy(difficulties.begin(),difficulties.end(),difficulty_options.begin());
		}

		const int res = gui::show_dialog2(disp(),NULL,_("Difficulty"),
		                            _("Select difficulty level:"),
		                            gui::OK_CANCEL,&difficulty_options);
		if(res == -1) {
			return false;
		}

		state_.difficulty = difficulties[res];
		defines_map_.clear();
		defines_map_[difficulties[res]] = preproc_define();
	}
#ifdef HAVE_PYTHON
	defines_map_["PYTHON"] = preproc_define();
#endif

	state_.campaign_define = campaign["define"];

	return true;
}

namespace
{

std::string format_file_size(const std::string& size_str)
{
	double size = lexical_cast_default<double>(size_str,0.0);

	const double k = 1024;
	if(size > 0.0) {
		std::string size_postfix = _("B");
		if(size > k) {
			size /= k;
			size_postfix = _("KB");
			if(size > k) {
				size /= k;
				size_postfix = _("MB");
			}
		}

		std::ostringstream stream;
#ifdef _MSC_VER
		//Visual C++ makes 'precision' set the number of decimal places. Other platforms
		//make it set the number of significant figures
		stream.precision(1);
		stream << std::fixed << size << size_postfix;
#else
		if (size < 100) stream.precision(3);
		else size = (int)size;
		stream << size << size_postfix;
#endif
		return stream.str();
	} else {
		return "";
	}
}

}

bool game_controller::change_language()
{
	const std::vector<language_def>& languages = get_languages();
	std::vector<std::string> langs;

	for (std::vector<language_def>::const_iterator itor = languages.begin();
			itor != languages.end(); ++itor) {
		if (*itor == get_language()) {
			langs.push_back("*" + itor->language);
		} else {
			langs.push_back(itor->language);
		}
	}

	const int res = gui::show_dialog2(disp(),NULL,_("Language"),
	                         _("Choose your preferred language:"),
	                         gui::OK_CANCEL,&langs);
	if(size_t(res) < langs.size()) {
		::set_language(languages[res]);
		preferences::set_language(languages[res].localename);

		if(!no_gui_) {
			SDL_WM_SetCaption(_("The Battle for Wesnoth"), NULL);
		}

		refresh_game_cfg(true);
	}

	font::load_font_config();
	hotkey::load_descriptions();

	return false;
}

void game_controller::show_preferences()
{
	const preferences::display_manager disp_manager(&disp());
	preferences::show_preferences_dialog(disp(),game_config_);

	disp().redraw_everything();
}

//this function reads the game configuration, searching for valid cached copies first
void game_controller::read_game_cfg(const preproc_map& defines, config& cfg, bool use_cache)
{
	log_scope("read_game_cfg");

	if(defines.size() < 5) {
		bool is_valid = true;
		std::stringstream str;
		str << "-v" << game_config::version;
		for(preproc_map::const_iterator i = defines.begin(); i != defines.end(); ++i) {
			if(i->second.value != "" || i->second.arguments.empty() == false) {
				is_valid = false;
				break;
			}

			str << "-" << i->first;
		}
		//std::string localename = get_locale().localename;
		//str << "-lang_" << (localename.empty() ? "default" : localename);

		if(is_valid) {
			const std::string& cache = get_cache_dir();
			if(cache != "") {
				const std::string fname = cache + "/game.cfg-cache" + str.str();
				const std::string fname_checksum = fname + ".checksum";

				file_tree_checksum dir_checksum;

				if(use_cache && !force_valid_cache_) {
					try {
						if(file_exists(fname_checksum)) {
							config checksum_cfg;
							scoped_istream stream = istream_file(fname_checksum);
							read(checksum_cfg, *stream);
							dir_checksum = file_tree_checksum(checksum_cfg);
						}
					} catch(config::error&) {
						std::cerr << "cache checksum is corrupt\n";
					} catch(io_exception&) {
						std::cerr << "error reading cache checksum\n";
					}
				}

				if(force_valid_cache_)
					std::cerr << "skipping cache validation (forced)\n";

				if(use_cache && file_exists(fname) && (force_valid_cache_ || file_create_time(fname) > data_tree_checksum().modified && dir_checksum == data_tree_checksum())) {
					std::cerr << "found valid cache at '" << fname << "' using it\n";
					log_scope("read cache");
					try {
						scoped_istream stream = istream_file(fname);
						read_compressed(cfg, *stream);
						return;
					} catch(config::error&) {
						std::cerr << "cache is corrupt. Loading from files\n";
					} catch(io_exception&) {
						std::cerr << "error reading cache. Loading from files\n";
					}
				}

				std::cerr << "no valid cache found. Writing cache to '" << fname << "'\n";

				preproc_map defines_map(defines);

				//read the file and then write to the cache
				scoped_istream stream = preprocess_file("data/game.cfg", &defines_map);

				//reset the parse counter before reading the game files
				if (loadscreen::global_loadscreen) {
					loadscreen::global_loadscreen->parser_counter = 0;
				}

				std::string error_log, user_error_log;

				read(cfg, *stream, &error_log);

				//load user campaigns
				const std::string user_campaign_dir = get_user_data_dir() + "/data/campaigns/";
				std::vector<std::string> user_campaigns, error_campaigns;
				get_files_in_dir(user_campaign_dir,&user_campaigns,NULL,ENTIRE_FILE_PATH);
				for(std::vector<std::string>::const_iterator uc = user_campaigns.begin(); uc != user_campaigns.end(); ++uc) {
					static const std::string extension = ".cfg";
					if(uc->size() < extension.size() || std::equal(uc->end() - extension.size(),uc->end(),extension.begin()) == false) {
						continue;
					}

					try {
						preproc_map user_defines_map(defines_map);
						scoped_istream stream = preprocess_file(*uc,&user_defines_map);

						std::string campaign_error_log;

						config user_campaign_cfg;
						read(user_campaign_cfg,*stream,&campaign_error_log);

						if(campaign_error_log.empty()) {
							cfg.append(user_campaign_cfg);
						} else {
							user_error_log += campaign_error_log;
							error_campaigns.push_back(*uc);
						}
					} catch(config::error& err) {
						std::cerr << "error reading user campaign '" << *uc << "'\n";
						error_campaigns.push_back(*uc);

						user_error_log += err.message + "\n";
					} catch(io_exception&) {
						std::cerr << "error reading user campaign '" << *uc << "'\n";
						error_campaigns.push_back(*uc);
					}
				}

				if(error_campaigns.empty() == false) {
					std::stringstream msg;
					msg << _("The following add-on campaign(s) had errors and could not be loaded:");
					for(std::vector<std::string>::const_iterator i = error_campaigns.begin(); i != error_campaigns.end(); ++i) {
						msg << "\n" << *i;
					}

					msg << "\n" << _("ERROR DETAILS:") << "\n" << user_error_log;

					gui::show_error_message(disp(),msg.str());
				}

				cfg.merge_children("units");

				if(!error_log.empty()) {
					gui::show_error_message(disp(),
							_("Warning: Errors occurred while loading game configuration files: '") +
							error_log);

				} else {
					try {
						scoped_ostream cache = ostream_file(fname);
						write_compressed(*cache, cfg);
						config checksum_cfg;
						data_tree_checksum().write(checksum_cfg);
						scoped_ostream checksum = ostream_file(fname_checksum);
						write(*checksum, checksum_cfg);
					} catch(io_exception&) {
						std::cerr << "could not write to cache '" << fname << "'\n";
					}
				}

				return;
			}
		}
	}

	std::cerr << "caching cannot be done. Reading file\n";
	preproc_map defines_map(defines);
	scoped_istream stream = preprocess_file("data/game.cfg", &defines_map);
	read(cfg, *stream);
}

void game_controller::refresh_game_cfg(bool reset_translations)
{
	try {
		if(old_defines_map_.empty() || defines_map_ != old_defines_map_ || reset_translations) {

			units_data_.clear();

			if(!reset_translations) {
				game_config_.clear();
				read_game_cfg(defines_map_, game_config_, use_caching_);
			} else {
				game_config_.reset_translation();
			}

			const config* const units = game_config_.child("units");
			if(units != NULL) {
				units_data_.set_config(*units);
			}

			old_defines_map_ = defines_map_;
		}
	} catch(config::error& e) {
		std::cerr << "Error loading game configuration files\n";
		gui::show_error_message(disp(), _("Error loading game configuration files: '") +
		                        e.message + _("' (The game will now exit)"));
		throw e;
	}
}

void game_controller::reset_game_cfg()
{
	defines_map_.clear();

	//load in the game's configuration files
#if defined(__APPLE__)
	defines_map_["APPLE"] = preproc_define();
#endif

	defines_map_["NORMAL"] = preproc_define();
	defines_map_["MEDIUM"] = preproc_define();

	refresh_game_cfg();
}

void game_controller::play_game(RELOAD_GAME_DATA reload)
{
	if(reload == RELOAD_DATA) {
		if(state_.campaign_define.empty() == false) {
			defines_map_[state_.campaign_define] = preproc_define();
		}

		if(defines_map_.count("NORMAL")) {
			defines_map_["MEDIUM"] = preproc_define();
		}

		refresh_game_cfg();
	}

	const binary_paths_manager bin_paths_manager(game_config_);

	try {
		const LEVEL_RESULT result = ::play_game(disp(),state_,game_config_,units_data_,video_);
		if(result == VICTORY && state_.campaign_type.empty()) {
			the_end(disp());
			about::show_about(disp(),state_.campaign);
		}
	} catch(game::load_game_exception& e) {

		//this will make it so next time through the title screen loop, this game is loaded
		loaded_game_ = e.game;
		loaded_game_show_replay_ = e.show_replay;
	}
}

} //end anon namespace

void game_controller::play_replay()
{
	const binary_paths_manager bin_paths_manager(game_config_);

	try {
		::play_replay(disp(),state_,game_config_,units_data_,video_);

	} catch(game::load_game_exception& e) {

		//this will make it so next time through the title screen loop, this game is loaded
		loaded_game_ = e.game;
		loaded_game_show_replay_ = e.show_replay;
	}
}

int play_game(int argc, char** argv)
{
	const int start_ticks = SDL_GetTicks();

	//parse arguments that shouldn't require a display device
	int arg;
	for(arg = 1; arg != argc; ++arg) {
		const std::string val(argv[arg]);
		if(val.empty()) {
			continue;
		}

		if(val == "--help" || val == "-h") {
			std::cout << "usage: " << argv[0]
			<< " [OPTIONS] [DATA-DIRECTORY]\n"
			<< "  --bpp number                 sets BitsPerPixel value. Example: --bpp 32\n"
			<< "  --compress INFILE OUTFILE    compresses a savefile (INFILE) that is in text WML\n"
			<< "                               format into binary WML format (OUTFILE).\n"
			<< "  -d, --debug                  shows extra debugging information and enables\n"
			<< "                               additional command mode options in-game.\n"
			<< "  --decompress INFILE OUTFILE  decompresses a savefile (INFILE) that is in binary\n"
			<< "                               WML format into text WML format (OUTFILE).\n"
			<< "  -f, --fullscreen             runs the game in full screen mode.\n"
			<< "  --fps                        displays the number of frames per second the game\n"
			<< "                               is currently running at, in a corner of the screen.\n"
			<< "  -h, --help                   prints this message and exits.\n"
			<< "  --load SAVEGAME              loads the file SAVEGAME from the standard save\n"
			<< "                               game directory.\n"
			<< "  --log-error=\"domain1,domain2,...\", --log-warning=..., --log-info=...\n"
			<< "                               sets the severity level of the debug domains.\n"
			<< "                               \"all\" can be used to match any debug domain.\n"
			<< "  --nocache                    disables caching of game data.\n"
			<< "  --nosound                    runs the game without sounds and music.\n"
			<< "  --path                       prints the name of the game data directory and exits.\n"
			<< "  -r, --resolution XxY         sets the screen resolution. Example: -r 800x600\n"
			<< "  -t, --test                   runs the game in a small test scenario.\n"
			<< "  -v, --version                prints the game's version number and exits.\n"
			<< "  -w, --windowed               runs the game in windowed mode.\n"
			<< "  --no-delay                   run the game without any delays.\n"
			;
			return 0;
		} else if(val == "--version" || val == "-v") {
			std::cout << _("Battle for Wesnoth") << " " << game_config::version
			          << "\n";
			return 0;
		} else if(val == "--path") {
			std::cout <<  game_config::path
			          << "\n";
			return 0;
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
		} else if(val == "--compress" || val == "--decompress") {
			if(argc != arg+3) {
				std::cerr << "format of " << val << " command: " << val << " <input file> <output file>\n";
				return 0;
			}

			const std::string input(argv[arg+1]);
			const std::string output(argv[arg+2]);

			scoped_istream stream = istream_file(input);
			if (stream->fail()) {
				std::cerr << "could not read file '" << input << "'\n";
				return 0;
			}

			config cfg;

			const bool compress = val == "--compress";
			try {
				const bool is_compressed = detect_format_and_read(cfg, *stream);
				if(is_compressed && compress) {
					std::cerr << input << " is already compressed\n";
					return 0;
				} else if(!is_compressed && !compress) {
					std::cerr << input << " is already decompressed\n";
					return 0;
				}

				scoped_ostream output_stream = ostream_file(output);
				write_possibly_compressed(*output_stream, cfg, compress);
			} catch(config::error& e) {
				std::cerr << input << " is not a valid Wesnoth file: " << e.message << "\n";
			} catch(io_exception& e) {
				std::cerr << "IO error: " << e.what() << "\n";
			}

			return 0;
		}
	}

	srand(time(NULL));

	game_controller game(argc,argv);

	if (!filesystem_init()) {
		std::cerr << "cannot init filesystem code\n";
		return 1;
	}

	// I would prefer to setup locale first so that early error
	// messages can get localized, but we need the game_controller
	// initialized to have get_intl_dir() to work.  Note: this
	// setlocale() but this does not take GUI language setting
	// into account.
	setlocale(LC_ALL, "C");
	setlocale(LC_MESSAGES, "");
	const std::string& intl_dir = get_intl_dir();
	bindtextdomain (PACKAGE, intl_dir.c_str());
	bind_textdomain_codeset (PACKAGE, "UTF-8");
	bindtextdomain (PACKAGE "-lib", intl_dir.c_str());
	bind_textdomain_codeset (PACKAGE "-lib", "UTF-8");
	textdomain (PACKAGE);

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

	loadscreen::global_loadscreen = new loadscreen(game.disp().video());
	loadscreen::global_loadscreen->clear_screen();

	res = game.init_language();
	if(res == false) {
		std::cerr << "could not initialize the language\n";
		return 0;
	}

	loadscreen::global_loadscreen->increment_progress(5, _ ("Loading game configuration."));
	res = game.init_config();
	if(res == false) {
		std::cerr << "could not initialize game config\n";
		return 0;
	}
	loadscreen::global_loadscreen->increment_progress(10, _ ("Re-initialize fonts for the current language."));

	res = font::load_font_config();
	if(res == false) {
		std::cerr << "could not re-initialize fonts for the current language\n";
		return 0;
	}

	const cursor::manager cursor_manager;
#if defined(_X11) && !defined(__APPLE__)
	SDL_EventState(SDL_SYSWMEVENT, SDL_ENABLE);
#endif

	int ntip = -1;
	config tips_of_day;

	loadscreen::global_loadscreen->set_progress(100, _("Loading title screen."));
	delete loadscreen::global_loadscreen;
	loadscreen::global_loadscreen = 0;

	for(;;) {
		//make sure the game config is always set to how it should be at the title screen
		//game.reset_game_cfg();

		statistics::fresh_stats();

		sound::play_music_repeatedly(game_config::title_music);

		std::cerr << "started music\n";
		std::cerr << (SDL_GetTicks() - start_ticks) << "\n";

		if(game.play_test() == false) {
			return 0;
		}

		recorder.clear();

		std::cerr << "showing title screen...\n";
		std::cerr << (SDL_GetTicks() - start_ticks) << "\n";
		gui::TITLE_RESULT res = game.is_loading() ? gui::LOAD_GAME : gui::TITLE_CONTINUE;

		while(res == gui::TITLE_CONTINUE) {
			res = gui::show_title(game.disp(),tips_of_day,&ntip);
		}

		game_controller::RELOAD_GAME_DATA should_reload = game_controller::RELOAD_DATA;
		std::cerr << "title screen returned result\n";
		if(res == gui::QUIT_GAME) {
			std::cerr << "quitting game...\n";
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
		} else if(res == gui::EDIT_PREFERENCES) {
			game.show_preferences();
			continue;
		} else if(res == gui::SHOW_ABOUT) {
			about::show_about(game.disp());
			continue;
		} else if(res == gui::SHOW_HELP) {
			help::help_manager help_manager(&game.game_config(), &game.units_data(), NULL);
			help::show_help(game.disp());
			continue;
		}
		if (recorder.at_end()) {
			game.play_game(should_reload);
		}
		else{
			game.play_replay();
		}
		ntip = -1; // Change tip when a game is played
	}

	return 0;
}

int main(int argc, char** argv)
{
#ifdef OS2 /* required for SDL_GetTicks to work on OS/2 */
        if ( SDL_Init(SDL_INIT_TIMER) < 0 ) {
		fprintf(stderr, "Couldn't initialize SDL: %s\n",SDL_GetError());
		return(1);
	}
#endif

#ifdef GP2X
        atexit(gp2x::return_to_menu);

        if(gp2x::init_joystick() < 0) {
                fprintf(stderr, "Couldn't initialize joystick: %s\n", SDL_GetError());
                return 1;
        }
#endif

	try {
		std::cerr << "Battle for Wesnoth v" << VERSION << "\n";
		time_t t = time(NULL);
		std::cerr << "Started on " << ctime(&t) << "\n";

		std::cerr << "started game: " << SDL_GetTicks() << "\n";
		const int res = play_game(argc,argv);
		std::cerr << "exiting with code " << res << "\n";
#ifdef OS2 /* required to correctly shutdown SDL on OS/2 */
	        SDL_Quit();
#endif
		return res;
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
	} 

#ifdef GP2X
        // We want this in gp2x so that users don't have to power-cycle
        // their consoles on unhandled exception
        catch(...) {
                std::cerr << "Unhandled exception. Exiting\n";
        }
#endif

	return 0;
}
