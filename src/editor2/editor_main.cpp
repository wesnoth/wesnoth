/* $Id$ */
/*
   Copyright (C) 2008 by Tomasz Sniatowski <kailoran@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "editor_main.hpp"

#include "../editor/editor.hpp"

#include "config.hpp"
#include "filesystem.hpp"
#include "game_preferences.hpp"
#include "gamestatus.hpp"
#include "log.hpp"
#include "wml_exception.hpp"
#include "serialization/parser.hpp"
#include "serialization/preprocessor.hpp"
#include "serialization/string_utils.hpp"
namespace editor2 {
	
EXIT_STATUS start(config& game_conf, CVideo& video)
{
	preproc_map defines_map;
	// define editor to do conditionnal loading in the main cfg
	defines_map["EDITOR"] = preproc_define();
	
	defines_map["MEDIUM"] = preproc_define();
	defines_map["NORMAL"] = preproc_define();
		
#if defined(__APPLE__)
	defines_map["APPLE"] = preproc_define();
#endif
	std::string mapdata = map_editor::new_map(22, 22, t_translation::GRASS_LAND);
	config cfg;
	scoped_istream stream = preprocess_file("data/", &defines_map);
//	read(cfg, *stream);			
	cfg = game_conf;
	config* theme_cfg = cfg.find_child("theme", "name", "editor");
	config dummy_theme;
	if (!theme_cfg) {
		std::cerr << "Editor theme could not be loaded." << std::endl;
		theme_cfg = &dummy_theme;
	}
	const config dummy_cfg;
	std::string filename = "new map";
	bool from_scenario = true;
		
	for(;;) {
		try {
			editormap map(cfg, mapdata);
			editor_display gui(video, map, *theme_cfg, cfg, config());
			map_editor::map_editor editor(gui, map, *theme_cfg, cfg);
			editor.set_file_to_save_as(filename, from_scenario);
			editor.main_loop();
			break;
		} catch (map_editor::map_editor::new_map_exception &e) {
			mapdata = e.new_map;
			filename = e.new_filename;
			from_scenario = e.from_scenario;
		} catch (gamemap::incorrect_format_exception) {
			std::cerr << "The map is not in a correct format, sorry." << std::endl;
			return EXIT_ERROR;
		} catch (twml_exception& e) {
			std::cerr << "WML exception:\nUser message: " 
				<< e.user_message << "\nDev message: " << e.dev_message << '\n';
			return EXIT_ERROR;
		}
	}
	return EXIT_ERROR;
}

} //end namespace editor2
