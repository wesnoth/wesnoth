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

#include "../actions.hpp"
#include "../ai.hpp"
#include "../config.hpp"
#include "../dialogs.hpp"
#include "../display.hpp"
#include "../font.hpp"
#include "../game_config.hpp"
#include "../gamestatus.hpp"
#include "../key.hpp"
#include "../language.hpp"
#include "../menu.hpp"
#include "../pathfind.hpp"
#include "../playlevel.hpp"
#include "../team.hpp"
#include "../unit_types.hpp"
#include "../unit.hpp"
#include "../video.hpp"

#include <cctype>
#include <iostream>
#include <map>
#include <string>

int main(int argc, char** argv)
{
	const double scroll_speed = 30.0;
	const double zoom_amount = 5.0;

	if(argc == 1) {
		std::cout << "usage: " << argv[0] << " map-name\n";
		return 0;
	}

	std::map<std::string,std::string> defines_map;
	defines_map["MEDIUM"] = "";
	config cfg(preprocess_file("data/game.cfg",&defines_map));

	set_language("English", cfg);

	std::cout << "a\n";
	std::string mapdata = read_file(argv[1]);
	if(mapdata.empty()) {
		for(int i = 0; i != 30; ++i) {
			mapdata = mapdata + "gggggggggggggggggggggggggggggggggggggg\n";
		}
	}
	
	std::cout << "b\n";
	gamemap map(cfg,mapdata);
	
	CVideo video;

	video.setMode(1024,768,16,0);

	CKey key;
	gamestatus status(cfg,0);
	std::vector<team> teams;

	std::map<gamemap::location,unit> units;
	display gui(units,video,map,status,teams);
	gui.draw_terrain_palette(gui.mapx()+10,150,0);

	gamemap::TERRAIN selected_terrain = 0;

	bool first_time = true;

	const font::manager font_manager;

	std::cerr << "starting for(;;)\n";
	for(;;) {
		if(key[SDLK_ESCAPE])
			break;

		int mousex, mousey;
		const int mouse_flags = SDL_GetMouseState(&mousex,&mousey);
		const bool new_left_button = mouse_flags & SDL_BUTTON_LMASK;
		const bool new_right_button = mouse_flags & SDL_BUTTON_RMASK;

		if(key[SDLK_UP] || mousey == 0)
			gui.scroll(0.0,-scroll_speed);

		if(key[SDLK_DOWN] || mousey == gui.y()-1)
			gui.scroll(0.0,scroll_speed);

		if(key[SDLK_LEFT] || mousex == 0)
			gui.scroll(-scroll_speed,0.0);

		if(key[SDLK_RIGHT] || mousex == gui.x()-1)
			gui.scroll(scroll_speed,0.0);

		if(key[SDLK_Z])
			gui.zoom(zoom_amount);

		if(key[SDLK_X])
			gui.zoom(-zoom_amount);

		if(key[SDLK_D])
			gui.default_zoom();

		gui.highlight_hex(gui.hex_clicked_on(mousex,mousey));
		if(new_left_button) {
			const gamemap::TERRAIN terrain_on =
			               gui.get_terrain_on(gui.mapx()+10,150,mousex,mousey);
			if(terrain_on && terrain_on != selected_terrain) {
				selected_terrain = terrain_on;
				gui.draw_terrain_palette(gui.mapx()+10,150,selected_terrain);
			}

			const gamemap::location hex = gui.hex_clicked_on(mousex,mousey);
			if(map.on_board(hex)) {
				const gamemap::TERRAIN terrain = map[hex.x][hex.y];
				if(selected_terrain && selected_terrain != terrain) {
					map.set_terrain(hex,selected_terrain);
					gui.recalculate_minimap();

					gamemap::location locs[7];
					locs[0] = hex;
					get_adjacent_tiles(hex,locs+1);
					for(int i = 0; i != 7; ++i) {
						gui.draw_tile(locs[i].x,locs[i].y);
					}
				}
			}
		}

		gui.draw();

		if(first_time) {
			std::cerr << "drawing terrain pallette...\n";
			gui.draw_terrain_palette(gui.mapx()+10,150,0);
			first_time = false;
		}

		SDL_Delay(20);
		pump_events();
	}

	system("pwd");
	int res = gui::show_dialog(gui,NULL,"Save?","Do you want to save changes?",
	                           gui::YES_NO);
	if(res == 0) {
		write_file(argv[1],map.write());
	}

	return 0;
}
