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
#include "SDL_keysym.h"

#include "../actions.hpp"
#include "../ai.hpp"
#include "../config.hpp"
#include "../dialogs.hpp"
#include "../display.hpp"
#include "../filesystem.hpp"
#include "../font.hpp"
#include "../game_config.hpp"
#include "../gamestatus.hpp"
#include "../image.hpp"
#include "../key.hpp"
#include "../language.hpp"
#include "../widgets/menu.hpp"
#include "../pathfind.hpp"
#include "../playlevel.hpp"
#include "../preferences.hpp"
#include "../team.hpp"
#include "../unit_types.hpp"
#include "../unit.hpp"
#include "../video.hpp"

#include <cctype>
#include <iostream>
#include <map>
#include <string>

void drawbar(display& disp);
bool drawterrainpalette(display& disp, int start, gamemap::TERRAIN selected, gamemap map);
int tileselected(int x, int y, display& disp);


int main(int argc, char** argv)
{
	if(argc > 2) {
		std::cout << "usage: " << argv[0] << " map-name\n";
		return 0;
	}

	const double scroll_speed = 30.0;
	const double zoom_amount = 5.0;

	CVideo video;

	video.setMode(1024,768,16,0);

	const font::manager font_manager;
	const preferences::manager prefs_manager;
	const image::manager image_manager;

	preproc_map defines_map;
	defines_map["MEDIUM"] = preproc_define();
	config cfg(preprocess_file("data/game.cfg",&defines_map));

	set_language("English", cfg);

	std::string filename;

	if(argc == 1) {

		const std::string path = "data/maps/";

		display::unit_map u_map;
		config dummy_cfg("");

		display disp(u_map,video,gamemap(dummy_cfg,"1"),gamestatus(dummy_cfg,0),
		             std::vector<team>());

		std::vector<std::string> files;
		get_files_in_dir(path,&files);

		files.push_back("New Map...");

		const int res = gui::show_dialog(disp,NULL,"","Choose map to edit:",gui::OK_CANCEL,&files);
		if(res < 0) {
			return 0;
		}

		if(res == int(files.size()-1)) {
			filename = "new-map";
			gui::show_dialog(disp,NULL,"","Create new map",gui::OK_ONLY,NULL,NULL,"",&filename);
			if(filename == "")
				return 0;
		} else {
			filename = files[res];
		}

		filename = path + filename;

	} else if(argc == 2) {
		filename = argv[1];
	}

	std::cout << "a\n";
	std::string mapdata = read_file(filename);
	if(mapdata.empty()) {
		for(int i = 0; i != 30; ++i) {
			mapdata = mapdata + "gggggggggggggggggggggggggggggggggggggg\n";
		}
	}
	
	std::cout << "b\n";
	gamemap map(cfg,mapdata);
	
	CKey key;
	gamestatus status(cfg,0);
	std::vector<team> teams;

	std::map<gamemap::location,unit> units;
	display gui(units,video,map,status,teams);

	std::vector<std::string> terrain_names;
	const std::vector<gamemap::TERRAIN> terrains = map.get_terrain_precedence();
	if(terrains.empty()) {
		std::cerr << "No terrain found\n";
		return 0;
	}

	for(std::vector<gamemap::TERRAIN>::const_iterator t = terrains.begin(); t != terrains.end(); ++t) {
		terrain_names.push_back(map.terrain_name(*t));
	}

	gui::menu terrain_menu(gui,terrain_names);
	gui::button tup(gui, "", gui::button::TYPE_PRESS,"uparrow");
	gui::button tdown(gui, "", gui::button::TYPE_PRESS,"downarrow");
	tup.set_xy(gui.mapx() + 10, 165);
	tdown.set_xy((gui.x() - tdown.width()) - 10, 165);

	terrain_menu.set_width((gui.x() - gui.mapx()));
	terrain_menu.set_loc(gui.mapx()+2,200);

	gamemap::TERRAIN selected_terrain = terrains[1];
	int tstart = 0;

	//Draw the nice background bar
	drawbar(gui);

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

		if(key[SDLK_z])
			gui.zoom(zoom_amount);

		if(key[SDLK_x])
			gui.zoom(-zoom_amount);

		if(key[SDLK_d])
			gui.default_zoom();

		const gamemap::location cur_hex = gui.hex_clicked_on(mousex,mousey);
		for(int num_key = SDLK_1; num_key != SDLK_9; ++num_key) {
			if(key[num_key]) {
				if(map.on_board(cur_hex)) {
					map.set_terrain(cur_hex,gamemap::CASTLE);
				}
				map.set_starting_position(num_key+1-SDLK_1,cur_hex);

				gui.invalidate_all();
				break;
			}
		}

		gui.highlight_hex(cur_hex);
		if(new_left_button) {

			const gamemap::location hex = gui.hex_clicked_on(mousex,mousey);
			if(map.on_board(hex)) {
				const gamemap::TERRAIN terrain = map[hex.x][hex.y];
				if(selected_terrain != terrain) {
					map.set_terrain(hex,selected_terrain);
					gui.recalculate_minimap();

					gamemap::location locs[7];
					locs[0] = hex;
					get_adjacent_tiles(hex,locs+1);
					for(int i = 0; i != 7; ++i) {
						gui.draw_tile(locs[i].x,locs[i].y);
					}
				}
			}else{
				int tselect = tileselected(mousex,mousey,gui);
				if(tselect >= 0)
					selected_terrain = terrains[tstart+tselect];
			}
		}


		gui.draw(false);
		if(drawterrainpalette(gui, tstart, selected_terrain, map)==false)
			tstart--;

		if(tup.process(mousex,mousey,new_left_button)) {
			tstart++;
		}

		if(tdown.process(mousex,mousey,new_left_button)) {
			tstart--;
			if(tstart<0)
				tstart=0;
		}
		
		gui.update_display();
		SDL_Delay(20);
		events::pump();
	}

	int res = gui::show_dialog(gui,NULL,"Save?","Do you want to save changes?",
	                           gui::YES_NO);
	if(res == 0) {
		write_file(filename,map.write());
	}

	return 0;
}

void drawbar(display& disp)
{
	const std::string RightSideBot = "misc/rightside-bottom.png";
	const std::string RightSideTop = "misc/rightside-editor.png";
	SDL_Surface* const screen = disp.video().getSurface();
	const scoped_sdl_surface image_top(image::get_image(RightSideTop,image::UNSCALED));

	const scoped_sdl_surface image(image_top != NULL ?
	 image::get_image_dim(RightSideBot,image_top->w,screen->h-image_top->h) : NULL);
	if(image_top != NULL && image != NULL && image_top->h < screen->h) {
		SDL_Rect dstrect;
		dstrect.x = disp.mapx();
		dstrect.y = 0;
		dstrect.w = image_top->w;
		dstrect.h = image_top->h;

		if(dstrect.x + dstrect.w <= disp.x() &&
		   dstrect.y + dstrect.h <= disp.y()) {
			SDL_BlitSurface(image_top,NULL,screen,&dstrect);
				dstrect.y = image_top->h;
			dstrect.h = image->h;
			if(dstrect.y + dstrect.h <= disp.y()) {
				SDL_BlitSurface(image,NULL,screen,&dstrect);
			}
		} else {
			std::cout << (dstrect.x+dstrect.w) << " > " << disp.x() << " or " << (dstrect.y + dstrect.h) << " > " << disp.y() << "\n";
		}
	}

	update_rect(disp.mapx(),0,disp.x()-disp.mapx(),disp.y());
}

namespace {
	const size_t nterrains = 6;
}

bool drawterrainpalette(display& disp, int start, gamemap::TERRAIN selected, gamemap map)
{
	int x = disp.mapx() + 35;
	int y = 200;

	int starting = start;
	int ending = starting+nterrains;

	bool status = true;

	SDL_Rect invalid_rect;
	invalid_rect.x = x;
	invalid_rect.y = y;
	invalid_rect.w = 0;

	SDL_Surface* const screen = disp.video().getSurface();

	std::vector<gamemap::TERRAIN> terrains = map.get_terrain_precedence();
	if(ending>terrains.size()){
		ending = terrains.size();
		starting = ending - nterrains;
		status = false;
	}

	for(int counter = starting; counter < ending; counter++){
		const gamemap::TERRAIN terrain = terrains[counter];
		const scoped_sdl_surface image(image::get_image("terrain/" + map.get_terrain_info(terrain).default_image() + ".png"));

		if(image == NULL) {
			std::cerr << "image for terrain '" << counter << "' not found\n";
			return status;
		}

		SDL_Rect dstrect;
		dstrect.x = x;
		dstrect.y = y;
		dstrect.w = image->w;
		dstrect.h = image->h;

		SDL_BlitSurface(image,NULL,screen,&dstrect);
		gui::draw_rectangle(x,y,image->w-1,image->h-1,
		                    terrain == selected?0xF000:0,screen);

		y += dstrect.h+2;

		if(image->w > invalid_rect.w)
			invalid_rect.w = image->w;

	}

	invalid_rect.h = y - invalid_rect.y;
	update_rect(invalid_rect);
	return status;
}

int tileselected(int x, int y, display& disp)
{
	int status = -1;

	for(int i = 0; i != nterrains; i++)
	{
		int px = disp.mapx() + 35;
		int py = 200 + (i * 77);
		int pxx = disp.mapx() + 35 + 75;
		int pyy = 200 + ((1 + i) * 77);
		if(x>px && x<pxx && y>py && y<pyy)
			status = i;
	}
	return status;
}

