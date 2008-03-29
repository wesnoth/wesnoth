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

//! @file editor/editor_main.cpp
//!

#include "editor.hpp"
#include "../config.hpp"
#include "../cursor.hpp"
#include "../filesystem.hpp"
#include "../font.hpp"
#include "../game_config.hpp"
#include "../gettext.hpp"
#include "../image.hpp"
#include "../language.hpp"
#include "../log.hpp"
#include "../map.hpp"
#include "../minimap.hpp"
#include "../preferences.hpp"
#include "../random.hpp"
#include "../util.hpp"
#include "../video.hpp"
#include "../wesconfig.h"
#include "../wml_exception.hpp"
#include "serialization/parser.hpp"
#include "serialization/preprocessor.hpp"
#include "serialization/string_utils.hpp"

#include <cctype>
#include <clocale>
#include <cstdlib>
#include <iostream>
#include <map>
#include <string>

namespace {
	static std::string wm_title_string;
}

int main(int argc, char** argv)
{
	const std::string rev = game_config::revision.empty() ? "" :
		" (" + game_config::revision + ")";

	std::cerr << "Battle for Wesnoth Map Editor v" << VERSION << rev << '\n';
	time_t t = time(NULL);
	std::cerr << "Started on " << ctime(&t) << "\n";

	game_config::editor = true;
	rng generator;
	const set_random_generator generator_setter(&generator);

	int arg;
	for(arg = 1; arg != argc; ++arg) {
		const std::string val(argv[arg]);
		if(val.empty()) {
			continue;
		}

		if(val == "--help" || val == "-h") {
			std::cout << "usage: " << argv[0]
			<< " [options] [map]\n"
                	<< "  --bpp                     Set the bits per pixel.\n"
                	<< "  --datadir                 Select the data directory to use.\n"
                	<< "  -f, --fullscreen          Runs the editor in full screen mode.\n"
                	<< "  -h, --help                Prints this message and exits.\n"
                	<< "  --log-<level>=<domain1>,<domain2>,...\n"
                	<< "                            Sets the severity level of the log domains.\n"
                	<< "                            'all' can be used to match any log domain.\n"
                	<< "                            Available levels: error, warning, info, debug.\n"
                	<< "                            By default the 'error' level is used.\n"
                	<< "  --logdomains              List defined log domains and exit.\n"
                	<< "  --path                    Prints the name of the game data directory and exits.\n"
			<< "  -r, --resolution XxY      Sets the screen resolution. Example: -r 800x600.\n"
                	<< "  -v, --version             Prints the game's version number and exits.\n"
                	<< "  -w, --windowed            Runs the editor in windowed mode.\n"
			;
			return 0;
		} else if(val == "--version" || val == "-v") {
			std::cout << "Battle for Wesnoth "
				  << game_config::version
#if defined(SVNREV) && defined(DO_DISPLAY_REVISION)
				  << " (" << game_config::revision << ")"
#endif /* defined(SVNREV) and defined(DO_DISPLAY_REVISION) */
			          << "\n";
			return 0;
		} else if(val == "--path") {
			std::cout <<  game_config::path
			          << "\n";
			return 0;
		} else if(val == "--logdomains") {
		  std::cout << lg::list_logdomains() << "\n";
			return 0;
		}
	}

	CVideo video;

	const font::manager font_manager;
	const preferences::base_manager prefs_manager;
	const image::manager image_manager;
	resize_monitor resize_monitor_;
	binary_paths_manager paths_manager;
	std::string filename = "";
	std::string mapdata;
	bool from_scenario;

	int choosen_bpp = 0;

	for(arg = 1; arg != argc; ++arg) {
		const std::string val(argv[arg]);
		if(val.empty()) {
			continue;
		}
		if(val == "--bpp" && arg + 1 != argc) {
			++arg;
			choosen_bpp = lexical_cast_default<int>(argv[arg]);
		}

		else if(val == "--resolution" || val == "-r") {
			if(arg+1 != argc) {
				++arg;
				const std::string val(argv[arg]);
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
		} else if(val == "--windowed" || val == "-w") {
			preferences::set_fullscreen(false);
		} else if(val == "--fullscreen" || val == "-f") {
			preferences::set_fullscreen(true);
		} else if(val == "--datadir") {
			if (arg+1 != argc) {
				const std::string val(argv[++arg]);

				const std::string cwd = get_cwd();

				if(val[0] == '/') {
					game_config::path = val;
				} else if(cwd != "") {
					game_config::path = cwd + '/' + val;
				} else {
					std::cerr << "Could not get working directory\n";
					return 0;
				}

				if(!is_directory(game_config::path)) {
					std::cerr << "Could not find directory '" << game_config::path << "'\n";
					return 0;
				}
			}
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
		} else if(val[0] == '-') {
			std::cerr << "unknown option: " << val << "\n";
			return 0;
		} else {
			filename = val;
			try {
				mapdata = read_file(filename);
			}
			catch (io_exception) {
				std::cerr << "Could not read the map file, sorry." << std::endl;
				return 1;
			}
		}
	}

	setlocale (LC_ALL, "");
	const std::string& intl_dir = get_intl_dir();
	bindtextdomain (PACKAGE "-editor", intl_dir.c_str());
	bind_textdomain_codeset (PACKAGE "-editor", "UTF-8");
	bindtextdomain (PACKAGE "-lib", intl_dir.c_str());
	bind_textdomain_codeset (PACKAGE "-lib", "UTF-8");
	textdomain (PACKAGE "-editor");

	// Blatant cut and paste from game.cpp
	image::set_wm_icon();
	int video_flags = preferences::fullscreen() ? FULL_SCREEN : 0;
	std::pair<int,int> resolution = preferences::resolution();

	std::cerr << "checking mode possible...\n";
	const int default_bpp = 24;
	int bpp = default_bpp;

	if(choosen_bpp == 0) {
		bpp = video.modePossible(resolution.first,resolution.second,default_bpp,video_flags);

		std::cerr << bpp << "\n";

		if(bpp == 0)  {
			//Video mode not supported, maybe from bad prefs.
			std::cerr << "The video mode, " << resolution.first
				  << "x" << resolution.second << "x" << default_bpp <<
				  "is not supported\nAttempting 1024x768x" << default_bpp << "...\n";

			//Attempt 1024x768.
			resolution.first = 1024;
			resolution.second = 768;

			bpp = video.modePossible(resolution.first,resolution.second,default_bpp,video_flags);

			if(bpp == 0) {
				 //Attempt 1024x768.
				resolution.first = 1024;
				resolution.second = 768;
				std::cerr << "1024x768x" << default_bpp << " is not possible.\nAttempting 800x600x" << default_bpp << "...\n";

				resolution.first = 800;
				resolution.second = 600;

				bpp = video.modePossible(resolution.first,resolution.second,default_bpp,video_flags);
			}

			if(bpp == 0) {
				//couldn't do 1024x768 or 800x600

				std::cerr << "The required video mode, " << resolution.first
					  << "x" << resolution.second << "x" << default_bpp <<
					  "is not supported\n";

				return 0;
			}
		}
	} else {
		bpp = choosen_bpp;
	}

	std::cerr << "setting mode to " << resolution.first << "x" << resolution.second << "\n";
	const int res = video.setMode(resolution.first,resolution.second,bpp,video_flags);
	video.setBpp(bpp);
	if(res == 0) {
		std::cerr << "required video mode, " << resolution.first << "x"
				  << resolution.second << "x" << bpp << " is not supported\n";
		return 0;
	}
	preproc_map defines_map;
	// define editor to do conditionnal loading in the main cfg
	defines_map["EDITOR"] = preproc_define();

	defines_map["MEDIUM"] = preproc_define();
	defines_map["NORMAL"] = preproc_define();

#if defined(__APPLE__)
	defines_map["APPLE"] = preproc_define();
#endif

	//Set the locale first, then read the configuration, or else WML
	//strings are not correctly translated. Does this work on on the win32
	//platform?
	load_language_list();
	const bool lang_res = ::set_language(get_locale());
	if(!lang_res) {
		std::cerr << "No translation for locale '" << get_locale().language
		          << "', default to system locale\n";

		const bool lang_res = ::set_language(get_languages()[0]);
		if(!lang_res) {
			std::cerr << "Language data not found\n";
		}
	}

	// Set the caption of the window
	wm_title_string = _("Battle for Wesnoth Map Editor");
	wm_title_string += " - " + game_config::version
			+ (game_config::revision.empty() ? "" :
			" (" + game_config::revision + ")");
	SDL_WM_SetCaption(wm_title_string.c_str(), NULL);

	//Read the configuration af
	config cfg;
	try {
		scoped_istream stream = preprocess_file("data/", &defines_map);
		read(cfg, *stream);
	}
	catch (config::error e) {
		std::cerr << "Error when reading game config: '" << e.message << "'" << std::endl;
	}
	font::load_font_config();
	paths_manager.set_paths(cfg);
	::init_textdomains(cfg);

	if(mapdata.empty()) {
		mapdata = map_editor::new_map(22, 22, t_translation::GRASS_LAND);
	}

	srand(time(NULL));
	bool done = false;
	config* theme_cfg = cfg.find_child("theme", "name", "editor");
	config dummy_theme;
	if (!theme_cfg) {
		std::cerr << "Editor theme could not be loaded." << std::endl;
		theme_cfg = &dummy_theme;
	}

	std::cerr << "entering while...\n";
	events::event_context ec;
	map_editor::check_data(mapdata, filename, from_scenario, cfg);
	while (!done) {
		try {
			std::cerr << "creating map...\n";
			//! @todo Allow the editor to also create mask maps
			editormap map(cfg, mapdata);

			const config dummy_cfg;
			editor_display gui(video, map, *theme_cfg, cfg, dummy_cfg);

			map_editor::map_editor editor(gui, map, *theme_cfg, cfg);
			editor.set_file_to_save_as(filename, from_scenario);
			editor.main_loop();
			done = true;
		}
		catch (map_editor::map_editor::new_map_exception &e) {
			mapdata = e.new_map;
			filename = e.new_filename;
			from_scenario = e.from_scenario;
		}
		catch (gamemap::incorrect_format_exception) {
			std::cerr << "The map is not in a correct format, sorry." << std::endl;
			return 1;
		} catch(twml_exception& e) {
			std::cerr << "WML exception:\nUser message: " 
				<< e.user_message << "\nDev message: " << e.dev_message << '\n';
			return 1;
		} catch (CVideo::quit&) {
			return 0;
		}
	}

	return 0;
}
