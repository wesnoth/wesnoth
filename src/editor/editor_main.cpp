/*
  Copyright (C) 2003 by David White <davidnwhite@optusnet.com.au>
  Part of the Battle for Wesnoth Project http://wesnoth.whitevine.net

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License.
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY.

  See the COPYING file for more details.
*/


#include "editor.hpp"
#include "../config.hpp"
#include "../game_config.hpp"
#include "../font.hpp"
#include "../image.hpp"
#include "../map.hpp"
#include "../team.hpp"
#include "../preferences.hpp"
#include "../language.hpp"

#include <cctype>
#include <iostream>
#include <map>
#include <string>

int main(int argc, char** argv)
{
	game_config::editor = true;

	if(argc > 2) {
		std::cout << "usage: " << argv[0] << " [map-name]" << std::endl;
		return 0;
	}

	CVideo video;

	const font::manager font_manager;
	const preferences::manager prefs_manager;
	const image::manager image_manager;
	std::pair<int, int> desired_resolution = preferences::resolution();
	int video_flags = preferences::fullscreen() ? FULL_SCREEN : 0;
	video.setMode(desired_resolution.first, desired_resolution.second,
				  16, video_flags);

	preproc_map defines_map;
	defines_map["MEDIUM"] = preproc_define();
	defines_map["NORMAL"] = preproc_define();
	config cfg(preprocess_file("data/game.cfg", &defines_map));

	set_language("English");

	std::string filename;
	std::string mapdata;

	if(argc == 2) {
		filename = argv[1];
		try {
			mapdata = read_file(filename);
		}
		catch (io_exception) {
			std::cerr << "Could not read the map file, sorry." << std::endl;
			return 1;
		}
	}
	else {
		filename = "";
	}
	if(mapdata.empty()) {
		for(int i = 0; i != 20; ++i) {
			mapdata = mapdata + "gggggggggggggggggggg\n";
		}
	}
	bool done = false;
	gamestatus status(cfg, 0);
	std::vector<team> teams;
	// Add a dummy team so the reports will be handled properly.
	teams.push_back(team(cfg));
	config* theme_cfg = cfg.find_child("theme", "name", "editor");
	config dummy_theme("");
	if (!theme_cfg) {
		std::cerr << "Editor theme could not be loaded." << std::endl;
		theme_cfg = &dummy_theme;
	}
	std::map<gamemap::location,unit> units;
	while (! done) {
		try {
			gamemap map(cfg, mapdata);

			std::cerr << "Using theme cfg: " << std::endl << theme_cfg->write() << std::endl;
			display gui(units, video, map, status, teams,
				    *theme_cfg, cfg);
			gui.set_grid(preferences::grid());
	
			events::event_context ec;
			map_editor::map_editor editor(gui, map, *theme_cfg, cfg);
			editor.set_file_to_save_as(filename);
			editor.main_loop();
			done = true;
		}
		catch (map_editor::map_editor::new_map_exception &e) {
			mapdata = e.new_map;
			filename = e.new_filename;
		}
		catch (gamemap::incorrect_format_exception) {
			std::cerr << "The map is not in a correct format, sorry." << std::endl;
			return 1;
		}
	}
	return 0;
}
